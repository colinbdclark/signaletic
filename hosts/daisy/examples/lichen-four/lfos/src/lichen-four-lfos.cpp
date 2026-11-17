#include <libsignaletic.h>
#include "../../../../include/lichen-four-device.hpp"
#include "../../../shared/include/summed-cv-in.h"
#include "../../../shared/include/scaled-clock-frequency.h"

using namespace lichen::four;
using namespace sig::libdaisy;

#define SAMPLERATE 48000
#define BLOCK_SIZE 96
#define WAVETABLE_SIZE 2048
#define HEAP_SIZE 1024 * 384 // 384 KB
#define MAX_NUM_SIGNALS 64
#define NUM_DIVISION_STEPS 37
#define NUM_MULTIPLICATION_STEPS 35

// The index of the 1:1 ratio in the division list, which LFOs 2 and 3 use.
// Their slider covers this many ratios above 1:1, and CV can push it this
// many ratios in either direction.
// The list needs this many ratios below 1:1 and twice as many above it.
#define UNITY_DIVISION_STEP 12

// LFOs 1 and 4 can be transposed by V/Oct CV. Within this distance of a
// whole number of octaves, the transposition snaps to it, which keeps the
// LFO locked to the clock. In octaves, which at 1 V/octave are volts.
// TODO: Tune this tolerance to the noise of the CV inputs.
#define OCTAVE_SNAP_TOLERANCE 0.01f

// Within this distance of a whole table, the waveform index snaps to it,
// which makes each pure waveform easy to select despite noise and offsets
// in the knob and CV. In tables.
#define WAVEFORM_SNAP_TOLERANCE 0.01f

// Smooths out the steps and snapping in the waveform index, which cause
// zipper noise when the waveform changes, in seconds.
#define WAVEFORM_SMOOTHING_TIME 0.02f

// A fast clock multiplied by a high ratio and transposed up by CV can exceed
// the Nyquist frequency, so LFOs 1 and 4 are limited to this frequency,
// in Hz.
#define MAX_LFO_FREQUENCY 20000.0f

#define WAVETABLE_HEAP_SIZE 1024 * 1024 // 1 MB

// When an LFO's ratio changes, its phase glides into alignment with the
// clock over this time, in seconds.
#define RATIO_GLIDE 0.5f

// LFOs 1 and 4 go up to audio rate, so their wavetables are bandlimited.
// Below about 10 Hz, the table length limits the number of harmonics that
// can be represented, so the lowest level starts at 8 Hz.
// The highest level covers 8-16 kHz, and harmonics are always below 20 kHz.
#define NUM_BANDLIMITED_LEVELS 11
#define LOWEST_BANDLIMITED_FREQUENCY 8.0f
#define MAX_HARMONIC_FREQUENCY 20000.0f

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

struct sig_dsp_Signal* listStorage[MAX_NUM_SIGNALS];
struct sig_List signals;
struct sig_dsp_SignalListEvaluator* evaluator;
DaisyHost<FourDevice> host;

// Each LFO runs at a ratio of the clock's frequency,
// numerators[i]:denominators[i].
// Below 1:1, the slow LFOs speed up through more closely spaced ratios,
// stopping at 5:1 so that their CV outputs stay well below audio rate.
float divisionNumerators[NUM_DIVISION_STEPS] = {
    5.0f, 9.0f, 4.0f, 7.0f, 3.0f, 5.0f, 2.0f, 5.0f,
    3.0f, 4.0f, 5.0f, 6.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};

float divisionDenominators[NUM_DIVISION_STEPS] = {
    1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 3.0f,
    2.0f, 3.0f, 4.0f, 5.0f, 1.0f, 2.0f, 3.0f, 4.0f,
    5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 13.0f,
    17.0f, 19.0f, 20.0f, 23.0f, 29.0f, 31.0f, 67.0f, 100.0f,
    131.0f, 200.0f, 263.0f, 300.0f, 523.0f
};

struct sig_Buffer divisionNumeratorList = {
    .length = NUM_DIVISION_STEPS,
    .samples = divisionNumerators
};

struct sig_Buffer divisionDenominatorList = {
    .length = NUM_DIVISION_STEPS,
    .samples = divisionDenominators
};

// LFOs 1 and 4 can be transposed by octaves with CV, so their list stops
// at 31:1. The ratios below 1:1 mirror the ratios above it, so that 1:1 is
// at the centre of the slider.
float multiplicationNumerators[NUM_MULTIPLICATION_STEPS] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
    8.0f, 9.0f, 10.0f, 11.0f, 13.0f, 17.0f, 19.0f, 20.0f,
    23.0f, 29.0f, 31.0f
};

float multiplicationDenominators[NUM_MULTIPLICATION_STEPS] = {
    31.0f, 29.0f, 23.0f, 20.0f, 19.0f, 17.0f, 13.0f, 11.0f,
    10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f,
    2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f
};

struct sig_Buffer multiplicationNumeratorList = {
    .length = NUM_MULTIPLICATION_STEPS,
    .samples = multiplicationNumerators
};

struct sig_Buffer multiplicationDenominatorList = {
    .length = NUM_MULTIPLICATION_STEPS,
    .samples = multiplicationDenominators
};

struct sig_WavetableBank* lfoWavetables;
struct sig_AdditiveWavetableBank* bandlimitedWavetables;
struct sig_host_GateIn* clockIn;
struct sig_host_SwitchIn* tapTempoButton;
struct sig_dsp_ClockSource* clock;
struct sig_dsp_ClockPhasor* clockPhase;
struct sig_dsp_AdditiveWavetableBankReader* lfo1;
struct sig_dsp_WavetableBankReader* lfo2;
struct sig_dsp_WavetableBankReader* lfo3;
struct sig_dsp_AdditiveWavetableBankReader* lfo4;
struct sig_host_AudioOut* out1;
struct sig_host_CVOut* out2;
struct sig_host_CVOut* out3;
struct sig_host_AudioOut* out4;
struct sig_host_GateOut* clockOut;
struct sig_host_GateOut* led;

// The LFOs use sine-phase waveforms, so that every waveform starts its cycle
// at the clock pulse, and crossfading between waveforms does not reduce
// the LFO's level.
// TODO: Refactor this into a library function.
void buildWavetables() {
    lfoWavetables = sig_WavetableBank_new(&allocator,
        sig_StandardWaveform_COUNT, WAVETABLE_SIZE);

    for (size_t i = 0; i < lfoWavetables->length; i++) {
        struct sig_Buffer* wavetable = lfoWavetables->waves[i];
        sig_Buffer_fillWithWaveform(wavetable,
            sig_SINE_PHASE_WAVEFORMS[i].generate, WAVETABLE_SIZE, 0.0f, 1.0f);
    }

    // The SDRAM is only available after the board has been initialized.
    wavetableAllocator.impl->init(&wavetableAllocator);
    bandlimitedWavetables = sig_AdditiveWavetableBank_new(
        &wavetableAllocator, sig_SINE_PHASE_WAVEFORMS,
        sig_StandardWaveform_COUNT, NUM_BANDLIMITED_LEVELS, WAVETABLE_SIZE,
        LOWEST_BANDLIMITED_FREQUENCY, MAX_HARMONIC_FREQUENCY,
        sig_HarmonicTaper_LANCZOS);
}

struct sig_host_SummedCVIn* buildSummedCVIn(
    struct sig_SignalContext* context, struct sig_Status* status,
    int knob, int cv, float scale) {
    struct sig_host_SummedCVIn* summedCVIn = sig_host_SummedCVIn_new(
        &allocator, context);
    summedCVIn->hardware = &host.device.hardware;
    sig_List_append(&signals, summedCVIn, status);
    summedCVIn->leftCVIn->parameters.control = knob;
    summedCVIn->rightCVIn->parameters.control = cv;
    summedCVIn->leftCVIn->parameters.scale = scale;
    summedCVIn->rightCVIn->parameters.scale = scale;

    return summedCVIn;
}

// The wavetable index is cyclic, so the waveform knob and CV span
// up to the last table rather than crossfading back to the first.
// The index is snapped to whole tables, then converted to cycles of the
// bank, which the wavetable readers use.
struct sig_dsp_Smooth* buildWaveformCV(
    struct sig_SignalContext* context, struct sig_Status* status,
    int knob, int cv, size_t numTables) {
    float lastTable = (float) (numTables - 1);
    struct sig_host_SummedCVIn* tables = buildSummedCVIn(context, status,
        knob, cv, lastTable);

    struct sig_dsp_SnapToInteger* snappedTables = sig_dsp_SnapToInteger_new(
        &allocator, context);
    sig_List_append(&signals, snappedTables, status);
    snappedTables->inputs.source = tables->outputs.main;
    snappedTables->parameters.tolerance = WAVEFORM_SNAP_TOLERANCE;

    struct sig_dsp_ScaleOffset* tableIndex = sig_dsp_ScaleOffset_new(
        &allocator, context);
    sig_List_append(&signals, tableIndex, status);
    tableIndex->inputs.source = snappedTables->outputs.main;
    tableIndex->parameters.scale = sig_osc_WavetableBank_tableIndex(1,
        numTables);

    struct sig_dsp_Smooth* smoothedTableIndex = sig_dsp_Smooth_new(
        &allocator, context);
    sig_List_append(&signals, smoothedTableIndex, status);
    smoothedTableIndex->inputs.source = tableIndex->outputs.main;
    smoothedTableIndex->parameters.time = WAVEFORM_SMOOTHING_TIME;

    return smoothedTableIndex;
}

// An LFO's phase runs at a ratio of the clock's phase, selected by the
// index from the numerators and denominators lists. When the ratio is a
// whole number over a whole number, the LFO's phase is locked to the clock.
struct sig_dsp_RatioPhasor* buildRatioPhasor(
    struct sig_SignalContext* context, struct sig_Status* status,
    struct sig_Buffer* numerators, struct sig_Buffer* denominators,
    float_array_ptr index) {
    // The lists step between ratios without interpolating,
    // because only whole-number ratios lock to the clock.
    struct sig_dsp_List* numerator = sig_dsp_List_new(&allocator, context);
    sig_List_append(&signals, numerator, status);
    numerator->list = numerators;
    numerator->parameters.wrap = 0.0f;
    numerator->inputs.index = index;

    struct sig_dsp_List* denominator = sig_dsp_List_new(&allocator, context);
    sig_List_append(&signals, denominator, status);
    denominator->list = denominators;
    denominator->parameters.wrap = 0.0f;
    denominator->inputs.index = index;

    struct sig_dsp_RatioPhasor* phase = sig_dsp_RatioPhasor_new(&allocator,
        context);
    sig_List_append(&signals, phase, status);
    phase->inputs.reference = clockPhase->outputs.main;
    phase->inputs.numerator = numerator->outputs.main;
    phase->inputs.denominator = denominator->outputs.main;
    phase->parameters.glide = RATIO_GLIDE;

    return phase;
}

// For LFOs 2 and 3, the slider and CV together select the ratio.
struct sig_dsp_RatioPhasor* buildLFOPhase(struct sig_SignalContext* context,
    struct sig_Status* status, struct sig_Buffer* numerators,
    struct sig_Buffer* denominators, int knob, int cv) {
    float unityIndex = (float) UNITY_DIVISION_STEP /
        (float) (NUM_DIVISION_STEPS - 1);
    struct sig_host_SummedCVIn* ratioCV = buildSummedCVIn(context,
        status, knob, cv, unityIndex);
    ratioCV->rightCVIn->parameters.offset = unityIndex;

    return buildRatioPhasor(context, status, numerators, denominators,
        ratioCV->outputs.main);
}

// For LFOs 1 and 4, the slider selects the ratio from the whole list,
// and V/Oct CV transposes it smoothly. When the CV is close to a whole
// number of volts, the LFO stays locked to the clock.
struct sig_dsp_RatioPhasor* buildTransposableLFOPhase(
    struct sig_SignalContext* context, struct sig_Status* status,
    struct sig_Buffer* numerators, struct sig_Buffer* denominators,
    int knob, int cv) {
    struct sig_host_FilteredCVIn* ratioKnob = sig_host_FilteredCVIn_new(
        &allocator, context);
    ratioKnob->hardware = &host.device.hardware;
    sig_List_append(&signals, ratioKnob, status);
    ratioKnob->parameters.control = knob;

    struct sig_dsp_RatioPhasor* phase = buildRatioPhasor(context, status,
        numerators, denominators, ratioKnob->outputs.main);

    struct sig_host_CVIn* vOctCV = sig_host_CVIn_new(&allocator, context);
    vOctCV->hardware = &host.device.hardware;
    sig_List_append(&signals, vOctCV, status);
    vOctCV->parameters.control = cv;
    vOctCV->parameters.scale = 5.0f;

    struct sig_dsp_SnapToInteger* octaves = sig_dsp_SnapToInteger_new(
        &allocator, context);
    sig_List_append(&signals, octaves, status);
    octaves->inputs.source = vOctCV->outputs.main;
    octaves->parameters.tolerance = OCTAVE_SNAP_TOLERANCE;

    phase->inputs.octaves = octaves->outputs.main;
    phase->parameters.maxFrequency = MAX_LFO_FREQUENCY;

    return phase;
}

// The band-limited wavetables are selected by the LFO's frequency,
// which is the clock's frequency scaled by the LFO's transposed ratio.
struct sig_dsp_ScaledClockFrequency* buildLFOFrequency(
    struct sig_SignalContext* context, struct sig_Status* status,
    struct sig_dsp_RatioPhasor* phase) {
    struct sig_dsp_BinaryOp* ratio = sig_dsp_Div_new(&allocator, context);
    sig_List_append(&signals, ratio, status);
    ratio->inputs.left = phase->inputs.numerator;
    ratio->inputs.right = phase->inputs.denominator;

    struct sig_dsp_LinearToFreq* transposition = sig_dsp_LinearToFreq_new(
        &allocator, context);
    sig_List_append(&signals, transposition, status);
    transposition->inputs.source = phase->inputs.octaves;
    transposition->parameters.middleFreq = 1.0f;

    struct sig_dsp_BinaryOp* transposedRatio = sig_dsp_Mul_new(&allocator,
        context);
    sig_List_append(&signals, transposedRatio, status);
    transposedRatio->inputs.left = ratio->outputs.main;
    transposedRatio->inputs.right = transposition->outputs.main;

    struct sig_dsp_ScaledClockFrequency* frequency =
        sig_dsp_ScaledClockFrequency_new(&allocator, context);
    sig_List_append(&signals, frequency, status);
    frequency->inputs.clock = clock->outputs.main;
    frequency->inputs.scale = transposedRatio->outputs.main;

    return frequency;
}

struct sig_dsp_WavetableBankReader* buildLFO(
    struct sig_SignalContext* context, struct sig_Status* status,
    struct sig_Buffer* numerators, struct sig_Buffer* denominators,
    int ratioKnob, int ratioCV, int waveformKnob, int waveformCV) {
    struct sig_dsp_RatioPhasor* phase = buildLFOPhase(context, status,
        numerators, denominators, ratioKnob, ratioCV);
    struct sig_dsp_Smooth* waveform = buildWaveformCV(context, status,
        waveformKnob, waveformCV, sig_StandardWaveform_COUNT);

    struct sig_dsp_WavetableBankReader* lfo =
        sig_dsp_WavetableBankReader_new(&allocator, context);
    sig_List_append(&signals, lfo, status);
    lfo->wavetables = lfoWavetables;
    lfo->inputs.phase = phase->outputs.main;
    lfo->inputs.tableIndex = waveform->outputs.main;

    return lfo;
}

struct sig_dsp_AdditiveWavetableBankReader* buildBandlimitedLFO(
    struct sig_SignalContext* context, struct sig_Status* status,
    struct sig_Buffer* numerators, struct sig_Buffer* denominators,
    int ratioKnob, int ratioCV, int waveformKnob, int waveformCV) {
    struct sig_dsp_RatioPhasor* phase = buildTransposableLFOPhase(context,
        status, numerators, denominators, ratioKnob, ratioCV);
    struct sig_dsp_ScaledClockFrequency* frequency = buildLFOFrequency(
        context, status, phase);
    struct sig_dsp_Smooth* waveform = buildWaveformCV(context, status,
        waveformKnob, waveformCV, sig_StandardWaveform_COUNT);

    struct sig_dsp_AdditiveWavetableBankReader* lfo =
        sig_dsp_AdditiveWavetableBankReader_new(&allocator, context);
    sig_List_append(&signals, lfo, status);
    lfo->wavetables = bandlimitedWavetables;
    lfo->inputs.phase = phase->outputs.main;
    lfo->inputs.frequency = frequency->outputs.main;
    lfo->inputs.tableIndex = waveform->outputs.main;

    return lfo;
}

void buildSignalGraph(struct sig_SignalContext* context,
    struct sig_Status* status) {
    clockIn = sig_host_GateIn_new(&allocator, context);
    clockIn->hardware = &host.device.hardware;
    sig_List_append(&signals, clockIn, status);
    clockIn->parameters.control = sig_host_GATE_IN_1;

    tapTempoButton = sig_host_SwitchIn_new(&allocator, context);
    tapTempoButton->hardware = &host.device.hardware;
    sig_List_append(&signals, tapTempoButton, status);
    tapTempoButton->parameters.control = sig_host_TOGGLE_1;

    clock = sig_dsp_ClockSource_new(&allocator, context);
    sig_List_append(&signals, clock, status);
    clock->inputs.pulse = clockIn->outputs.main;
    clock->inputs.tap = tapTempoButton->outputs.main;

    // The clock phase has no downbeat. It locks each LFO to the clock's
    // pulses, but not to any particular pulse.
    clockPhase = sig_dsp_ClockPhasor_new(&allocator, context);
    sig_List_append(&signals, clockPhase, status);
    clockPhase->inputs.pulse = clock->outputs.main;

    buildWavetables();

    lfo1 = buildBandlimitedLFO(context, status,
        &multiplicationNumeratorList, &multiplicationDenominatorList,
        sig_host_KNOB_1, sig_host_CV_IN_8, sig_host_KNOB_5, sig_host_CV_IN_1);

    out1 = sig_host_AudioOut_new(&allocator, context);
    out1->hardware = &host.device.hardware;
    sig_List_append(&signals, out1, status);
    out1->parameters.channel = sig_host_AUDIO_OUT_1;
    out1->inputs.source = lfo1->outputs.main;

    lfo2 = buildLFO(context, status,
        &divisionNumeratorList, &divisionDenominatorList,
        sig_host_KNOB_2, sig_host_CV_IN_7, sig_host_KNOB_6, sig_host_CV_IN_2);

    out2 = sig_host_CVOut_new(&allocator, context);
    out2->hardware = &host.device.hardware;
    sig_List_append(&signals, out2, status);
    out2->parameters.control = sig_host_CV_OUT_1;
    out2->inputs.source = lfo2->outputs.main;

    lfo3 = buildLFO(context, status,
        &divisionNumeratorList, &divisionDenominatorList,
        sig_host_KNOB_3, sig_host_CV_IN_5, sig_host_KNOB_7, sig_host_CV_IN_3);

    out3 = sig_host_CVOut_new(&allocator, context);
    out3->hardware = &host.device.hardware;
    sig_List_append(&signals, out3, status);
    out3->parameters.control = sig_host_CV_OUT_2;
    out3->inputs.source = lfo3->outputs.main;

    lfo4 = buildBandlimitedLFO(context, status,
        &multiplicationNumeratorList, &multiplicationDenominatorList,
        sig_host_KNOB_4, sig_host_CV_IN_6, sig_host_KNOB_8, sig_host_CV_IN_4);

    out4 = sig_host_AudioOut_new(&allocator, context);
    out4->hardware = &host.device.hardware;
    sig_List_append(&signals, out4, status);
    out4->parameters.channel = sig_host_AUDIO_OUT_2;
    out4->inputs.source = lfo4->outputs.main;

    clockOut = sig_host_GateOut_new(&allocator, context);
    clockOut->hardware = &host.device.hardware;
    sig_List_append(&signals, clockOut, status);
    clockOut->parameters.control = sig_host_GATE_OUT_1;
    clockOut->inputs.source = clock->outputs.main;

    led = sig_host_GateOut_new(&allocator, context);
    led->hardware = &host.device.hardware;
    sig_List_append(&signals, led, status);
    led->parameters.control = sig_host_GATE_OUT_2;
    led->inputs.source = clock->outputs.main;
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

    buildSignalGraph(context, &status);

    host.Start();

    while (1) {}
}
