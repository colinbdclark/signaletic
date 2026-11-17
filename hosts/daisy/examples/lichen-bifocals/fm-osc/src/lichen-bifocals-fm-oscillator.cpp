#include <libsignaletic.h>
#include "../../../../include/lichen-bifocals-device.hpp"
#include "../../../shared/include/summed-cv-in.h"

using namespace lichen::bifocals;
using namespace sig::libdaisy;

#define SAMPLERATE 96000
#define BLOCK_SIZE 384 // 4 ms.
#define HEAP_SIZE 1024 * 256 // 256 KB
#define WAVETABLE_HEAP_SIZE 1024 * 1024 * 2 // 2 MB
#define MAX_NUM_SIGNALS 64
#define NUM_RATIOS 17

// The frequency knob spans 1-200 Hz, centred on its geometric mean.
#define FUNDAMENTAL_CENTRE_FREQUENCY 14.142136f // sqrt(200)
#define FUNDAMENTAL_HALF_RANGE 3.821928f // log2(200) / 2 octaves

// V/Oct CV can push the fundamental an octave beyond either end of the
// knob's range, to 0.5-400 Hz.
#define FUNDAMENTAL_CV_HALF_RANGE (FUNDAMENTAL_HALF_RANGE + 1.0f)

// In free mode, the modulator spans 12 octaves around the same centre.
#define FREE_MODULATOR_RANGE 12.0f

// The modulation index and the external phase modulation amount are each
// the sum of a knob (0.0 to 1.0) and CV (-1.0 to 1.0), in cycles of phase
// deviation, clamped to this maximum and to zero.
#define MAX_MODULATION_AMOUNT 2.0f

// When the ratio changes, the modulator's phase glides into alignment
// with the carrier's phase over this time, in seconds.
#define RATIO_GLIDE 0.05f

// Each wavetable level covers one octave from 1 Hz to 32 kHz, and contains
// the harmonics below 20 kHz. Phase modulation makes the bank readers read
// much faster than the fundamental, so the levels extend far above it.
#define WAVETABLE_LENGTH 4096
#define NUM_WAVETABLE_LEVELS 15
#define LOWEST_WAVETABLE_FREQUENCY 1.0f
#define MAX_HARMONIC_FREQUENCY 20000.0f

// The external phase modulation input is low pass filtered at this
// frequency, in Hz. Phase modulation by a signal with sharp edges, such as
// a saw, spreads sidebands so widely that they alias.
#define EXT_PHASE_MODULATION_LOW_PASS_FREQUENCY 2000.0f

// The bank readers select each wavetable level from the peak rate at which
// they read, so that phase modulation doesn't make their harmonics alias.
// The peak falls back over this time, in seconds. The release has to be
// much longer than a modulation cycle, or the level changes within each
// cycle and adds sidebands.
#define PEAK_RELEASE 0.5f

#define LOW_PASS_FREQUENCY 300.0f
// The state variable filter's resonance ranges from 0.0 to 1.0.
// A resonance of 0.293 gives a maximally flat response with a Q of 0.707.
#define LOW_PASS_RESONANCE 0.293f
#define DC_BLOCK_FREQUENCY 0.15f
#define SOFT_CLIP_THRESHOLD 0.8f

// Each comment maps the control to the Bifocals panel label.

// Freq
#define FREQUENCY_KNOB sig_host_KNOB_1
#define V_OCT_CV sig_host_CV_IN_2

 // Gain
#define INDEX_KNOB sig_host_KNOB_2
#define INDEX_CV sig_host_CV_IN_4

// Skew
#define RATIO_KNOB sig_host_KNOB_3
#define RATIO_CV sig_host_CV_IN_5

// Shape
#define EXT_PHASE_MODULATION_AMOUNT_KNOB sig_host_KNOB_4
#define EXT_PHASE_MODULATION_AMOUNT_CV sig_host_CV_IN_1

// Reso
#define WAVEFORM_KNOB sig_host_KNOB_5
#define WAVEFORM_CV sig_host_CV_IN_3

uint8_t memory[HEAP_SIZE];
struct sig_AllocatorHeap heap = {
    .length = HEAP_SIZE,
    .memory = (void*) memory
};

struct sig_Allocator allocator = {
    .impl = &sig_TLSFAllocatorImpl,
    .heap = &heap
};

uint8_t DSY_SDRAM_BSS wavetableMemory[WAVETABLE_HEAP_SIZE];
struct sig_AllocatorHeap wavetableHeap = {
    .length = WAVETABLE_HEAP_SIZE,
    .memory = (void*) wavetableMemory
};

struct sig_Allocator wavetableAllocator = {
    .impl = &sig_TLSFAllocatorImpl,
    .heap = &wavetableHeap
};

// Each modulator:carrier ratio is numerators[i]:denominators[i].
// The list has eight ratios on either side of 1:1, which is at the centre
// of the ratio knob. Above 1:1, the ratios are from Truax's normal form
// ratios ordered to the Farey Sequence, limited to 4:1 to keep the
// modulator in low frequency territory.
// https://www.sfu.ca/sonic-studio-webdav/handbook/fmtut.html
// Below 1:1, the ratios are mostly subharmonics, down to 1:8.
float numerators[NUM_RATIOS] = {
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 2.0f, 1.0f,
    1.0f,
    2.0f, 9.0f, 7.0f, 5.0f,
    8.0f, 3.0f, 7.0f, 4.0f
};

float denominators[NUM_RATIOS] = {
    8.0f, 7.0f, 6.0f, 5.0f,
    4.0f, 3.0f, 5.0f, 2.0f,
    1.0f,
    1.0f, 4.0f, 3.0f, 2.0f,
    3.0f, 1.0f, 2.0f, 1.0f
};

struct sig_Buffer numeratorList = {
    .length = NUM_RATIOS,
    .samples = numerators
};

struct sig_Buffer denominatorList = {
    .length = NUM_RATIOS,
    .samples = denominators
};

// The waveforms are sine-phase, so phase modulation between the
// phase-locked carrier and modulator produces no DC offset, and crossfading
// between neighbouring waveforms does not cancel their fundamentals.
const struct sig_WaveformShape shapes[sig_StandardWaveform_COUNT] = {
    sig_SINE_PHASE_WAVEFORMS[sig_StandardWaveform_SINE],
    sig_SINE_PHASE_WAVEFORMS[sig_StandardWaveform_TRIANGLE],
    sig_SINE_PHASE_WAVEFORMS[sig_StandardWaveform_REVERSE_SAW],
    sig_SINE_PHASE_WAVEFORMS[sig_StandardWaveform_SQUARE],
    sig_SINE_PHASE_WAVEFORMS[sig_StandardWaveform_SAW]
};

struct sig_dsp_Signal* listStorage[MAX_NUM_SIGNALS];
struct sig_List signals;
struct sig_dsp_SignalListEvaluator* evaluator;
DaisyHost<BifocalsDevice> host;

struct sig_Buffer* sineTable;
struct sig_AdditiveWavetableBank* wavetables;

struct sig_host_FilteredCVIn* frequencyKnob;
struct sig_host_CVIn* vOctCV;
struct sig_dsp_BinaryOp* pitch;
struct sig_dsp_Clamp* clampedPitch;
struct sig_dsp_LinearToFreq* fundamental;
struct sig_host_SummedCVIn* indexIn;
struct sig_dsp_Clamp* clampedIndex;
struct sig_dsp_Smooth* modulationIndex;
struct sig_host_FilteredCVIn* ratioKnob;
struct sig_host_FilteredCVIn* ratioCV;
struct sig_dsp_BinaryOp* combinedRatio;
struct sig_dsp_Clamp* clampedRatio;
struct sig_dsp_List* ratioNumerator;
struct sig_dsp_List* ratioDenominator;
struct sig_dsp_ScaleOffset* freeModulatorPitch;
struct sig_dsp_LinearToFreq* freeModulatorFrequency;
struct sig_dsp_BinaryOp* freeNumerator;
struct sig_dsp_ConstantValue* freeDenominator;
struct sig_host_SwitchIn* modeButton;
struct sig_dsp_ToggleGate* freeMode;
struct sig_host_CVOut* led;
struct sig_dsp_Branch* numerator;
struct sig_dsp_Branch* denominator;
struct sig_dsp_BinaryOp* ratio;
struct sig_dsp_BinaryOp* modulatorFrequency;
struct sig_dsp_Phasor* carrierPhase;
struct sig_dsp_RatioPhasor* modulatorPhase;
struct sig_dsp_ConstantValue* audioInLowPassFrequency;
struct sig_dsp_ConstantValue* audioInLowPassResonance;
struct sig_host_SummedCVIn* phaseModulationAmountIn;
struct sig_dsp_Clamp* clampedPhaseModulationAmount;
struct sig_dsp_Smooth* phaseModulationAmount;
struct sig_dsp_BinaryOp* leftExternalPhaseModulation;
struct sig_dsp_BinaryOp* rightExternalPhaseModulation;
struct sig_host_SummedCVIn* waveformIn;
struct sig_dsp_Smooth* waveform;
struct sig_dsp_WavetableReader* sineModulator;
struct sig_dsp_AdditiveWavetableBankReader* waveformCarrier;
struct sig_dsp_AdditiveWavetableBankReader* waveformModulator;
struct sig_dsp_WavetableReader* sineCarrier;
struct sig_dsp_ConstantValue* lowPassFrequency;
struct sig_dsp_ConstantValue* lowPassResonance;
struct sig_dsp_SVFLowPass* leftLowPass;
struct sig_dsp_SVFLowPass* rightLowPass;
struct sig_dsp_DCBlock* leftDCBlocker;
struct sig_dsp_DCBlock* rightDCBlocker;
struct sig_host_AudioOut* leftOut;
struct sig_host_AudioOut* rightOut;

void buildWavetables() {
    sineTable = sig_Buffer_new(&allocator, WAVETABLE_LENGTH);
    sig_Buffer_fillWithWaveform(sineTable, sig_waveform_sine,
        WAVETABLE_LENGTH, 0.0f, 1.0f);

    wavetables = sig_AdditiveWavetableBank_new(&wavetableAllocator,
        shapes, sig_StandardWaveform_COUNT,
        NUM_WAVETABLE_LEVELS, WAVETABLE_LENGTH, LOWEST_WAVETABLE_FREQUENCY,
        MAX_HARMONIC_FREQUENCY, sig_HarmonicTaper_LANCZOS);
}

struct sig_dsp_Clamp* buildClamp(struct sig_SignalContext* context,
    struct sig_Status* status, float_array_ptr source, float min,
    float max) {
    struct sig_dsp_Clamp* clamp = sig_dsp_Clamp_new(&allocator, context);
    sig_List_append(&signals, clamp, status);
    clamp->inputs.source = source;
    clamp->parameters.min = min;
    clamp->parameters.max = max;

    return clamp;
}

void buildFrequencyGraph(struct sig_SignalContext* context,
    struct sig_Status* status) {
    frequencyKnob = sig_host_FilteredCVIn_new(&allocator, context);
    frequencyKnob->hardware = &host.device.hardware;
    sig_List_append(&signals, frequencyKnob, status);
    frequencyKnob->parameters.control = FREQUENCY_KNOB;
    frequencyKnob->parameters.scale = FUNDAMENTAL_HALF_RANGE * 2.0f;
    frequencyKnob->parameters.offset = -FUNDAMENTAL_HALF_RANGE;

    vOctCV = sig_host_CVIn_new(&allocator, context);
    vOctCV->hardware = &host.device.hardware;
    sig_List_append(&signals, vOctCV, status);
    vOctCV->parameters.control = V_OCT_CV;
    vOctCV->parameters.scale = 5.0f;

    pitch = sig_dsp_Add_new(&allocator, context);
    sig_List_append(&signals, pitch, status);
    pitch->inputs.left = frequencyKnob->outputs.main;
    pitch->inputs.right = vOctCV->outputs.main;

    clampedPitch = buildClamp(context, status, pitch->outputs.main,
        -FUNDAMENTAL_CV_HALF_RANGE, FUNDAMENTAL_CV_HALF_RANGE);

    fundamental = sig_dsp_LinearToFreq_new(&allocator, context);
    sig_List_append(&signals, fundamental, status);
    fundamental->inputs.source = clampedPitch->outputs.main;
    fundamental->parameters.middleFreq = FUNDAMENTAL_CENTRE_FREQUENCY;
}

void buildModulatorFrequencyGraph(struct sig_SignalContext* context,
    struct sig_Status* status) {
    ratioKnob = sig_host_FilteredCVIn_new(&allocator, context);
    ratioKnob->hardware = &host.device.hardware;
    sig_List_append(&signals, ratioKnob, status);
    ratioKnob->parameters.control = RATIO_KNOB;

    // The knob spans the whole ratio list, and +/-5V of CV spans half of it
    // in either direction.
    ratioCV = sig_host_FilteredCVIn_new(&allocator, context);
    ratioCV->hardware = &host.device.hardware;
    sig_List_append(&signals, ratioCV, status);
    ratioCV->parameters.control = RATIO_CV;
    ratioCV->parameters.scale = 0.5f;

    combinedRatio = sig_dsp_Add_new(&allocator, context);
    sig_List_append(&signals, combinedRatio, status);
    combinedRatio->inputs.left = ratioKnob->outputs.main;
    combinedRatio->inputs.right = ratioCV->outputs.main;

    // Clamping keeps the ratio at the ends of the list instead of wrapping
    // from 4:1 to 1:8, and limits the free modulator's range.
    clampedRatio = buildClamp(context, status, combinedRatio->outputs.main,
        0.0f, 1.0f);

    ratioNumerator = sig_dsp_List_new(&allocator, context);
    sig_List_append(&signals, ratioNumerator, status);
    ratioNumerator->list = &numeratorList;
    ratioNumerator->inputs.index = clampedRatio->outputs.main;

    ratioDenominator = sig_dsp_List_new(&allocator, context);
    sig_List_append(&signals, ratioDenominator, status);
    ratioDenominator->list = &denominatorList;
    ratioDenominator->inputs.index = clampedRatio->outputs.main;

    freeModulatorPitch = sig_dsp_ScaleOffset_new(&allocator, context);
    sig_List_append(&signals, freeModulatorPitch, status);
    freeModulatorPitch->inputs.source = clampedRatio->outputs.main;
    freeModulatorPitch->parameters.scale = FREE_MODULATOR_RANGE;
    freeModulatorPitch->parameters.offset = -FREE_MODULATOR_RANGE / 2.0f;

    freeModulatorFrequency = sig_dsp_LinearToFreq_new(&allocator, context);
    sig_List_append(&signals, freeModulatorFrequency, status);
    freeModulatorFrequency->inputs.source = freeModulatorPitch->outputs.main;
    freeModulatorFrequency->parameters.middleFreq = FUNDAMENTAL_CENTRE_FREQUENCY;

    // In free mode, the ratio is rarely a whole number over 1,
    // so the modulator's phase follows the carrier without locking to it.
    freeNumerator = sig_dsp_Div_new(&allocator, context);
    sig_List_append(&signals, freeNumerator, status);
    freeNumerator->inputs.left = freeModulatorFrequency->outputs.main;
    freeNumerator->inputs.right = fundamental->outputs.main;

    freeDenominator = sig_dsp_ConstantValue_new(&allocator, context, 1.0f);

    modeButton = sig_host_SwitchIn_new(&allocator, context);
    modeButton->hardware = &host.device.hardware;
    sig_List_append(&signals, modeButton, status);
    modeButton->parameters.control = sig_host_TOGGLE_1;

    freeMode = sig_dsp_ToggleGate_new(&allocator, context);
    sig_List_append(&signals, freeMode, status);
    freeMode->inputs.trigger = modeButton->outputs.main;

    led = sig_host_CVOut_new(&allocator, context);
    led->hardware = &host.device.hardware;
    sig_List_append(&signals, led, status);
    led->parameters.control = sig_host_CV_OUT_1;
    led->parameters.scale = 0.51f;
    led->inputs.source = freeMode->outputs.main;

    numerator = sig_dsp_Branch_new(&allocator, context);
    sig_List_append(&signals, numerator, status);
    numerator->inputs.condition = freeMode->outputs.main;
    numerator->inputs.on = freeNumerator->outputs.main;
    numerator->inputs.off = ratioNumerator->outputs.main;

    denominator = sig_dsp_Branch_new(&allocator, context);
    sig_List_append(&signals, denominator, status);
    denominator->inputs.condition = freeMode->outputs.main;
    denominator->inputs.on = freeDenominator->outputs.main;
    denominator->inputs.off = ratioDenominator->outputs.main;

    ratio = sig_dsp_Div_new(&allocator, context);
    sig_List_append(&signals, ratio, status);
    ratio->inputs.left = numerator->outputs.main;
    ratio->inputs.right = denominator->outputs.main;

    // The modulator's band-limited wavetables are selected by its frequency.
    modulatorFrequency = sig_dsp_Mul_new(&allocator, context);
    sig_List_append(&signals, modulatorFrequency, status);
    modulatorFrequency->inputs.left = fundamental->outputs.main;
    modulatorFrequency->inputs.right = ratio->outputs.main;
}

// The modulator's phase is locked to the carrier's phase, so that
// phase modulation at whole-number ratios produces no DC offset.
void buildPhaseGraph(struct sig_SignalContext* context,
    struct sig_Status* status) {
    carrierPhase = sig_dsp_Phasor_new(&allocator, context);
    sig_List_append(&signals, carrierPhase, status);
    carrierPhase->inputs.frequency = fundamental->outputs.main;

    modulatorPhase = sig_dsp_RatioPhasor_new(&allocator, context);
    sig_List_append(&signals, modulatorPhase, status);
    modulatorPhase->inputs.reference = carrierPhase->outputs.main;
    modulatorPhase->inputs.numerator = numerator->outputs.main;
    modulatorPhase->inputs.denominator = denominator->outputs.main;
    modulatorPhase->parameters.glide = RATIO_GLIDE;
}

// Low pass filters an audio input and scales it by the external phase
// modulation amount.
struct sig_dsp_BinaryOp* buildExternalPhaseModulation(
    struct sig_SignalContext* context, struct sig_Status* status,
    int channel) {
    struct sig_host_AudioIn* audioIn = sig_host_AudioIn_new(&allocator,
        context);
    audioIn->hardware = &host.device.hardware;
    sig_List_append(&signals, audioIn, status);
    audioIn->parameters.channel = channel;

    struct sig_dsp_SVFLowPass* lowPass = sig_dsp_SVFLowPass_new(&allocator,
        context);
    sig_List_append(&signals, lowPass, status);
    lowPass->inputs.source = audioIn->outputs.main;
    lowPass->inputs.frequency = audioInLowPassFrequency->outputs.main;
    lowPass->inputs.resonance = audioInLowPassResonance->outputs.main;

    struct sig_dsp_BinaryOp* phaseModulation = sig_dsp_Mul_new(&allocator,
        context);
    sig_List_append(&signals, phaseModulation, status);
    phaseModulation->inputs.left = lowPass->outputs.main;
    phaseModulation->inputs.right = phaseModulationAmount->outputs.main;

    return phaseModulation;
}

void buildModulationGraph(struct sig_SignalContext* context,
    struct sig_Status* status) {
    indexIn = sig_host_SummedCVIn_new(&allocator, context);
    indexIn->hardware = &host.device.hardware;
    sig_List_append(&signals, indexIn, status);
    indexIn->leftCVIn->parameters.control = INDEX_KNOB;
    indexIn->rightCVIn->parameters.control = INDEX_CV;

    // A negative index inverts the modulator, which sounds almost the same
    // as a positive one. Clamping at zero makes negative CV reduce the
    // index instead of increasing it again.
    clampedIndex = buildClamp(context, status, indexIn->outputs.main,
        0.0f, MAX_MODULATION_AMOUNT);

    // Discontinuities in the amplitude of a phase modulator
    // are really noticeable, so lots of smoothing is important.
    modulationIndex = sig_dsp_Smooth_new(&allocator, context);
    sig_List_append(&signals, modulationIndex, status);
    modulationIndex->inputs.source = clampedIndex->outputs.main;
    modulationIndex->parameters.time = 0.02f;

    audioInLowPassFrequency = sig_dsp_ConstantValue_new(&allocator, context,
        EXT_PHASE_MODULATION_LOW_PASS_FREQUENCY);
    audioInLowPassResonance = sig_dsp_ConstantValue_new(&allocator, context,
        LOW_PASS_RESONANCE);

    phaseModulationAmountIn = sig_host_SummedCVIn_new(&allocator, context);
    phaseModulationAmountIn->hardware = &host.device.hardware;
    sig_List_append(&signals, phaseModulationAmountIn, status);
    phaseModulationAmountIn->leftCVIn->parameters.control =
        EXT_PHASE_MODULATION_AMOUNT_KNOB;
    phaseModulationAmountIn->rightCVIn->parameters.control =
        EXT_PHASE_MODULATION_AMOUNT_CV;

    clampedPhaseModulationAmount = buildClamp(context, status,
        phaseModulationAmountIn->outputs.main, 0.0f, MAX_MODULATION_AMOUNT);

    phaseModulationAmount = sig_dsp_Smooth_new(&allocator, context);
    sig_List_append(&signals, phaseModulationAmount, status);
    phaseModulationAmount->inputs.source =
        clampedPhaseModulationAmount->outputs.main;
    phaseModulationAmount->parameters.time = 0.02f;

    // Each audio input modulates its own output's modulator.
    leftExternalPhaseModulation = buildExternalPhaseModulation(context,
        status, sig_host_AUDIO_IN_1);
    rightExternalPhaseModulation = buildExternalPhaseModulation(context,
        status, sig_host_AUDIO_IN_2);

    // The wavetable index can wrap around.
    // The knob spans the full bank, while CV can push it past either end,
    // crossfading between the last waveform and the first.
    float lastWaveform = sig_osc_WavetableBank_tableIndex(
        sig_StandardWaveform_COUNT - 1, sig_StandardWaveform_COUNT);

    waveformIn = sig_host_SummedCVIn_new(&allocator, context);
    waveformIn->hardware = &host.device.hardware;
    sig_List_append(&signals, waveformIn, status);
    waveformIn->leftCVIn->parameters.control = WAVEFORM_KNOB;
    waveformIn->rightCVIn->parameters.control = WAVEFORM_CV;
    waveformIn->leftCVIn->parameters.scale = lastWaveform;
    waveformIn->rightCVIn->parameters.scale = lastWaveform;

    // Smooth out steps and ADC noise that cause zipper noise when crossfading the waveform.
    waveform = sig_dsp_Smooth_new(&allocator, context);
    sig_List_append(&signals, waveform, status);
    waveform->inputs.source = waveformIn->outputs.main;
    waveform->parameters.time = 0.02f;
}

// First output: carrier is the selected waveform, modulator is a sine.
void buildWaveformCarrierVoice(struct sig_SignalContext* context,
    struct sig_Status* status) {
    sineModulator = sig_dsp_WavetableReader_new(&allocator, context);
    sig_List_append(&signals, sineModulator, status);
    sineModulator->wavetable = sineTable;
    sineModulator->inputs.phase = modulatorPhase->outputs.main;
    sineModulator->inputs.phaseOffset =
        leftExternalPhaseModulation->outputs.main;
    sineModulator->inputs.mul = modulationIndex->outputs.main;

    waveformCarrier = sig_dsp_AdditiveWavetableBankReader_new(
        &allocator, context);
    sig_List_append(&signals, waveformCarrier, status);
    waveformCarrier->wavetables = wavetables;
    waveformCarrier->parameters.peakRelease = PEAK_RELEASE;
    waveformCarrier->inputs.phase = carrierPhase->outputs.main;
    waveformCarrier->inputs.phaseOffset = sineModulator->outputs.main;
    waveformCarrier->inputs.frequency = fundamental->outputs.main;
    waveformCarrier->inputs.tableIndex = waveform->outputs.main;
}

// Second output is the opposite: carrier is a sine, modulator is the selected waveform.
void buildWaveformModulatorVoice(struct sig_SignalContext* context,
    struct sig_Status* status) {
    waveformModulator = sig_dsp_AdditiveWavetableBankReader_new(
        &allocator, context);
    sig_List_append(&signals, waveformModulator, status);
    waveformModulator->wavetables = wavetables;
    waveformModulator->parameters.peakRelease = PEAK_RELEASE;
    waveformModulator->inputs.phase = modulatorPhase->outputs.main;
    waveformModulator->inputs.phaseOffset =
        rightExternalPhaseModulation->outputs.main;
    waveformModulator->inputs.frequency = modulatorFrequency->outputs.main;
    waveformModulator->inputs.tableIndex = waveform->outputs.main;
    waveformModulator->inputs.mul = modulationIndex->outputs.main;

    sineCarrier = sig_dsp_WavetableReader_new(&allocator, context);
    sig_List_append(&signals, sineCarrier, status);
    sineCarrier->wavetable = sineTable;
    sineCarrier->inputs.phase = carrierPhase->outputs.main;
    sineCarrier->inputs.phaseOffset = waveformModulator->outputs.main;
}

struct sig_dsp_SVFLowPass* buildLowPass(struct sig_SignalContext* context,
    struct sig_Status* status, float_array_ptr source) {
    struct sig_dsp_SVFLowPass* lowPass = sig_dsp_SVFLowPass_new(&allocator,
        context);
    sig_List_append(&signals, lowPass, status);
    lowPass->inputs.source = source;
    lowPass->inputs.frequency = lowPassFrequency->outputs.main;
    lowPass->inputs.resonance = lowPassResonance->outputs.main;

    return lowPass;
}

struct sig_host_AudioOut* buildOutput(struct sig_SignalContext* context,
    struct sig_Status* status, float_array_ptr source,
    struct sig_dsp_DCBlock** dcBlocker, int channel) {
    *dcBlocker = sig_dsp_DCBlock_new(&allocator, context);
    sig_List_append(&signals, *dcBlocker, status);
    (*dcBlocker)->inputs.source = source;
    (*dcBlocker)->parameters.frequency = DC_BLOCK_FREQUENCY;

    // Removing a DC offset shifts the waveform, which raises its peaks.
    // The soft clipper limits those peaks without distorting quieter signals.
    struct sig_dsp_SoftClip* softClip = sig_dsp_SoftClip_new(&allocator,
        context);
    sig_List_append(&signals, softClip, status);
    softClip->parameters.threshold = SOFT_CLIP_THRESHOLD;
    softClip->inputs.source = (*dcBlocker)->outputs.main;

    struct sig_host_AudioOut* out = sig_host_AudioOut_new(&allocator,
        context);
    out->hardware = &host.device.hardware;
    sig_List_append(&signals, out, status);
    out->parameters.channel = channel;
    out->inputs.source = softClip->outputs.main;

    return out;
}

void buildSignalGraph(struct sig_SignalContext* context,
    struct sig_Status* status) {
    buildFrequencyGraph(context, status);
    buildModulatorFrequencyGraph(context, status);
    buildPhaseGraph(context, status);
    buildModulationGraph(context, status);
    buildWaveformCarrierVoice(context, status);
    buildWaveformModulatorVoice(context, status);

    lowPassFrequency = sig_dsp_ConstantValue_new(&allocator, context,
        LOW_PASS_FREQUENCY);
    lowPassResonance = sig_dsp_ConstantValue_new(&allocator, context,
        LOW_PASS_RESONANCE);
    leftLowPass = buildLowPass(context, status,
        waveformCarrier->outputs.main);
    rightLowPass = buildLowPass(context, status, sineCarrier->outputs.main);

    leftOut = buildOutput(context, status, leftLowPass->outputs.main,
        &leftDCBlocker, sig_host_AUDIO_OUT_1);
    rightOut = buildOutput(context, status, rightLowPass->outputs.main,
        &rightDCBlocker, sig_host_AUDIO_OUT_2);
}

int main(void) {
    allocator.impl->init(&allocator);

    struct sig_AudioSettings audioSettings = {
        .sampleRate = SAMPLERATE,
        .numChannels = 2,
        .blockSize = BLOCK_SIZE
    };

    struct sig_Status status;
    sig_Status_init(&status);
    sig_List_init(&signals, (void**) &listStorage, MAX_NUM_SIGNALS);

    struct sig_SignalContext* context = sig_SignalContext_new(&allocator,
        &audioSettings);
    evaluator = sig_dsp_SignalListEvaluator_new(&allocator, &signals);
    host.Init(&audioSettings, (struct sig_dsp_SignalEvaluator*) evaluator);

    // The SDRAM is only available after the board has been initialized.
    wavetableAllocator.impl->init(&wavetableAllocator);
    buildWavetables();
    buildSignalGraph(context, &status);

    host.Start();

    while (1) {}
}
