#include <math.h>   // For powf, fmodf, sinf, roundf, fabsf
#include <stdlib.h> // For labs
#include <tlsf.h>   // Includes assert.h, limits.h, stddef.h
                    // stdio.h, stdlib.h, string.h (for errors etc.)
#include <libsignaletic.h>

// libsignaletic.h defines inline functions that can be inlined from any file.
// The extern inline declarations below make this the one file that provides each
// function's linkable definition, so calls that the compiler does not inline can
// still be linked.

extern inline float sig_fminf(float a, float b);

extern inline float sig_fmaxf(float a, float b);

extern inline float sig_clamp(float value, float min, float max);

extern inline float sig_snapToInteger(float value, float tolerance);

extern inline float sig_fastMod1f(float x);

extern inline float sig_flooredfmodf(float numer, float denom);

struct sig_Random* sig_Random_new(struct sig_Allocator* allocator,
    uint32_t seed) {
    struct sig_Random* self = sig_MALLOC(allocator,
        struct sig_Random);
    sig_Random_init(self, seed);

    return self;
}

void sig_Random_init(struct sig_Random* self, uint32_t seed) {
    sig_Random_seed(self, seed);
}

void sig_Random_seed(struct sig_Random* self, uint32_t seed) {
    self->seed = seed;
    // Expand and diffuse the seed into four state words
    // using SplitMix32.
    // Implementation from Kaito Udagawa's CC0 splitmix32.c
    // https://github.com/umireon/my-random-stuff/blob/master/xorshift/splitmix32.c
    for (size_t i = 0; i < 4; i++) {
        seed += 0x9e3779b9;
        uint32_t z = seed;
        z = (z ^ (z >> 16)) * 0x85ebca6b;
        z = (z ^ (z >> 13)) * 0xc2b2ae35;
        self->state[i] = z ^ (z >> 16);
    }
}

uint32_t sig_Random_nextU32(struct sig_Random* self) {
    // Directly from Black & Vigna's public domain implementation:
    // https://prng.di.unimi.it/xoshiro128plus.c
    uint32_t* s = self->state;
    const uint32_t result = s[0] + s[3];
	const uint32_t t = s[1] << 9;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];
	s[2] ^= t;
	s[3] = (s[3] << 11) | (s[3] >> (32 - 11));

	return result;
}

float sig_Random_nextFloat(struct sig_Random* self) {
    // Only take the top 24 bits:
    // 1. because xoshiro128+ has low linear complexity in
    // the lowest four bits.
    // 2. so that every output is an exact multiple
    // of 2^-24, avoiding float rounding artifacts.
    return (sig_Random_nextU32(self) >> 8) * (1.0f / 16777216.0f);
}

float sig_Random_generate(struct sig_Random* self) {
    return (sig_Random_nextU32(self) >> 8) * (1.0f / 16777215.0f);
}

void sig_Random_destroy(struct sig_Allocator* allocator,
    struct sig_Random* self) {
    allocator->impl->free(allocator, self);
}

// The global default generator used by sig_randf.
static struct sig_Random sig_Random_GLOBAL_GENERATOR = {
    .seed = 1,
    .state = {0x5E2D1772, 0x14E498F0, 0xD20EA1FD, 0xB382F339}
};

float sig_randf(void) {
    return sig_Random_generate(&sig_Random_GLOBAL_GENERATOR);
}

void sig_randf_seed(uint32_t seed) {
    sig_Random_seed(&sig_Random_GLOBAL_GENERATOR, seed);
}

extern inline float sig_fastTanhf(float x);

extern inline float sig_linearMap(float value,
    float fromMin, float fromMax, float toMin, float toMax);

extern inline uint16_t sig_unipolarToUint12(float sample);

extern inline uint16_t sig_bipolarToUint12(float sample);

extern inline uint16_t sig_bipolarToInvUint12(float sample);

extern inline float sig_uint16ToBipolar(uint16_t sample);

extern inline float sig_uint16ToUnipolar(uint16_t sample);

extern inline float sig_invUint16ToBipolar(uint16_t sample);

float sig_midiToFreq(float midiNum) {
    return powf(2, (midiNum - 69.0f) / 12.0f) * 440.0f;
}

float sig_freqToMidi(float frequency) {
    return(12.0f * logf(frequency / 440.0f) / sig_LOG2) + 69.0f;
}

float sig_linearToFreq(float vOct, float middleFreq) {
    return middleFreq * powf(2, vOct);
}

float sig_freqToLinear(float freq, float middleFreq) {
    return (logf(freq / middleFreq) / sig_LOG2);
}

extern inline float sig_sum(float_array_ptr values,
    size_t length);

extern inline size_t sig_indexOfMin(float_array_ptr values,
    size_t length);

extern inline size_t sig_indexOfMax(float_array_ptr values,
    size_t length);

float sig_randomFill(size_t i, float_array_ptr array) {
    return sig_randf();
}

void sig_fill(float_array_ptr array, size_t length,
    sig_array_filler filler) {
    for (size_t i = 0; i < length; i++) {
        FLOAT_ARRAY(array)[i] = filler(i, array);
    }
}

void sig_fillWithValue(float_array_ptr array, size_t size,
    float value) {
    for (size_t i = 0; i < size; i++) {
        FLOAT_ARRAY(array)[i] = value;
    }
}

void sig_fillWithSilence(float_array_ptr array, size_t size) {
    sig_fillWithValue(array, size, 0.0f);
}

// TODO: Unit tests.
float sig_interpolate_linear(float idx, float_array_ptr table,
    size_t length) {
    int32_t idxIntegral = (int32_t) idx;
    float idxFractional = idx - (float) idxIntegral;
    size_t aIdx = (size_t) idxIntegral % length;
    float a = FLOAT_ARRAY(table)[aIdx];
    float b = FLOAT_ARRAY(table)[(aIdx + 1) % length];

    return a + (b - a) * idxFractional;
}

// TODO: Unit tests.
float sig_interpolate_cubic(float idx, float_array_ptr table,
    size_t length) {
    size_t idxIntegral = (size_t) idx;
    float idxFractional = idx - (float) idxIntegral;
    const size_t i0 = idxIntegral % length;
    const float xm1 = FLOAT_ARRAY(table)[i0 > 0 ? i0 - 1 : length - 1];
    const float x0 = FLOAT_ARRAY(table)[i0];
    const float x1 = FLOAT_ARRAY(table)[(i0 + 1) % length];
    const float x2 = FLOAT_ARRAY(table)[(i0 + 2) % length];
    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float bNeg = w + a;

    return (((a * idxFractional) - bNeg) * idxFractional + c) *
        idxFractional + x0;
}

extern inline float sig_filter_mean(float_array_ptr values,
    size_t length);

extern inline float sig_filter_meanExcludeMinMax(
    float_array_ptr values, size_t length);

extern inline float sig_filter_ema(float current, float previous,
    float a);

extern inline float sig_filter_onepole(float current,
    float previous, float b0, float a1);

extern inline float sig_filter_onepole_HPF_calculateA1(
    float frequency, float sampleRate);

extern inline float sig_filter_onepole_HPF_calculateB0(float a1);

extern inline float sig_filter_onepole_LPF_calculateA1(
    float frequency, float sampleRate);

extern inline float sig_filter_onepole_LPF_calculateB0(float a1);

extern inline float sig_filter_smooth(float current,
    float previous, float coeff);

extern inline float sig_filter_smooth_calculateCoefficient(
    float time, float sampleRate);

extern inline float sig_filter_dcBlock(float current,
    float previousInput, float previousOutput, float r);

extern inline float sig_filter_dcBlock_calculateCoefficient(
    float frequency, float sampleRate);

extern inline float sig_filter_svf_highPass(float input, float band,
    float low, float k);

extern inline float sig_filter_svf_notch(float input, float band,
    float low, float k);

extern inline float sig_filter_svf_peak(float input, float band,
    float low, float k);

extern inline float sig_filter_svf_allPass(float input, float band,
    float low, float k);

void sig_filter_Smooth_init(struct sig_filter_Smooth* self,
    float coeff) {
    self->coeff = coeff;
    self->previous = 0;
}

extern inline float sig_filter_Smooth_generate(
    struct sig_filter_Smooth* self, float value);

void sig_filter_DCBlock_init(struct sig_filter_DCBlock* self,
    float frequency, float sampleRate) {
    self->r = sig_filter_dcBlock_calculateCoefficient(frequency, sampleRate);
    self->previousInput = 0.0f;
    self->previousOutput = 0.0f;
}

extern inline float sig_filter_DCBlock_generate(
    struct sig_filter_DCBlock* self, float value);

void sig_filter_SVF_init(struct sig_filter_SVF* self) {
    self->k = 1.0f;
    self->a1 = 0.0f;
    self->a2 = 0.0f;
    self->a3 = 0.0f;
    self->ic1eq = 0.0f;
    self->ic2eq = 0.0f;
}

extern inline void sig_filter_SVF_calculateCoefficients(
    struct sig_filter_SVF* self, float frequency, float resonance,
    float sampleRate);

extern inline void sig_filter_SVF_generate(struct sig_filter_SVF* self,
    float value, struct sig_filter_SVF_Taps* taps);

void sig_filter_Ladder_init(struct sig_filter_Ladder* self,
    uint8_t interpolation) {
    self->interpolation = interpolation;
    self->interpolationRecip = 1.0f / interpolation;
    self->alpha = 1.0f;
    self->qAdjust = 1.0f;
    self->z0[0] = self->z0[1] = self->z0[2] = self->z0[3] = 0.0f;
    self->z1[0] = self->z1[1] = self->z1[2] = self->z1[3] = 0.0f;
    self->previousInput = 0.0f;
}

extern inline void sig_filter_Ladder_calculateCoefficients(
    struct sig_filter_Ladder* self, float frequency, float sampleRate);

extern inline float sig_filter_Ladder_calcStage(
    struct sig_filter_Ladder* self, float s, uint8_t i);

extern inline float sig_filter_Ladder_calcStageInput(
    struct sig_filter_Ladder* self, float value, float interp,
    float resonance, float passbandGain);

extern inline void sig_filter_Ladder_calcStages(
    struct sig_filter_Ladder* self, float u,
    struct sig_filter_Ladder_Taps* taps);

extern inline void sig_filter_Ladder_generate(struct sig_filter_Ladder* self,
    float value, float resonance, float passbandGain,
    struct sig_filter_Ladder_Taps* taps);

// TODO: Unit tests.
float sig_waveform_sine(float phase) {
    return sinf(phase);
}

// TODO: Unit tests.
float sig_waveform_square(float phase) {
    return phase <= sig_PI ? 1.0f : -1.0f;
}

// TODO: Unit tests.
float sig_waveform_saw(float phase) {
    return (2.0f * (phase * (1.0f / sig_TWOPI))) - 1.0f;
}

// TODO: Unit tests.
float sig_waveform_reverseSaw(float phase) {
    return 1.0f - 2.0f * (phase * (1.0f / sig_TWOPI));
}

// TODO: Unit tests.
float sig_waveform_triangle(float phase) {
    float val = sig_waveform_saw(phase);
    if (val < 0.0) {
        val = -val;
    }

    return 2.0f * (val - 0.5f);
}

float sig_waveform_sinePhaseTriangle(float phase) {
    float value = phase * (2.0f / sig_PI);

    if (value < 1.0f) {
        return value;
    }

    if (value < 3.0f) {
        return 2.0f - value;
    }

    return value - 4.0f;
}

float sig_waveform_sinePhaseSaw(float phase) {
    float value = phase * (1.0f / sig_PI);

    return value < 1.0f ? value : value - 2.0f;
}

float sig_waveform_sineHarmonicGain(size_t harmonic) {
    return harmonic == 1 ? 1.0f : 0.0f;
}

float sig_waveform_triangleHarmonicGain(size_t harmonic) {
    float k = (float) harmonic;

    return harmonic % 2 == 1 ? 8.0f / (sig_PI * sig_PI * k * k) : 0.0f;
}

float sig_waveform_sawHarmonicGain(size_t harmonic) {
    return -2.0f / (sig_PI * (float) harmonic);
}

float sig_waveform_squareHarmonicGain(size_t harmonic) {
    return harmonic % 2 == 1 ? 4.0f / (sig_PI * (float) harmonic) : 0.0f;
}

float sig_waveform_reverseSawHarmonicGain(size_t harmonic) {
    return -sig_waveform_sawHarmonicGain(harmonic);
}

// Delaying a triangle by a quarter cycle turns its cosine harmonics into
// sines, negating harmonics 3, 7, 11, ...
float sig_waveform_sinePhaseTriangleHarmonicGain(size_t harmonic) {
    float gain = sig_waveform_triangleHarmonicGain(harmonic);

    return harmonic % 4 == 3 ? -gain : gain;
}

// Delaying a saw by half a cycle negates its odd harmonics.
float sig_waveform_sinePhaseSawHarmonicGain(size_t harmonic) {
    float gain = sig_waveform_sawHarmonicGain(harmonic);

    return harmonic % 2 == 1 ? -gain : gain;
}

// Triangle harmonics are cosines, which start a quarter cycle into a sine.
const struct sig_WaveformShape
    sig_STANDARD_WAVEFORMS[sig_StandardWaveform_COUNT] = {
    {
        .generate = sig_waveform_sine,
        .harmonic = sig_waveform_sineHarmonicGain,
        .harmonicPhase = 0.0f
    },
    {
        .generate = sig_waveform_triangle,
        .harmonic = sig_waveform_triangleHarmonicGain,
        .harmonicPhase = 0.25f
    },
    {
        .generate = sig_waveform_saw,
        .harmonic = sig_waveform_sawHarmonicGain,
        .harmonicPhase = 0.0f
    },
    {
        .generate = sig_waveform_square,
        .harmonic = sig_waveform_squareHarmonicGain,
        .harmonicPhase = 0.0f
    },
    {
        .generate = sig_waveform_reverseSaw,
        .harmonic = sig_waveform_reverseSawHarmonicGain,
        .harmonicPhase = 0.0f
    }
};

const struct sig_WaveformShape
    sig_SINE_PHASE_WAVEFORMS[sig_StandardWaveform_COUNT] = {
    {
        .generate = sig_waveform_sine,
        .harmonic = sig_waveform_sineHarmonicGain,
        .harmonicPhase = 0.0f
    },
    {
        .generate = sig_waveform_sinePhaseTriangle,
        .harmonic = sig_waveform_sinePhaseTriangleHarmonicGain,
        .harmonicPhase = 0.0f
    },
    {
        .generate = sig_waveform_sinePhaseSaw,
        .harmonic = sig_waveform_sinePhaseSawHarmonicGain,
        .harmonicPhase = 0.0f
    },
    {
        .generate = sig_waveform_square,
        .harmonic = sig_waveform_squareHarmonicGain,
        .harmonicPhase = 0.0f
    },
    {
        .generate = sig_waveform_reverseSaw,
        .harmonic = sig_waveform_reverseSawHarmonicGain,
        .harmonicPhase = 0.0f
    }
};

float sig_waveform_harmonicGain(enum sig_StandardWaveform waveform,
    size_t harmonic) {
    return sig_STANDARD_WAVEFORMS[waveform].harmonic(harmonic);
}

float sig_waveform_harmonicPhase(enum sig_StandardWaveform waveform) {
    return sig_STANDARD_WAVEFORMS[waveform].harmonicPhase;
}

float sig_waveform_harmonicTaper(enum sig_HarmonicTaper taper,
    size_t harmonic, size_t numHarmonics) {
    if (taper != sig_HarmonicTaper_LANCZOS) {
        return 1.0f;
    }

    // The sigma factors are divided by the fundamental's, so that the
    // fundamental's gain doesn't depend on the number of harmonics.
    float step = sig_PI / (float) (numHarmonics + 1);
    float x = step * (float) harmonic;

    return (sinf(x) / x) / (sinf(step) / step);
}


void sig_osc_Oscillator_init(struct sig_osc_Oscillator* self) {
    self->phaseAccumulator = 0.0f;
}

extern inline float sig_osc_Oscillator_eoc(float phase);

extern inline float sig_osc_Oscillator_wrapPhase(float phase);

extern inline void sig_osc_Oscillator_accumulatePhase(
    float* phaseAccumulator, float frequency, float sampleRate);

void sig_osc_Wavetable_init(struct sig_osc_Wavetable* self,
    struct sig_Buffer* wavetable) {
    self->phaseAccumulator = 0.0f;
    self->wavetable = wavetable;
}

extern inline float sig_osc_Wavetable_generate(
    struct sig_osc_Wavetable* self, float frequency,
    float phaseOffset, float sampleRate, float* eocOut);


void sig_osc_WavetableBank_init(struct sig_osc_WavetableBank* self,
    struct sig_WavetableBank* wavetables) {
    self->phaseAccumulator = 0.0f;
    self->wavetables = wavetables;
}

float sig_osc_WavetableBank_tableIndex(size_t table, size_t numTables) {
    return (float) table / (float) numTables;
}

extern inline float sig_osc_WavetableBank_generate(
    struct sig_osc_WavetableBank* self,
    float frequency, float phaseOffset, float tableIndex,
    float sampleRate, float* eocOut);

void sig_osc_AdditiveWavetableBank_init(
    struct sig_osc_AdditiveWavetableBank* self,
    struct sig_AdditiveWavetableBank* wavetables) {
    self->phaseAccumulator = 0.0f;
    self->wavetables = wavetables;
    self->previousFrequency = 0.0f;
    self->level = 0.0f;
}

extern inline float sig_osc_AdditiveWavetableBank_readAtPhase(
    struct sig_osc_AdditiveWavetableBank* self, float frequency, float phase,
    float tableIndex);

extern inline float sig_osc_AdditiveWavetableBank_generate(
    struct sig_osc_AdditiveWavetableBank* self, float frequency,
    float phaseOffset, float tableIndex, float sampleRate, float* eocOut);

void sig_osc_RatioPhasor_init(struct sig_osc_RatioPhasor* self) {
    self->phase = 0.0f;
    self->previousReference = 0.0f;
    self->cycle = 0;
    self->correction = 1.0f;
    self->maxIncrement = 0.5f;
}

void sig_osc_RatioPhasor_calculateCoefficient(
    struct sig_osc_RatioPhasor* self, float glide, float sampleRate) {
    self->correction = glide > 0.0f ?
        1.0f - expf(-1.0f / (glide * sampleRate)) : 1.0f;
}

void sig_osc_RatioPhasor_setMaxFrequency(struct sig_osc_RatioPhasor* self,
    float maxFrequency, float sampleRate) {
    self->maxIncrement = sig_clamp(maxFrequency / sampleRate, 0.0f, 0.5f);
}

extern inline float sig_osc_RatioPhasor_generate(
    struct sig_osc_RatioPhasor* self, float reference, float numerator,
    float denominator, float octaves);

void sig_osc_ClockPhasor_init(struct sig_osc_ClockPhasor* self) {
    self->phase = 0.0f;
    self->increment = 0.0f;
    self->correction = 0.0f;
    self->correctionSamples = 0;
    self->samplesSinceLastPulse = 0;
    self->hasPulsed = false;
    self->previousPulse = 0.0f;
}

extern inline float sig_osc_ClockPhasor_generate(
    struct sig_osc_ClockPhasor* self, float pulse, float threshold);

void sig_osc_PeakFrequency_init(struct sig_osc_PeakFrequency* self) {
    self->previousPhase = 0.0f;
    self->hasPreviousPhase = false;
    self->peak = 0.0f;
    self->release = 0.0f;
}

void sig_osc_PeakFrequency_calculateCoefficient(
    struct sig_osc_PeakFrequency* self, float release, float sampleRate) {
    self->release = release > 0.0f ?
        expf(-1.0f / (release * sampleRate)) : 0.0f;
}

extern inline float sig_osc_PeakFrequency_generate(
    struct sig_osc_PeakFrequency* self, float phase, float sampleRate);


void sig_osc_FastLFSine_init(struct sig_osc_FastLFSine* self,
    float sampleRate) {
    self->sampleRate = sampleRate;
    self->sinZ = 0.0f;
    self->cosZ = 1.0f;
    sig_osc_FastLFSine_setFrequency(self, 1.0f);
}

extern inline void sig_osc_FastLFSine_setFrequency(
    struct sig_osc_FastLFSine* self, float frequency);

extern inline void sig_osc_FastLFSine_setFrequencyFast(
    struct sig_osc_FastLFSine* self, float frequency);

extern inline void sig_osc_FastLFSine_generate(struct sig_osc_FastLFSine* self);


// TODO: Implement enough test coverage for sig_Allocator
// to support a switch from TLSF to another memory allocator
// implementation sometime in the future (gh-26).
void sig_TLSFAllocator_init(struct sig_Allocator* allocator) {
    tlsf_create_with_pool(allocator->heap->memory,
        allocator->heap->length);
}

void* sig_TLSFAllocator_malloc(struct sig_Allocator* allocator,
    size_t size) {
    // TODO: TLSF will return a null pointer if we're out of memory.
    // The Allocator API needs to be extended to always take a Status object
    // for any operation that can fail.
    return tlsf_malloc(allocator->heap->memory, size);
}

void sig_TLSFAllocator_free(struct sig_Allocator* allocator,
    void* obj) {
    tlsf_free(allocator->heap->memory, obj);
}

struct sig_AllocatorImpl sig_TLSFAllocatorImpl = {
    .init = sig_TLSFAllocator_init,
    .malloc = sig_TLSFAllocator_malloc,
    .free = sig_TLSFAllocator_free
};


struct sig_Status* sig_Status_new(struct sig_Allocator* allocator) {
    struct sig_Status* self = allocator->impl->malloc(allocator,
        sizeof(struct sig_Status));
    sig_Status_init(self);

    return self;
}

void sig_Status_init(struct sig_Status* status) {
    sig_Status_reset(status);
}

void sig_Status_reset(struct sig_Status* status) {
    status->result = SIG_RESULT_NONE;
}

extern inline void sig_Status_reportResult(
    struct sig_Status* status, enum sig_Result result);


// TODO: Unit tests.
struct sig_List* sig_List_new(struct sig_Allocator* allocator,
    size_t capacity) {
    struct sig_List* self = allocator->impl->malloc(allocator,
        sizeof(struct sig_List));
    void** items = allocator->impl->malloc(allocator,
        sizeof(void*) * capacity);
    sig_List_init(self, items, capacity);

    return self;
}

// TODO: Unit tests.
void sig_List_init(struct sig_List* self, void** items,
    size_t capacity) {
    self->items = items;
    self->length = 0;
    self->capacity = capacity;
}

// TODO: Unit tests.
void sig_List_insert(struct sig_List* self,
    size_t index, void* item, struct sig_Status* status) {
    // We don't support sparse lists
    // or any kind of index wrapping.
    if (index > self->length || index < 0) {
        sig_Status_reportResult(status, SIG_ERROR_INDEX_OUT_OF_BOUNDS);
        return;
    }

    if (index == self->length) {
        sig_List_append(self, item, status);
        return;
    }

    if (self->length >= self->capacity) {
        sig_Status_reportResult(status, SIG_ERROR_EXCEEDS_CAPACITY);
        return;
    }

    for (size_t i = self->length - 1; i >= 0; i--) {
        self->items[i + 1] = self->items[i];
    }

    self->length++;

    sig_Status_reportResult(status, SIG_RESULT_SUCCESS);
}

// TODO: Unit tests.
void sig_List_append(struct sig_List* self, void* item,
    struct sig_Status* status) {
    size_t index = self->length;

    if (index >= self->capacity) {
        sig_Status_reportResult(status, SIG_ERROR_EXCEEDS_CAPACITY);
        return;
    }

    self->items[index] = item;
    self->length++;

    sig_Status_reportResult(status, SIG_RESULT_SUCCESS);
    return;
}

// TODO: Unit tests.
void* sig_List_pop(struct sig_List* self,
    struct sig_Status* status) {
    if (self->length < 1) {
        sig_Status_reportResult(status, SIG_ERROR_INDEX_OUT_OF_BOUNDS);
        return NULL;
    }

    size_t index = self->length - 1;
    void* item = self->items[index];

    self->items[index] = NULL;
    self->length--;

    sig_Status_reportResult(status, SIG_RESULT_SUCCESS);
    return item;
}

// TODO: Unit tests.
void* sig_List_remove(struct sig_List* self, size_t index,
    struct sig_Status* status) {
    if (index >= self->length || index < 1) {
        sig_Status_reportResult(status, SIG_ERROR_INDEX_OUT_OF_BOUNDS);
        return NULL;
    }

    if (index == self->length - 1) {
        return sig_List_pop(self, status);
    }

    void* removedItem = self->items[index];

    for (size_t i = index + 1; i < self->length - 1; i++) {
        void* item = self->items[i];
        self->items[i - 1] = item;
    }

    self->items[self->length - 1] = NULL;
    self->length--;

    sig_Status_reportResult(status, SIG_RESULT_SUCCESS);
    return removedItem;
}

// TODO: Unit tests.
void sig_List_destroy(struct sig_Allocator* allocator,
    struct sig_List* self) {
    allocator->impl->free(allocator, self->items);
    allocator->impl->free(allocator, self);
}


struct sig_AudioSettings* sig_AudioSettings_new(
    struct sig_Allocator* allocator) {
    struct sig_AudioSettings* settings =
        (struct sig_AudioSettings*)allocator->impl->malloc(
            allocator, sizeof(struct sig_AudioSettings));

    settings->sampleRate = sig_DEFAULT_AUDIOSETTINGS.sampleRate;
    settings->numChannels = sig_DEFAULT_AUDIOSETTINGS.numChannels;
    settings->blockSize = sig_DEFAULT_AUDIOSETTINGS.blockSize;

    return settings;
}

void sig_AudioSettings_destroy(struct sig_Allocator* allocator,
    struct sig_AudioSettings* self) {
    allocator->impl->free(allocator, self);
}


struct sig_SignalContext* sig_SignalContext_new(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings) {
    struct sig_SignalContext* self = sig_MALLOC(allocator,
        struct sig_SignalContext);

    self->audioSettings = audioSettings;

    struct sig_Buffer* emptyBuffer = sig_Buffer_new(allocator, 0);
    self->emptyBuffer = emptyBuffer;

    struct sig_DelayLine* oneSampleDelayLine = sig_DelayLine_new(allocator, 1);
    self->oneSampleDelayLine = oneSampleDelayLine;

    struct sig_dsp_ConstantValue* silence = sig_dsp_ConstantValue_new(
        allocator, self, 0.0f);
    self->silence = silence;

    struct sig_dsp_ConstantValue* unity = sig_dsp_ConstantValue_new(
        allocator, self, 1.0f);
    self->unity = unity;

    return self;
}

void sig_SignalContext_destroy(struct sig_Allocator* allocator,
    struct sig_SignalContext* self) {
    sig_Buffer_destroy(allocator, self->emptyBuffer);
    sig_DelayLine_destroy(allocator, self->oneSampleDelayLine);
    sig_dsp_ConstantValue_destroy(allocator, self->silence);
    sig_dsp_ConstantValue_destroy(allocator, self->unity);
    allocator->impl->free(allocator, self);
}


// TODO: Unit tests.
size_t sig_secondsToSamples(struct sig_AudioSettings* audioSettings,
    float duration) {
    float numSamplesF = audioSettings->sampleRate * duration;
    long rounded = lroundf(numSamplesF);
    return (size_t) labs(rounded);
}


float_array_ptr sig_samples_new(struct sig_Allocator* allocator,
    size_t length) {
    return (float_array_ptr) allocator->impl->malloc(allocator,
        sizeof(float) * length);
}

// TODO: Does an AudioBlock type need to be introduced?
float_array_ptr sig_AudioBlock_new(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings) {
    return sig_samples_new(allocator, audioSettings->blockSize);
}

float_array_ptr sig_AudioBlock_newWithValue(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    float value) {
    float_array_ptr block = sig_AudioBlock_new(allocator,
        audioSettings);
    sig_fillWithValue(block, audioSettings->blockSize, value);

    return block;
}

float_array_ptr sig_AudioBlock_newSilent(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings) {
    return sig_AudioBlock_newWithValue(allocator, audioSettings, 0.0f);
}

void sig_AudioBlock_destroy(struct sig_Allocator* allocator,
    float_array_ptr self) {
    allocator->impl->free(allocator, self);
}

struct sig_Buffer* sig_Buffer_new(struct sig_Allocator* allocator,
    size_t length) {
    struct sig_Buffer* self = (struct sig_Buffer*)
        allocator->impl->malloc(allocator, sizeof(struct sig_Buffer));
    self->length = length;
    self->samples = sig_samples_new(allocator, length);

    return self;
}

void sig_Buffer_fill(struct sig_Buffer* self,
    sig_array_filler filler) {
    sig_fill(self->samples, self->length, filler);
}

void sig_Buffer_fillWithValue(struct sig_Buffer* self,
    float value) {
    sig_fillWithValue(self->samples, self->length, value);
}


void sig_Buffer_fillWithSilence(struct sig_Buffer* self) {
    sig_fillWithSilence(self->samples, self->length);
}

// TODO: Unit tests.
void sig_Buffer_fillWithWaveform(struct sig_Buffer* self,
    sig_waveform_generator generate, float sampleRate,
    float phase, float freq) {
    float phaseInc = freq * sig_TWOPI / sampleRate;
    for (size_t i = 0; i < self->length; i++) {
        FLOAT_ARRAY(self->samples)[i] = generate(phase);
        phase += phaseInc;
        if (phase >= sig_TWOPI) {
            phase -= sig_TWOPI;
        } else if (phase < 0.0f) {
            phase += sig_TWOPI;
        }
    }
}

float sig_Buffer_read(struct sig_Buffer* self, float idx) {
    return FLOAT_ARRAY(self->samples)[(size_t) idx % self->length];
}

float sig_Buffer_readLinear(struct sig_Buffer* self, float idx) {
    return sig_interpolate_linear(idx, self->samples, self->length);
}

extern inline float sig_Buffer_readLinearAtPhase(
    struct sig_Buffer* self,
    float phase);

float sig_Buffer_readCubic(struct sig_Buffer* self, float idx) {
    return sig_interpolate_cubic(idx, self->samples, self->length);
}

void sig_FFT_init(struct sig_FFT* self, size_t length,
    float_array_ptr cosines, float_array_ptr sines) {
    self->length = length;
    self->cosines = cosines;
    self->sines = sines;

    for (size_t m = 0; m < length / 2; m++) {
        double angle = 2.0 * 3.14159265358979323846 * (double) m /
            (double) length;
        FLOAT_ARRAY(cosines)[m] = (float) cos(angle);
        FLOAT_ARRAY(sines)[m] = (float) sin(angle);
    }
}

struct sig_FFT* sig_FFT_new(struct sig_Allocator* allocator, size_t length) {
    struct sig_FFT* self = sig_MALLOC(allocator, struct sig_FFT);
    sig_FFT_init(self, length, sig_samples_new(allocator, length / 2),
        sig_samples_new(allocator, length / 2));

    return self;
}

// Reorders the samples into bit-reversed index order, which the in-place
// butterflies require.
static void sig_FFT_bitReverse(size_t length, float* SIG_RESTRICT real,
    float* SIG_RESTRICT imaginary) {
    size_t j = 0;
    for (size_t i = 0; i < length - 1; i++) {
        if (i < j) {
            float swapReal = real[i];
            real[i] = real[j];
            real[j] = swapReal;
            float swapImaginary = imaginary[i];
            imaginary[i] = imaginary[j];
            imaginary[j] = swapImaginary;
        }

        size_t bit = length >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j |= bit;
    }
}

// Transforms in place. A direction of -1.0 is the forward transform,
// and 1.0 is the inverse.
static void sig_FFT_transform(struct sig_FFT* self, float_array_ptr realIn,
    float_array_ptr imaginaryIn, float direction) {
    const size_t length = self->length;
    const float* SIG_RESTRICT cosines = FLOAT_ARRAY(self->cosines);
    const float* SIG_RESTRICT sines = FLOAT_ARRAY(self->sines);
    float* SIG_RESTRICT real = FLOAT_ARRAY(realIn);
    float* SIG_RESTRICT imaginary = FLOAT_ARRAY(imaginaryIn);

    sig_FFT_bitReverse(length, real, imaginary);

    for (size_t size = 2; size <= length; size <<= 1) {
        const size_t half = size >> 1;
        const size_t twiddleStep = length / size;

        for (size_t j = 0; j < half; j++) {
            const float twiddleReal = cosines[j * twiddleStep];
            const float twiddleImaginary = direction *
                sines[j * twiddleStep];

            for (size_t start = j; start < length; start += size) {
                const size_t other = start + half;
                float productReal = twiddleReal * real[other] -
                    twiddleImaginary * imaginary[other];
                float productImaginary = twiddleReal * imaginary[other] +
                    twiddleImaginary * real[other];
                real[other] = real[start] - productReal;
                imaginary[other] = imaginary[start] - productImaginary;
                real[start] += productReal;
                imaginary[start] += productImaginary;
            }
        }
    }
}

void sig_FFT_forward(struct sig_FFT* self, float_array_ptr real,
    float_array_ptr imaginary) {
    sig_FFT_transform(self, real, imaginary, -1.0f);
}

void sig_FFT_inverse(struct sig_FFT* self, float_array_ptr real,
    float_array_ptr imaginary) {
    sig_FFT_transform(self, real, imaginary, 1.0f);
}

void sig_FFT_destroy(struct sig_Allocator* allocator, struct sig_FFT* self) {
    allocator->impl->free(allocator, self->cosines);
    allocator->impl->free(allocator, self->sines);
    allocator->impl->free(allocator, self);
}

void sig_Buffer_addHarmonic(struct sig_Buffer* self,
    struct sig_Buffer* sineTable, size_t harmonic, float gain, float phase) {
    const size_t length = self->length;
    float* SIG_RESTRICT samples = FLOAT_ARRAY(self->samples);
    const float* SIG_RESTRICT sine = FLOAT_ARRAY(sineTable->samples);
    size_t sineIdx = ((size_t) (phase * (float) length)) % length;

    for (size_t i = 0; i < length; i++) {
        samples[i] += gain * sine[sineIdx];
        sineIdx += harmonic;
        if (sineIdx >= length) {
            sineIdx -= length;
        }
    }
}

void sig_Buffer_fillWithHarmonics(struct sig_Buffer* self,
    struct sig_FFT* fft, float_array_ptr imaginary,
    const struct sig_WaveformShape* shape, size_t numHarmonics,
    enum sig_HarmonicTaper taper) {
    const size_t length = self->length;
    float* SIG_RESTRICT real = FLOAT_ARRAY(self->samples);
    float* SIG_RESTRICT imag = FLOAT_ARRAY(imaginary);

    // A harmonic with gain g that starts at phase p (in cycles) is the real
    // part of the bin g * (sin(2pi * p) - i * cos(2pi * p)), transformed
    // back into a signal.
    const double angle = 2.0 * 3.14159265358979323846 *
        (double) shape->harmonicPhase;
    const float binReal = (float) sin(angle);
    const float binImaginary = (float) -cos(angle);

    sig_fillWithSilence(self->samples, length);
    sig_fillWithSilence(imaginary, length);
    for (size_t harmonic = 1; harmonic <= numHarmonics; harmonic++) {
        float gain = shape->harmonic(harmonic) *
            sig_waveform_harmonicTaper(taper, harmonic, numHarmonics);
        real[harmonic] = gain * binReal;
        imag[harmonic] = gain * binImaginary;
    }

    sig_FFT_inverse(fft, self->samples, imaginary);
}

void sig_Buffer_destroy(struct sig_Allocator* allocator,
    struct sig_Buffer* self) {
    allocator->impl->free(allocator, self->samples);
    allocator->impl->free(allocator, self);
};


struct sig_Buffer* sig_BufferView_new(
    struct sig_Allocator* allocator,
    struct sig_Buffer* buffer, size_t startIdx, size_t length) {
    struct sig_Buffer* self = sig_MALLOC(allocator, struct sig_Buffer);

    // TODO: Need to signal an error rather than
    // just returning a null pointer and a length of zero.
    if (startIdx < 0 || length > (buffer->length - startIdx)) {
        self->samples = NULL;
        self->length = 0;
    } else {
        self->samples = FLOAT_ARRAY(buffer->samples) + startIdx;
        self->length = length;
    }

    return self;
}

void sig_BufferView_destroy(struct sig_Allocator* allocator,
    struct sig_Buffer* self) {
    // Don't destroy the samples array;
    // it is shared with other Buffers.
    allocator->impl->free(allocator, self);
}


struct sig_WavetableBank* sig_WavetableBank_new(
    struct sig_Allocator* allocator,
    size_t numTables, size_t tableLength) {
    struct sig_WavetableBank* self = sig_MALLOC(allocator,
        struct sig_WavetableBank);
    self->length = numTables;

    self->waves = (struct sig_Buffer**) allocator->impl->malloc(
        allocator, sizeof(struct sig_Buffer*) * numTables);

    for (size_t i = 0; i < numTables; i++) {
        self->waves[i] = sig_Buffer_new(allocator, tableLength);
    }

    return self;
}

extern inline float sig_WavetableBank_readLinearAtPhase(
    struct sig_WavetableBank* self,
    float tableIdx, float phase);

void sig_WavetableBank_destroy(struct sig_Allocator* allocator,
    struct sig_WavetableBank* self) {
    for (size_t i = 0; i < self->length; i++) {
        sig_Buffer_destroy(allocator, self->waves[i]);
        self->waves[i] = NULL;
    }

    allocator->impl->free(allocator, self->waves);
    self->waves = NULL;
    allocator->impl->free(allocator, self);
    self = NULL;
}

struct sig_AdditiveWavetableBank* sig_AdditiveWavetableBank_new(
    struct sig_Allocator* allocator, const struct sig_WaveformShape* shapes,
    size_t numShapes, size_t numLevels, size_t tableLength,
    float lowestFrequency, float maxHarmonicFrequency,
    enum sig_HarmonicTaper taper) {
    struct sig_AdditiveWavetableBank* self = sig_MALLOC(allocator,
        struct sig_AdditiveWavetableBank);
    self->shapes = (struct sig_WaveformShape*) allocator->impl->malloc(
        allocator, sizeof(struct sig_WaveformShape) * numShapes);
    for (size_t i = 0; i < numShapes; i++) {
        self->shapes[i] = shapes[i];
    }
    self->numShapes = numShapes;
    self->numLevels = numLevels;
    self->tableLength = tableLength;
    self->lowestFrequency = lowestFrequency;
    self->maxHarmonicFrequency = maxHarmonicFrequency;
    self->taper = taper;
    self->levels = (struct sig_WavetableBank**) allocator->impl->malloc(
        allocator, sizeof(struct sig_WavetableBank*) * numLevels);

    for (size_t level = 0; level < numLevels; level++) {
        self->levels[level] = sig_WavetableBank_new(allocator, numShapes,
            tableLength);
    }

    struct sig_FFT* fft = sig_FFT_new(allocator, tableLength);
    float_array_ptr imaginary = sig_samples_new(allocator, tableLength);
    sig_AdditiveWavetableBank_fill(self, fft, imaginary);
    allocator->impl->free(allocator, imaginary);
    sig_FFT_destroy(allocator, fft);

    return self;
}

size_t sig_AdditiveWavetableBank_numHarmonics(
    struct sig_AdditiveWavetableBank* self, size_t level) {
    float topOfOctave = self->lowestFrequency *
        powf(2.0f, (float) (level + 1));
    size_t numHarmonics = (size_t) (self->maxHarmonicFrequency / topOfOctave);
    size_t maxHarmonics = self->tableLength / 2 - 1;

    if (numHarmonics < 1) {
        return 1;
    }

    return numHarmonics > maxHarmonics ? maxHarmonics : numHarmonics;
}

void sig_AdditiveWavetableBank_fill(
    struct sig_AdditiveWavetableBank* self, struct sig_FFT* fft,
    float_array_ptr imaginary) {
    for (size_t level = 0; level < self->numLevels; level++) {
        size_t numHarmonics = sig_AdditiveWavetableBank_numHarmonics(self,
            level);
        for (size_t shape = 0; shape < self->numShapes; shape++) {
            sig_Buffer_fillWithHarmonics(self->levels[level]->waves[shape],
                fft, imaginary, &self->shapes[shape], numHarmonics,
                self->taper);
        }
    }
}

extern inline float sig_AdditiveWavetableBank_levelForFrequency(
    struct sig_AdditiveWavetableBank* self, float frequency);

extern inline float sig_AdditiveWavetableBank_readLinearAtPhase(
    struct sig_AdditiveWavetableBank* self, float level, float tableIdx,
    float phase);

void sig_AdditiveWavetableBank_destroy(struct sig_Allocator* allocator,
    struct sig_AdditiveWavetableBank* self) {
    for (size_t level = 0; level < self->numLevels; level++) {
        sig_WavetableBank_destroy(allocator, self->levels[level]);
        self->levels[level] = NULL;
    }

    allocator->impl->free(allocator, self->levels);
    self->levels = NULL;
    allocator->impl->free(allocator, self->shapes);
    self->shapes = NULL;
    allocator->impl->free(allocator, self);
}

struct sig_DelayLine* sig_DelayLine_new(struct sig_Allocator* allocator,
    size_t maxDelayLength) {
    struct sig_DelayLine* self = sig_MALLOC(allocator, struct sig_DelayLine);
    self->buffer = sig_Buffer_new(allocator, maxDelayLength);
    sig_DelayLine_init(self);

    return self;
}

struct sig_DelayLine* sig_DelayLine_newSeconds(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings, float maxDelaySecs) {
    size_t maxDelayLength = (size_t) roundf(
        maxDelaySecs * audioSettings->sampleRate);

    return sig_DelayLine_new(allocator, maxDelayLength);
}

struct sig_DelayLine* sig_DelayLine_newWithTransferredBuffer(
    struct sig_Allocator* allocator, struct sig_Buffer* buffer) {
    struct sig_DelayLine* self = sig_MALLOC(allocator, struct sig_DelayLine);
    self->buffer = buffer;
    sig_DelayLine_init(self);

    return self;
}

void sig_DelayLine_init(struct sig_DelayLine* self) {
    self->writeIdx = 0;
    sig_Buffer_fillWithSilence(self->buffer); // Zero the delay line.
}

extern inline float sig_DelayLine_readAt(
    struct sig_DelayLine* self, size_t readPos);

extern inline float sig_DelayLine_linearReadAt(
    struct sig_DelayLine* self, float readPos);

extern inline float sig_DelayLine_cubicReadAt(
    struct sig_DelayLine* self, float readPos);

extern inline float sig_DelayLine_allpassReadAt(
    struct sig_DelayLine* self, float readPos,
    float previousSample);


extern inline float sig_DelayLine_readAtTime(
    struct sig_DelayLine* self, float source,
    float tapTime, float sampleRate);

extern inline float sig_DelayLine_linearReadAtTime(
    struct sig_DelayLine* self, float source, float tapTime,
    float sampleRate);

extern inline float sig_DelayLine_cubicReadAtTime(
    struct sig_DelayLine* self, float source, float tapTime,
    float sampleRate);

extern inline float sig_DelayLine_allpassReadAtTime(
    struct sig_DelayLine* self, float source, float tapTime,
    float sampleRate, float previousSample);


extern inline float sig_DelayLine_readAtTimes(
    struct sig_DelayLine* self, float source,
    float_array_ptr SIG_RESTRICT tapTimes,
    float_array_ptr SIG_RESTRICT tapGains,
    size_t numTaps, float sampleRate, float timeScale);

extern inline float sig_DelayLine_linearReadAtTimes(
    struct sig_DelayLine* self, float source,
    float_array_ptr SIG_RESTRICT tapTimes,
    float_array_ptr SIG_RESTRICT tapGains,
    size_t numTaps, float sampleRate, float timeScale);

extern inline float sig_DelayLine_cubicReadAtTimes(
    struct sig_DelayLine* self, float source,
    float_array_ptr SIG_RESTRICT tapTimes,
    float_array_ptr SIG_RESTRICT tapGains,
    size_t numTaps, float sampleRate, float timeScale);

extern inline void sig_DelayLine_write(struct sig_DelayLine* self,
    float sample);

extern inline float sig_DelayLine_calcFeedbackGain(float delayTime,
    float decayTime);

extern inline float sig_DelayLine_feedback(float sample,
    float read, float g);


extern inline float sig_DelayLine_comb(struct sig_DelayLine* self,
    float sample, size_t readPos, float g);

extern inline float sig_DelayLine_linearComb(
    struct sig_DelayLine* self, float sample, float readPos,
    float g);

extern inline float sig_DelayLine_cubicComb(
    struct sig_DelayLine* self, float sample, float readPos,
    float g);

extern inline float sig_DelayLine_allpass(
    struct sig_DelayLine* self, float sample, size_t readPos,
    float g);

extern inline float sig_DelayLine_linearAllpass(
    struct sig_DelayLine* self, float sample, float readPos,
    float g);

extern inline float sig_DelayLine_cubicAllpass(
    struct sig_DelayLine* self, float sample, float readPos,
    float g);

void sig_DelayLine_destroy(struct sig_Allocator* allocator,
    struct sig_DelayLine* self) {
    sig_Buffer_destroy(allocator, self->buffer);
    allocator->impl->free(allocator, self);
}

extern inline float sig_linearXFade(float left, float right,
    float mix);

extern inline float sig_sineWavefolder(float x, float gain,
    float factor);

extern inline float sig_softClip(float x, float threshold);

// Each band's peak follower decays by this factor per hop, which is
// about half a second at an envelope rate of 86 values per second.
#define SIG_TEMPO_WHITENING_PEAK_DECAY 0.98f
// The detector divides each band by at least this fraction (-30 dB) of
// the loudest band's peak, so that it does not amplify quiet bands
// beyond the loudest band's scale.
#define SIG_TEMPO_WHITENING_RELATIVE_FLOOR 0.0316f
// The filterbank's lowest centre frequency, in Hz.
#define SIG_TEMPO_FILTERBANK_MIN_FREQUENCY 30.0f
// The filterbank's highest centre frequency, in Hz, unless the sample
// rate limits it.
#define SIG_TEMPO_FILTERBANK_MAX_FREQUENCY 6000.0f
// The filterbank's highest centre frequency, as a fraction of the
// sample rate.
#define SIG_TEMPO_FILTERBANK_MAX_RATIO 0.45f
#define SIG_TEMPO_FILTERBANK_BANDS_PER_OCTAVE 24.0f
// The largest band weight, which keeps the noisy single-bin bands at
// the bottom of the range from dominating the envelope.
#define SIG_TEMPO_FILTERBANK_MAX_WEIGHT 2.0f
// The detector subtracts this value from the mean flux, so that the
// flux of noise below the threshold gives an envelope of zero.
#define SIG_TEMPO_ONSET_OFFSET 0.022f

// Fills a window with the Hann window
// w[n] = 0.5 * (1 - cos(2pi * n / length)).
static void sig_tempo_OnsetDetector_fillWindow(float_array_ptr window,
    size_t length) {
    float* SIG_RESTRICT samples = FLOAT_ARRAY(window);
    for (size_t n = 0; n < length; n++) {
        samples[n] = 0.5f * (1.0f - cosf(sig_TWOPI * (float) n /
            (float) length));
    }
}

// Finds the distinct FFT bins nearest to the filterbank's centre
// frequencies, writes them into edges unless edges is NULL, and
// returns their number.
static size_t sig_tempo_OnsetDetector_findBandEdges(float sampleRate,
    size_t windowLength, float* edges) {
    float binWidth = sampleRate / (float) windowLength;
    float maxFrequency = SIG_TEMPO_FILTERBANK_MAX_RATIO * sampleRate;
    if (maxFrequency > SIG_TEMPO_FILTERBANK_MAX_FREQUENCY) {
        maxFrequency = SIG_TEMPO_FILTERBANK_MAX_FREQUENCY;
    }

    size_t numEdges = 0;
    float previousBin = -1.0f;
    for (size_t k = 0; ; k++) {
        float frequency = SIG_TEMPO_FILTERBANK_MIN_FREQUENCY *
            powf(2.0f, (float) k / SIG_TEMPO_FILTERBANK_BANDS_PER_OCTAVE);
        if (frequency > maxFrequency) {
            break;
        }

        float bin = roundf(frequency / binWidth);
        if (bin != previousBin) {
            if (edges != NULL) {
                edges[numEdges] = bin;
            }
            numEdges++;
            previousBin = bin;
        }
    }

    return numEdges;
}

// Fills weights with each band's weight, half the distance between its
// neighbours' centres in steps of 1 / 24 octave, capped at the maximum
// weight, and returns the weights' sum.
static float sig_tempo_OnsetDetector_fillBandWeights(
    const float* SIG_RESTRICT edges, float* SIG_RESTRICT weights,
    size_t numBands) {
    float sum = 0.0f;
    for (size_t j = 0; j < numBands; j++) {
        float weight = 0.5f * SIG_TEMPO_FILTERBANK_BANDS_PER_OCTAVE *
            log2f(edges[j + 2] / edges[j]);
        if (weight > SIG_TEMPO_FILTERBANK_MAX_WEIGHT) {
            weight = SIG_TEMPO_FILTERBANK_MAX_WEIGHT;
        }
        weights[j] = weight;
        sum += weight;
    }

    return sum;
}

size_t sig_tempo_OnsetDetector_calcNumBands(float sampleRate,
    size_t windowLength) {
    size_t numEdges = sig_tempo_OnsetDetector_findBandEdges(sampleRate,
        windowLength, NULL);

    return numEdges > 2 ? numEdges - 2 : 0;
}

void sig_tempo_OnsetDetector_init(struct sig_tempo_OnsetDetector* self,
    float sampleRate, size_t windowLength, size_t hopLength,
    size_t diffLagHops, float threshold, struct sig_FFT* fft,
    float_array_ptr window, float_array_ptr frame, float_array_ptr real,
    float_array_ptr imaginary, float_array_ptr bandEdges,
    float_array_ptr bandWeights, float_array_ptr bandPeaks,
    float_array_ptr bandRows) {
    self->sampleRate = sampleRate;
    self->windowLength = windowLength;
    self->hopLength = hopLength;
    self->diffLagHops = diffLagHops;
    self->threshold = threshold;
    self->thresholdMagnitude = powf(10.0f, threshold / 20.0f);
    self->numBands = sig_tempo_OnsetDetector_calcNumBands(sampleRate,
        windowLength);
    self->fft = fft;
    self->window = window;
    self->frame = frame;
    self->real = real;
    self->imaginary = imaginary;
    self->bandEdges = bandEdges;
    self->bandWeights = bandWeights;
    self->bandPeaks = bandPeaks;
    self->bandRows = bandRows;
    self->frameWriteIdx = 0;
    self->hopIdx = 0;
    self->currentRow = 0;
    self->hasNewValue = false;
    self->envelope = 0.0f;

    sig_tempo_OnsetDetector_fillWindow(window, windowLength);
    sig_tempo_OnsetDetector_findBandEdges(sampleRate, windowLength,
        FLOAT_ARRAY(bandEdges));
    self->totalWeight = sig_tempo_OnsetDetector_fillBandWeights(
        FLOAT_ARRAY(bandEdges), FLOAT_ARRAY(bandWeights), self->numBands);
    sig_fillWithSilence(frame, windowLength);
    sig_fillWithSilence(real, windowLength);
    sig_fillWithSilence(imaginary, windowLength);
    sig_fillWithSilence(bandPeaks, self->numBands);
    sig_fillWithSilence(bandRows, (diffLagHops + 1) * self->numBands);
}

struct sig_tempo_OnsetDetector* sig_tempo_OnsetDetector_new(
    struct sig_Allocator* allocator, float sampleRate, size_t windowLength,
    size_t hopLength, size_t diffLagHops, float threshold) {
    size_t numBands = sig_tempo_OnsetDetector_calcNumBands(sampleRate,
        windowLength);
    struct sig_FFT* fft = sig_FFT_new(allocator, windowLength);

    struct sig_tempo_OnsetDetector* self =
        sig_MALLOC(allocator, struct sig_tempo_OnsetDetector);
    sig_tempo_OnsetDetector_init(self, sampleRate, windowLength, hopLength,
        diffLagHops, threshold, fft,
        sig_samples_new(allocator, windowLength),
        sig_samples_new(allocator, windowLength),
        sig_samples_new(allocator, windowLength),
        sig_samples_new(allocator, windowLength),
        sig_samples_new(allocator, numBands + 2),
        sig_samples_new(allocator, numBands),
        sig_samples_new(allocator, numBands),
        sig_samples_new(allocator, (diffLagHops + 1) * numBands));

    return self;
}

// Unrolls the frame ring buffer into time order in real, applies the
// window, transforms the frame, and leaves the first half of real
// containing the magnitude spectrum, scaled so that a full-scale
// sine's peak bin has a magnitude of about 1.
static void sig_tempo_OnsetDetector_calcMagnitudes(
    struct sig_tempo_OnsetDetector* self) {
    const size_t length = self->windowLength;
    const float scale = 4.0f / (float) length;
    const float* SIG_RESTRICT window = FLOAT_ARRAY(self->window);
    const float* SIG_RESTRICT frame = FLOAT_ARRAY(self->frame);
    float* SIG_RESTRICT real = FLOAT_ARRAY(self->real);
    float* SIG_RESTRICT imaginary = FLOAT_ARRAY(self->imaginary);

    // The write position contains the oldest sample in the ring.
    size_t readIdx = self->frameWriteIdx;
    for (size_t n = 0; n < length; n++) {
        real[n] = frame[readIdx] * window[n];
        imaginary[n] = 0.0f;
        readIdx++;
        if (readIdx == length) {
            readIdx = 0;
        }
    }

    sig_FFT_forward(self->fft, self->real, self->imaginary);

    for (size_t k = 0; k < length / 2; k++) {
        real[k] = scale * sqrtf(real[k] * real[k] +
            imaginary[k] * imaginary[k]);
    }
}

// Returns the weighted mean of the magnitudes under a triangular
// filter that rises from bin start to bin centre and falls to bin
// stop.
static float sig_tempo_OnsetDetector_calcBand(
    const float* SIG_RESTRICT magnitudes, size_t start, size_t centre,
    size_t stop) {
    float rise = (float) (centre - start + 1);
    float fall = (float) (stop - centre + 1);
    float sum = 0.0f;
    float weightSum = 0.0f;
    for (size_t k = start; k <= stop; k++) {
        float weight = k < centre ?
            (float) (k - start + 1) / rise : (float) (stop - k + 1) / fall;
        sum += weight * magnitudes[k];
        weightSum += weight;
    }

    return sum / weightSum;
}

// Writes the newest frame's whitened and compressed bands into its
// row of bandRows.
static void sig_tempo_OnsetDetector_writeBands(
    struct sig_tempo_OnsetDetector* self) {
    const size_t numBands = self->numBands;
    const size_t numRows = self->diffLagHops + 1;
    const float* SIG_RESTRICT magnitudes = FLOAT_ARRAY(self->real);
    const float* SIG_RESTRICT edges = FLOAT_ARRAY(self->bandEdges);
    float* SIG_RESTRICT peaks = FLOAT_ARRAY(self->bandPeaks);

    self->currentRow++;
    if (self->currentRow == numRows) {
        self->currentRow = 0;
    }
    float* SIG_RESTRICT row = FLOAT_ARRAY(self->bandRows) +
        self->currentRow * numBands;

    float maxPeak = 0.0f;
    for (size_t j = 0; j < numBands; j++) {
        float band = sig_tempo_OnsetDetector_calcBand(magnitudes,
            (size_t) edges[j], (size_t) edges[j + 1], (size_t) edges[j + 2]);
        row[j] = band;

        float decayed = SIG_TEMPO_WHITENING_PEAK_DECAY * peaks[j];
        float peak = band > decayed ? band : decayed;
        peaks[j] = peak;
        if (peak > maxPeak) {
            maxPeak = peak;
        }
    }

    float minimumDivisor = SIG_TEMPO_WHITENING_RELATIVE_FLOOR * maxPeak;
    if (minimumDivisor < self->thresholdMagnitude) {
        minimumDivisor = self->thresholdMagnitude;
    }

    for (size_t j = 0; j < numBands; j++) {
        float divisor = peaks[j] > minimumDivisor ?
            peaks[j] : minimumDivisor;
        row[j] = logf(1.0f + row[j] / divisor);
    }
}

// Takes the weighted mean of each band's increase over the largest of
// its three neighbouring bands in the frame diffLagHops hops earlier,
// and subtracts the offset.
static float sig_tempo_OnsetDetector_calcFlux(
    struct sig_tempo_OnsetDetector* self) {
    const size_t numBands = self->numBands;
    const size_t numRows = self->diffLagHops + 1;
    const float* SIG_RESTRICT rows = FLOAT_ARRAY(self->bandRows);
    const float* SIG_RESTRICT weights = FLOAT_ARRAY(self->bandWeights);

    const float* newest = rows + self->currentRow * numBands;
    // With numRows rows, the row diffLagHops back is the row after the
    // newest one.
    size_t referenceRow = self->currentRow + 1;
    if (referenceRow == numRows) {
        referenceRow = 0;
    }
    const float* reference = rows + referenceRow * numBands;

    float flux = 0.0f;
    for (size_t j = 0; j < numBands; j++) {
        float filtered = reference[j];
        if (j > 0 && reference[j - 1] > filtered) {
            filtered = reference[j - 1];
        }
        if (j + 1 < numBands && reference[j + 1] > filtered) {
            filtered = reference[j + 1];
        }

        float diff = newest[j] - filtered;
        if (diff > 0.0f) {
            flux += weights[j] * diff;
        }
    }

    float envelope = flux / self->totalWeight - SIG_TEMPO_ONSET_OFFSET;

    return envelope > 0.0f ? envelope : 0.0f;
}

float sig_tempo_OnsetDetector_generate(struct sig_tempo_OnsetDetector* self,
    float sample) {
    float* SIG_RESTRICT frame = FLOAT_ARRAY(self->frame);
    frame[self->frameWriteIdx] = sample;
    self->frameWriteIdx++;
    if (self->frameWriteIdx == self->windowLength) {
        self->frameWriteIdx = 0;
    }

    self->hopIdx++;
    self->hasNewValue = self->hopIdx == self->hopLength;
    if (self->hasNewValue) {
        self->hopIdx = 0;
        sig_tempo_OnsetDetector_calcMagnitudes(self);
        sig_tempo_OnsetDetector_writeBands(self);
        self->envelope = sig_tempo_OnsetDetector_calcFlux(self);
    }

    return self->envelope;
}

extern inline bool sig_tempo_OnsetDetector_hasNewValue(
    struct sig_tempo_OnsetDetector* self);

extern inline void sig_tempo_OnsetDetector_setThreshold(
    struct sig_tempo_OnsetDetector* self, float threshold);

void sig_tempo_OnsetDetector_destroy(struct sig_Allocator* allocator,
    struct sig_tempo_OnsetDetector* self) {
    sig_FFT_destroy(allocator, self->fft);
    allocator->impl->free(allocator, self->window);
    allocator->impl->free(allocator, self->frame);
    allocator->impl->free(allocator, self->real);
    allocator->impl->free(allocator, self->imaginary);
    allocator->impl->free(allocator, self->bandEdges);
    allocator->impl->free(allocator, self->bandWeights);
    allocator->impl->free(allocator, self->bandPeaks);
    allocator->impl->free(allocator, self->bandRows);
    allocator->impl->free(allocator, self);
}

// The difference between consecutive candidate tempos, in beats per
// minute.
#define SIG_TEMPO_CANDIDATE_STEP 1.0f
// The number of envelope values between estimations.
#define SIG_TEMPO_ESTIMATE_INTERVAL 16
// The centre of the tempo prior, in beats per minute.
#define SIG_TEMPO_PRIOR_CENTER 120.0f
// The width of the tempo prior, in octaves.
#define SIG_TEMPO_PRIOR_WIDTH 1.0f
// The number of beats at the slowest tempo that a window contains.
#define SIG_TEMPO_WINDOW_BEATS 4.0f
// The number of beats at the fastest tempo required before the first
// estimation.
#define SIG_TEMPO_MINIMUM_BEATS 3.0f
// The factor by which a distant candidate's score must exceed the
// current estimate's score to replace it.
#define SIG_TEMPO_SWITCH_MARGIN 1.3f
// The largest difference between the best candidate and the current
// estimate, in candidate steps, at which the estimator adopts the
// best candidate without the margin.
#define SIG_TEMPO_ADOPT_DISTANCE 2.0f
// The factor by which a candidate at a multiple of an established
// estimate's tempo must exceed the estimate's score to replace it.
#define SIG_TEMPO_MULTIPLE_SWITCH_MARGIN 2.0f
// The largest relative difference between two estimates, or between a
// tempo ratio and a whole number, that the estimator treats as equal.
#define SIG_TEMPO_STABILITY_TOLERANCE 0.08f
// The time, in seconds, for which the estimate must stay stable before
// the larger margin protects it.
#define SIG_TEMPO_ESTABLISHED_DURATION 4.0f
// The weight of the autocorrelation at twice a candidate's beat
// interval in the candidate's score.
#define SIG_TEMPO_DOUBLE_INTERVAL_WEIGHT 0.5f
// The estimator and the beat tracker treat scores and envelope peaks
// below this value as silence.
#define SIG_TEMPO_SILENCE 0.00001f

size_t sig_tempo_calcWindowLength(float envelopeRate, float lowestTempo) {
    return (size_t) ceilf(envelopeRate * SIG_TEMPO_WINDOW_BEATS * 60.0f /
        lowestTempo);
}

// Reads the estimator's window at a fractional position, where zero
// is the oldest value in the window.
static float sig_tempo_TempoEstimator_readWindow(
    const float* SIG_RESTRICT envelope, size_t windowLength, size_t start,
    size_t available, float position) {
    size_t index = (size_t) position;
    float fraction = position - (float) index;
    size_t next = index + 1 < available ? index + 1 : index;
    float first = envelope[(start + index) % windowLength];
    float second = envelope[(start + next) % windowLength];
    return first + fraction * (second - first);
}

// Returns the window's autocorrelation at a lag, divided by the number
// of overlapping values, or zero when the lag leaves no overlap.
static float sig_tempo_TempoEstimator_autocorrelate(
    const float* SIG_RESTRICT envelope, size_t windowLength, size_t start,
    size_t available, float lag) {
    float overlap = (float) available - lag;
    if (overlap < 1.0f) {
        return 0.0f;
    }

    float sum = 0.0f;
    for (size_t n = 0; (float) n < overlap; n++) {
        float first = sig_tempo_TempoEstimator_readWindow(envelope,
            windowLength, start, available, (float) n);
        float second = sig_tempo_TempoEstimator_readWindow(envelope,
            windowLength, start, available, (float) n + lag);
        sum += first * second;
    }

    return sum / overlap;
}

// Scores every candidate in the current range, and returns the index
// of the best one.
static size_t sig_tempo_TempoEstimator_scoreCandidates(
    struct sig_tempo_TempoEstimator* self, float* bestScore) {
    const size_t windowLength = self->windowLength;
    const size_t available = self->valueCount < windowLength ?
        self->valueCount : windowLength;
    const float* SIG_RESTRICT envelope = FLOAT_ARRAY(self->envelope);
    float* SIG_RESTRICT scores = FLOAT_ARRAY(self->scores);

    // The window starts at the oldest value. While it is filling, the
    // oldest value is the first one written.
    const size_t start = available == windowLength ? self->writeIdx : 0;
    const float lagLimit = (float) available / 2.0f;
    const size_t first = (size_t) ceilf((self->minTempo - self->lowestTempo) /
        self->candidateStep);
    size_t last = (size_t) floorf((self->maxTempo - self->lowestTempo) /
        self->candidateStep);
    if (last >= self->numCandidates) {
        last = self->numCandidates - 1;
    }

    *bestScore = 0.0f;
    size_t bestIndex = first;
    for (size_t c = 0; c < self->numCandidates; c++) {
        scores[c] = 0.0f;
        if (c < first || c > last) {
            continue;
        }

        float tempo = self->lowestTempo + (float) c * self->candidateStep;
        float lag = self->envelopeRate * 60.0f / tempo;
        if (lag > lagLimit) {
            continue;
        }

        // The autocorrelation at twice the beat interval lets a
        // candidate score on onsets that fall only on alternate beats,
        // so that a narrowed range can select a faster metrical level.
        float score = sig_tempo_TempoEstimator_autocorrelate(envelope,
            windowLength, start, available, lag) +
            SIG_TEMPO_DOUBLE_INTERVAL_WEIGHT *
            sig_tempo_TempoEstimator_autocorrelate(envelope,
                windowLength, start, available, 2.0f * lag);

        // Weight the score with a log-normal tempo prior, so that
        // octave-related tempos resolve to a consistent level.
        float octaves = log2f(tempo / self->priorCenter);
        float prior = expf(-octaves * octaves /
            (2.0f * self->priorWidth * self->priorWidth));
        scores[c] = score * prior;

        if (scores[c] > *bestScore) {
            *bestScore = scores[c];
            bestIndex = c;
        }
    }

    return bestIndex;
}

// Returns the ratio of the larger of two tempos to the smaller.
static float sig_tempo_calcTempoRatio(float first, float second) {
    return first > second ? first / second : second / first;
}

// Returns true if the current estimate has been stable for long enough
// to be established, and a candidate lies near 2, 3, or 4 times or a
// half, third, or quarter of it.
static bool sig_tempo_TempoEstimator_isEstablishedMultiple(
    struct sig_tempo_TempoEstimator* self, float candidate) {
    float stableDuration = (float) (self->stableCount *
        self->estimateInterval) / self->envelopeRate;
    if (stableDuration < SIG_TEMPO_ESTABLISHED_DURATION) {
        return false;
    }

    float ratio = sig_tempo_calcTempoRatio(candidate, self->tempo);
    for (int multiple = 2; multiple <= 4; multiple++) {
        if (fabsf(ratio / (float) multiple - 1.0f) <
            SIG_TEMPO_STABILITY_TOLERANCE) {
            return true;
        }
    }

    return false;
}

// Finds the best candidate tempo in the estimator's window, following
// the current tempo with hysteresis.
static float sig_tempo_TempoEstimator_estimate(
    struct sig_tempo_TempoEstimator* self) {
    const float* SIG_RESTRICT scores = FLOAT_ARRAY(self->scores);
    float bestScore;
    size_t bestIndex = sig_tempo_TempoEstimator_scoreCandidates(self,
        &bestScore);

    if (bestScore <= SIG_TEMPO_SILENCE) {
        return self->tempo;
    }

    float bestTempo = self->lowestTempo + (float) bestIndex *
        self->candidateStep;

    // The estimator adopts the first estimate, and the best candidate
    // when the current estimate lies outside the range, directly.
    if (self->tempo < self->minTempo || self->tempo > self->maxTempo) {
        return bestTempo;
    }

    size_t currentIndex = (size_t) ((self->tempo - self->lowestTempo) /
        self->candidateStep + 0.5f);
    if (currentIndex >= self->numCandidates) {
        currentIndex = self->numCandidates - 1;
    }

    float margin = sig_tempo_TempoEstimator_isEstablishedMultiple(self,
        bestTempo) ? SIG_TEMPO_MULTIPLE_SWITCH_MARGIN :
        SIG_TEMPO_SWITCH_MARGIN;
    float estimate = self->tempo;
    if (fabsf(bestTempo - self->tempo) <=
        SIG_TEMPO_ADOPT_DISTANCE * self->candidateStep ||
        bestScore > margin * scores[currentIndex]) {
        estimate = bestTempo;
    }

    if (sig_tempo_calcTempoRatio(estimate, self->tempo) <
        1.0f + SIG_TEMPO_STABILITY_TOLERANCE) {
        self->stableCount++;
    } else {
        self->stableCount = 0;
    }

    return estimate;
}

void sig_tempo_TempoEstimator_init(struct sig_tempo_TempoEstimator* self,
    float envelopeRate, float lowestTempo, float highestTempo,
    float_array_ptr envelope, float_array_ptr scores) {
    self->envelopeRate = envelopeRate;
    self->lowestTempo = lowestTempo;
    self->highestTempo = highestTempo;
    self->minTempo = lowestTempo;
    self->maxTempo = highestTempo;
    self->priorCenter = SIG_TEMPO_PRIOR_CENTER;
    self->priorWidth = SIG_TEMPO_PRIOR_WIDTH;
    self->candidateStep = SIG_TEMPO_CANDIDATE_STEP;
    self->numCandidates = (size_t) ((highestTempo - lowestTempo) /
        SIG_TEMPO_CANDIDATE_STEP) + 1;
    self->windowLength = sig_tempo_calcWindowLength(envelopeRate,
        lowestTempo);
    self->estimateInterval = SIG_TEMPO_ESTIMATE_INTERVAL;
    self->minimumValues = (size_t) ceilf(envelopeRate *
        SIG_TEMPO_MINIMUM_BEATS * 60.0f / highestTempo);
    self->envelope = envelope;
    self->scores = scores;
    self->writeIdx = 0;
    self->valueCount = 0;
    self->tempo = 0.0f;
    self->stableCount = 0;

    sig_fillWithSilence(envelope, self->windowLength);
    sig_fillWithSilence(scores, self->numCandidates);
}

struct sig_tempo_TempoEstimator* sig_tempo_TempoEstimator_new(
    struct sig_Allocator* allocator, float envelopeRate, float lowestTempo,
    float highestTempo) {
    struct sig_tempo_TempoEstimator* self =
        sig_MALLOC(allocator, struct sig_tempo_TempoEstimator);
    size_t windowLength = sig_tempo_calcWindowLength(envelopeRate,
        lowestTempo);
    size_t numCandidates = (size_t) ((highestTempo - lowestTempo) /
        SIG_TEMPO_CANDIDATE_STEP) + 1;

    sig_tempo_TempoEstimator_init(self, envelopeRate, lowestTempo,
        highestTempo, sig_samples_new(allocator, windowLength),
        sig_samples_new(allocator, numCandidates));

    return self;
}

extern inline void sig_tempo_TempoEstimator_setTempoRange(
    struct sig_tempo_TempoEstimator* self, float minTempo, float maxTempo);

float sig_tempo_TempoEstimator_generate(struct sig_tempo_TempoEstimator* self,
    float value) {
    float* SIG_RESTRICT envelope = FLOAT_ARRAY(self->envelope);
    envelope[self->writeIdx] = value;
    self->writeIdx++;
    if (self->writeIdx == self->windowLength) {
        self->writeIdx = 0;
    }
    self->valueCount++;

    if (self->valueCount >= self->minimumValues &&
        self->valueCount % self->estimateInterval == 0) {
        self->tempo = sig_tempo_TempoEstimator_estimate(self);
    }

    return self->tempo;
}

void sig_tempo_TempoEstimator_destroy(struct sig_Allocator* allocator,
    struct sig_tempo_TempoEstimator* self) {
    allocator->impl->free(allocator, self->envelope);
    allocator->impl->free(allocator, self->scores);
    allocator->impl->free(allocator, self);
}

// The number of envelope values between runs of the dynamic program.
#define SIG_TEMPO_DP_INTERVAL 4
// The dynamic program's interval penalty weight (Ellis, 2007).
#define SIG_TEMPO_DP_TIGHTNESS 12.0f
// The nearest and farthest predecessors that the dynamic program
// considers, in beat intervals.
#define SIG_TEMPO_DP_MIN_INTERVAL_RATIO 0.5f
#define SIG_TEMPO_DP_MAX_INTERVAL_RATIO 2.0f
// The most beats that one backtrace can find. The dynamic program
// skips beat intervals shorter than windowLength / 64, and its beats
// lie at least half an interval apart, which bounds a backtrace at
// this number of beats.
#define SIG_TEMPO_DP_MAX_BEATS 128
// Beats commit when they lie this many beat intervals behind the
// newest envelope value. A beat also commits only when it lies at
// least this far after the previously committed beat.
#define SIG_TEMPO_COMMIT_HORIZON 0.5f
// The fraction of the way that the strength reference moves toward
// each committed beat's value.
#define SIG_TEMPO_STRENGTH_SMOOTHING 0.1f
// Beats weaker than this strength are virtual.
#define SIG_TEMPO_VIRTUAL_STRENGTH 0.25f
// The largest error, in beats, at which the follower accepts a beat.
#define SIG_TEMPO_ACCEPT_WINDOW 0.25
// The most beats over which the follower measures an interval.
#define SIG_TEMPO_MAX_INTERVAL_BEATS 4.0
// The shortest and longest measured intervals that correct the grid's
// beat interval, relative to it.
#define SIG_TEMPO_MIN_INTERVAL_RATIO 0.8
#define SIG_TEMPO_MAX_INTERVAL_RATIO 1.25
// The shortest and longest beat intervals that a relock can choose,
// relative to the grid's beat interval.
#define SIG_TEMPO_MIN_RELOCK_RATIO 0.5
#define SIG_TEMPO_MAX_RELOCK_RATIO 2.0
// The ratio between the estimator's beat interval and the grid's at
// which the follower reseeds its grid.
#define SIG_TEMPO_RESEED_RATIO 1.33
// The fraction of the way that the confidence moves toward each
// committed beat's target.
#define SIG_TEMPO_CONFIDENCE_RATE 0.2f
// The smallest confidence after a tap.
#define SIG_TEMPO_TAP_CONFIDENCE 0.5f
// The amount that the dynamic program adds to the normalised envelope
// at the follower's predicted beats while it holds a tapped phase, and
// the Gaussian width of that addition in beats.
#define SIG_TEMPO_HOLD_GAIN 0.3f
#define SIG_TEMPO_HOLD_WIDTH 0.06
#define SIG_TEMPO_DEFAULT_RESPONSIVENESS 0.6f
#define SIG_TEMPO_DEFAULT_SILENCE_TIMEOUT 4.0f

void sig_tempo_BeatTracker_init(struct sig_tempo_BeatTracker* self,
    float sampleRate, size_t hopLength, float latency, float lowestTempo,
    float highestTempo, float_array_ptr envelope, float_array_ptr scores,
    float_array_ptr predecessors, float_array_ptr penalties) {
    self->sampleRate = sampleRate;
    self->hopLength = hopLength;
    self->envelopeRate = sampleRate / (float) hopLength;
    self->latency = latency;
    self->lowestTempo = lowestTempo;
    self->highestTempo = highestTempo;
    self->minTempo = lowestTempo;
    self->maxTempo = highestTempo;
    self->responsiveness = SIG_TEMPO_DEFAULT_RESPONSIVENESS;
    self->silenceTimeout = SIG_TEMPO_DEFAULT_SILENCE_TIMEOUT;
    self->windowLength = sig_tempo_calcWindowLength(self->envelopeRate,
        lowestTempo);
    self->envelope = envelope;
    self->scores = scores;
    self->predecessors = predecessors;
    self->penalties = penalties;
    self->writeIdx = 0;
    self->valueCount = 0;
    self->lastCommittedIndex = 0;
    self->hasCommitted = false;
    self->strengthReference = 0.0f;
    self->state = sig_tempo_BeatTracker_State_WAITING;
    self->period = 0.0;
    self->anchor = 0.0;
    self->sampleClock = 0.0;
    self->position = 0.0;
    self->lastAcceptedTime = 0.0;
    self->hasLastAccepted = false;
    self->lastStrongBeatTime = 0.0;
    self->numOutliers = 0;
    for (size_t i = 0; i < sig_tempo_BeatTracker_HELD_RELOCK_OUTLIERS; i++) {
        self->outlierTimes[i] = 0.0;
    }
    self->confidence = 0.0f;
    self->isHoldingTaps = false;
    for (size_t i = 0; i <= sig_tempo_BeatTracker_TAP_INTERVALS; i++) {
        self->tapTimes[i] = 0.0;
    }
    self->numTaps = 0;

    sig_fillWithSilence(envelope, self->windowLength);
    sig_fillWithSilence(scores, self->windowLength);
    sig_fillWithSilence(predecessors, self->windowLength);
    sig_fillWithSilence(penalties, self->windowLength);
}

struct sig_tempo_BeatTracker* sig_tempo_BeatTracker_new(
    struct sig_Allocator* allocator, float sampleRate, size_t hopLength,
    float latency, float lowestTempo, float highestTempo) {
    struct sig_tempo_BeatTracker* self =
        sig_MALLOC(allocator, struct sig_tempo_BeatTracker);
    size_t windowLength = sig_tempo_calcWindowLength(
        sampleRate / (float) hopLength, lowestTempo);

    sig_tempo_BeatTracker_init(self, sampleRate, hopLength, latency,
        lowestTempo, highestTempo, sig_samples_new(allocator, windowLength),
        sig_samples_new(allocator, windowLength),
        sig_samples_new(allocator, windowLength),
        sig_samples_new(allocator, windowLength));

    return self;
}

// Returns the beat interval in samples of a tempo in beats per minute.
static double sig_tempo_BeatTracker_tempoToPeriod(
    struct sig_tempo_BeatTracker* self, float tempo) {
    return 60.0 * (double) self->sampleRate / (double) tempo;
}

// Limits the grid's beat interval to the tempo range.
static void sig_tempo_BeatTracker_limitPeriod(
    struct sig_tempo_BeatTracker* self) {
    double shortest = sig_tempo_BeatTracker_tempoToPeriod(self,
        self->maxTempo);
    double longest = sig_tempo_BeatTracker_tempoToPeriod(self,
        self->minTempo);
    if (self->period < shortest) {
        self->period = shortest;
    } else if (self->period > longest) {
        self->period = longest;
    }
}

static void sig_tempo_BeatTracker_moveConfidence(
    struct sig_tempo_BeatTracker* self, float target) {
    self->confidence += SIG_TEMPO_CONFIDENCE_RATE *
        (target - self->confidence);
}

// Anchors the grid at a beat, and starts measuring intervals from it.
static void sig_tempo_BeatTracker_anchorAt(
    struct sig_tempo_BeatTracker* self, double time) {
    self->anchor = time;
    self->lastAcceptedTime = time;
    self->hasLastAccepted = true;
    self->numOutliers = 0;
}

// Returns the strength of a committed beat's envelope value, and
// updates the strength reference.
static float sig_tempo_BeatTracker_measureStrength(
    struct sig_tempo_BeatTracker* self, float value) {
    if (value > self->strengthReference) {
        self->strengthReference = value;
    } else {
        self->strengthReference += SIG_TEMPO_STRENGTH_SMOOTHING *
            (value - self->strengthReference);
    }

    if (self->strengthReference <= 0.0f) {
        return 0.0f;
    }

    float strength = value / self->strengthReference;

    return strength < 1.0f ? strength : 1.0f;
}

// Corrects the grid with a beat that lies within the acceptance window.
static void sig_tempo_BeatTracker_acceptBeat(
    struct sig_tempo_BeatTracker* self, double time, double gridBeat,
    double error, float strength) {
    double phaseGain = 0.2 + 0.7 * (double) self->responsiveness;
    double periodGain = 0.1 + 0.7 * (double) self->responsiveness;
    double period = self->period;
    double corrected = gridBeat + phaseGain * (double) strength * error *
        period;

    if (self->hasLastAccepted) {
        double span = time - self->lastAcceptedTime;
        double numBeats = floor(span / period + 0.5);
        if (numBeats >= 1.0 && numBeats <= SIG_TEMPO_MAX_INTERVAL_BEATS) {
            double interval = span / numBeats;
            if (interval >= SIG_TEMPO_MIN_INTERVAL_RATIO * period &&
                interval <= SIG_TEMPO_MAX_INTERVAL_RATIO * period) {
                self->period += periodGain * (double) strength *
                    (interval - period);
                sig_tempo_BeatTracker_limitPeriod(self);
            }
        }
    }

    // Anchoring at the newest beat makes a change of beat interval
    // rotate the grid about that beat.
    self->anchor = corrected;
    self->lastAcceptedTime = time;
    self->hasLastAccepted = true;
    self->numOutliers = 0;

    double distance = fabs(error) / SIG_TEMPO_ACCEPT_WINDOW;
    sig_tempo_BeatTracker_moveConfidence(self,
        strength * (float) (1.0 - distance));
}

// Records a beat that lies outside the acceptance window, and
// re-anchors the grid after enough consecutive outliers.
static void sig_tempo_BeatTracker_rejectBeat(
    struct sig_tempo_BeatTracker* self, double time) {
    const size_t relockOutliers = self->isHoldingTaps ?
        sig_tempo_BeatTracker_HELD_RELOCK_OUTLIERS :
        sig_tempo_BeatTracker_RELOCK_OUTLIERS;
    self->outlierTimes[self->numOutliers] = time;
    self->numOutliers++;
    sig_tempo_BeatTracker_moveConfidence(self, 0.0f);

    if (self->numOutliers < relockOutliers) {
        return;
    }

    double interval = (time - self->outlierTimes[0]) /
        (double) (relockOutliers - 1);
    double numBeats = floor(interval / self->period + 0.5);
    if (numBeats >= 1.0) {
        interval /= numBeats;
    }
    if (interval > SIG_TEMPO_MIN_RELOCK_RATIO * self->period &&
        interval < SIG_TEMPO_MAX_RELOCK_RATIO * self->period) {
        self->period = interval;
        sig_tempo_BeatTracker_limitPeriod(self);
    }

    sig_tempo_BeatTracker_anchorAt(self, time);
}

// Applies a committed beat to the follower.
static void sig_tempo_BeatTracker_commitBeat(
    struct sig_tempo_BeatTracker* self, double time, float value) {
    float strength = sig_tempo_BeatTracker_measureStrength(self, value);
    if (strength < SIG_TEMPO_VIRTUAL_STRENGTH) {
        sig_tempo_BeatTracker_moveConfidence(self, 0.0f);
        return;
    }

    self->lastStrongBeatTime = time;

    if (self->state != sig_tempo_BeatTracker_State_TRACKING) {
        // The first strong beat while searching or after a silence
        // anchors the grid, because music often resumes with a new
        // phase.
        sig_tempo_BeatTracker_anchorAt(self, time);
        self->state = sig_tempo_BeatTracker_State_TRACKING;
        self->confidence = 0.0f;
        sig_tempo_BeatTracker_moveConfidence(self, strength);
        return;
    }

    double beats = floor((time - self->anchor) / self->period + 0.5);
    double gridBeat = self->anchor + beats * self->period;
    double error = (time - gridBeat) / self->period;
    if (fabs(error) <= SIG_TEMPO_ACCEPT_WINDOW) {
        sig_tempo_BeatTracker_acceptBeat(self, time, gridBeat, error,
            strength);
    } else {
        sig_tempo_BeatTracker_rejectBeat(self, time);
    }
}

// Reads the tracker's window at a position, where zero is the oldest
// value in the window.
static float sig_tempo_BeatTracker_readWindow(
    struct sig_tempo_BeatTracker* self, size_t start, size_t position) {
    return FLOAT_ARRAY(self->envelope)[(start + position) %
        self->windowLength];
}

// Returns the offset, in envelope values, of the peak of a parabola
// through a window position and its two neighbours.
static double sig_tempo_BeatTracker_refineBeat(
    struct sig_tempo_BeatTracker* self, size_t start, size_t available,
    size_t position) {
    if (position == 0 || position + 1 >= available) {
        return 0.0;
    }

    double before = sig_tempo_BeatTracker_readWindow(self, start,
        position - 1);
    double centre = sig_tempo_BeatTracker_readWindow(self, start, position);
    double after = sig_tempo_BeatTracker_readWindow(self, start,
        position + 1);
    double curvature = before - 2.0 * centre + after;
    if (curvature >= 0.0) {
        return 0.0;
    }

    double offset = 0.5 * (before - after) / curvature;

    return offset < -0.5 ? -0.5 : (offset > 0.5 ? 0.5 : offset);
}

// Adds a Gaussian bump at each of the follower's predicted beats
// throughout the normalised window. The beat sequence at the tapped
// phase then competes with the stronger sequence over the whole window,
// so the program keeps the tapped phase unless its onsets are much
// weaker.
static void sig_tempo_BeatTracker_favourGrid(
    struct sig_tempo_BeatTracker* self, size_t available) {
    float* SIG_RESTRICT scores = FLOAT_ARRAY(self->scores);
    const size_t firstIndex = self->valueCount - available;
    const double width = SIG_TEMPO_HOLD_WIDTH;

    for (size_t p = 0; p < available; p++) {
        // An onset's two equal envelope values centre on the hop after
        // the first value's index.
        double time = ((double) (firstIndex + p) + 1.5) *
            (double) self->hopLength - (double) self->latency;
        double position = (time - self->anchor) / self->period;
        double distance = position - floor(position + 0.5);
        scores[p] += SIG_TEMPO_HOLD_GAIN *
            (float) exp(-0.5 * distance * distance / (width * width));
    }
}

// Fills the scores and predecessors with the dynamic program's
// cumulative scores and best predecessors for a beat interval, and
// returns false if the window is silent or too short.
static bool sig_tempo_BeatTracker_scoreWindow(
    struct sig_tempo_BeatTracker* self, size_t start, size_t available,
    float beatInterval) {
    float* SIG_RESTRICT scores = FLOAT_ARRAY(self->scores);
    float* SIG_RESTRICT predecessors = FLOAT_ARRAY(self->predecessors);
    float* SIG_RESTRICT penalties = FLOAT_ARRAY(self->penalties);

    // The program needs a full predecessor range in the window, and
    // the backtrace's bound requires a minimum beat interval.
    if (beatInterval < (float) self->windowLength / 64.0f ||
        (float) available < SIG_TEMPO_DP_MAX_INTERVAL_RATIO *
            beatInterval + 1.0f) {
        return false;
    }

    float maxValue = 0.0f;
    for (size_t p = 0; p < available; p++) {
        float value = sig_tempo_BeatTracker_readWindow(self, start, p);
        scores[p] = value;
        if (value > maxValue) {
            maxValue = value;
        }
    }

    if (maxValue < SIG_TEMPO_SILENCE) {
        return false;
    }

    for (size_t p = 0; p < available; p++) {
        scores[p] /= maxValue;
    }

    if (self->isHoldingTaps &&
        self->state == sig_tempo_BeatTracker_State_TRACKING) {
        sig_tempo_BeatTracker_favourGrid(self, available);
    }

    size_t maxDistance = (size_t) (SIG_TEMPO_DP_MAX_INTERVAL_RATIO *
        beatInterval);
    size_t minDistance = (size_t) ceilf(SIG_TEMPO_DP_MIN_INTERVAL_RATIO *
        beatInterval);
    for (size_t distance = 1; distance <= maxDistance; distance++) {
        double logRatio = log((double) distance / (double) beatInterval);
        penalties[distance] = SIG_TEMPO_DP_TIGHTNESS *
            (float) (logRatio * logRatio);
    }

    for (size_t p = 0; p < available; p++) {
        // Positions closer to the start than the nearest predecessor
        // have no predecessor.
        if (p < minDistance) {
            predecessors[p] = 0.0f;
            continue;
        }

        size_t first = p > maxDistance ? p - maxDistance : 0;
        size_t last = p - minDistance;
        float best = scores[first] - penalties[p - first];
        size_t bestPredecessor = first;
        for (size_t j = first + 1; j <= last; j++) {
            float candidate = scores[j] - penalties[p - j];
            if (candidate > best) {
                best = candidate;
                bestPredecessor = j;
            }
        }

        scores[p] += best;
        predecessors[p] = (float) (p - bestPredecessor);
    }

    return true;
}

// Traces the best beat sequence back from the best position in the
// window's final beat interval, writes the beats' window positions
// into beats, newest first, and returns their number.
static size_t sig_tempo_BeatTracker_backtrace(
    struct sig_tempo_BeatTracker* self, size_t available,
    float beatInterval, size_t* beats) {
    const float* SIG_RESTRICT scores = FLOAT_ARRAY(self->scores);
    const float* SIG_RESTRICT predecessors = FLOAT_ARRAY(self->predecessors);

    size_t tailStart = available - (size_t) beatInterval;
    size_t p = tailStart;
    for (size_t q = tailStart + 1; q < available; q++) {
        if (scores[q] > scores[p]) {
            p = q;
        }
    }

    size_t numBeats = 0;
    while (numBeats < SIG_TEMPO_DP_MAX_BEATS) {
        beats[numBeats] = p;
        numBeats++;

        float offset = predecessors[p];
        if (offset <= 0.0f) {
            break;
        }
        p -= (size_t) offset;
    }

    return numBeats;
}

// Runs the dynamic program over the window, and commits the beats that
// lie far enough behind the newest envelope value.
static void sig_tempo_BeatTracker_runDynamicProgram(
    struct sig_tempo_BeatTracker* self, float tempo) {
    const size_t windowLength = self->windowLength;
    const size_t available = self->valueCount < windowLength ?
        self->valueCount : windowLength;
    const size_t start = available == windowLength ? self->writeIdx : 0;
    const size_t firstIndex = self->valueCount - available;
    const float beatInterval = self->envelopeRate * 60.0f / tempo;

    if (!sig_tempo_BeatTracker_scoreWindow(self, start, available,
        beatInterval)) {
        return;
    }

    size_t beats[SIG_TEMPO_DP_MAX_BEATS];
    size_t numBeats = sig_tempo_BeatTracker_backtrace(self, available,
        beatInterval, beats);

    const float horizon = (float) (self->valueCount - 1) -
        SIG_TEMPO_COMMIT_HORIZON * beatInterval;
    for (size_t b = numBeats; b > 0; b--) {
        size_t position = beats[b - 1];
        size_t index = firstIndex + position;
        if ((float) index > horizon) {
            break;
        }
        if (self->hasCommitted && (index <= self->lastCommittedIndex ||
            (float) (index - self->lastCommittedIndex) <
                SIG_TEMPO_COMMIT_HORIZON * beatInterval)) {
            continue;
        }

        // Envelope value i describes the hop that ends at sample
        // (i + 1) * hopLength.
        double offset = sig_tempo_BeatTracker_refineBeat(self, start,
            available, position);
        double time = ((double) index + 1.0 + offset) *
            (double) self->hopLength - (double) self->latency;
        self->lastCommittedIndex = index;
        self->hasCommitted = true;
        sig_tempo_BeatTracker_commitBeat(self, time,
            sig_tempo_BeatTracker_readWindow(self, start, position));
    }
}

// Starts the follower at the first tempo estimate, and reseeds the grid
// when the estimate moves to a different metrical level.
static void sig_tempo_BeatTracker_followEstimate(
    struct sig_tempo_BeatTracker* self, float tempo) {
    double estimatePeriod = sig_tempo_BeatTracker_tempoToPeriod(self,
        tempo);

    if (self->state == sig_tempo_BeatTracker_State_WAITING) {
        self->period = estimatePeriod;
        self->anchor = self->sampleClock;
        self->position = 0.0;
        self->state = sig_tempo_BeatTracker_State_SEARCHING;
        return;
    }

    if (self->isHoldingTaps) {
        return;
    }

    double ratio = estimatePeriod / self->period;
    if (ratio > SIG_TEMPO_RESEED_RATIO ||
        ratio < 1.0 / SIG_TEMPO_RESEED_RATIO) {
        double beats = floor((self->sampleClock - self->anchor) /
            self->period + 0.5);
        self->anchor += beats * self->period;
        self->period = estimatePeriod;
        self->hasLastAccepted = false;
        self->numOutliers = 0;
        self->confidence = 0.0f;
        self->state = sig_tempo_BeatTracker_State_SEARCHING;
    }
}

// Makes the follower silent when no strong beat has arrived for the
// silence timeout. A beat commits up to half a beat interval plus one
// run of the dynamic program after it sounds, so the follower waits for
// that delay too.
static void sig_tempo_BeatTracker_checkSilence(
    struct sig_tempo_BeatTracker* self) {
    if (self->state != sig_tempo_BeatTracker_State_TRACKING) {
        return;
    }

    double timeout = ((double) self->silenceTimeout +
        SIG_TEMPO_COMMIT_HORIZON) * self->period +
        (double) (SIG_TEMPO_DP_INTERVAL * self->hopLength);
    if (self->sampleClock - self->lastStrongBeatTime > timeout) {
        self->state = sig_tempo_BeatTracker_State_SILENT;
        self->confidence = 0.0f;
        self->isHoldingTaps = false;
    }
}

void sig_tempo_BeatTracker_analyse(struct sig_tempo_BeatTracker* self,
    float value, float tempo) {
    float* SIG_RESTRICT envelope = FLOAT_ARRAY(self->envelope);
    envelope[self->writeIdx] = value;
    self->writeIdx++;
    if (self->writeIdx == self->windowLength) {
        self->writeIdx = 0;
    }
    self->valueCount++;

    if (tempo <= 0.0f) {
        if (self->state == sig_tempo_BeatTracker_State_WAITING) {
            return;
        }
        tempo = sig_tempo_BeatTracker_tempo(self);
    }

    tempo = sig_clamp(tempo, self->minTempo, self->maxTempo);
    sig_tempo_BeatTracker_followEstimate(self, tempo);
    sig_tempo_BeatTracker_limitPeriod(self);

    if (self->valueCount % SIG_TEMPO_DP_INTERVAL == 0) {
        float programTempo = self->state ==
            sig_tempo_BeatTracker_State_TRACKING ?
            sig_tempo_BeatTracker_tempo(self) : tempo;
        sig_tempo_BeatTracker_runDynamicProgram(self, programTempo);
    }

    sig_tempo_BeatTracker_checkSilence(self);
}

// Records a tap's time, and returns the number of taps that measure
// the current tapped tempo.
static size_t sig_tempo_BeatTracker_recordTap(
    struct sig_tempo_BeatTracker* self, double time) {
    double longestPeriod = sig_tempo_BeatTracker_tempoToPeriod(self,
        self->minTempo);
    if (self->numTaps > 0 &&
        time - self->tapTimes[self->numTaps - 1] > longestPeriod) {
        self->numTaps = 0;
    }

    if (self->numTaps == sig_tempo_BeatTracker_TAP_INTERVALS + 1) {
        for (size_t i = 1; i < self->numTaps; i++) {
            self->tapTimes[i - 1] = self->tapTimes[i];
        }
        self->numTaps--;
    }
    self->tapTimes[self->numTaps] = time;
    self->numTaps++;

    return self->numTaps;
}

void sig_tempo_BeatTracker_tap(struct sig_tempo_BeatTracker* self) {
    double time = self->sampleClock;
    size_t numTaps = sig_tempo_BeatTracker_recordTap(self, time);

    if (numTaps >= 2) {
        self->period = (time - self->tapTimes[0]) / (double) (numTaps - 1);
        sig_tempo_BeatTracker_limitPeriod(self);
    } else if (self->state == sig_tempo_BeatTracker_State_WAITING) {
        return;
    }

    if (self->state == sig_tempo_BeatTracker_State_WAITING) {
        self->position = 0.0;
    }

    sig_tempo_BeatTracker_anchorAt(self, time);
    self->state = sig_tempo_BeatTracker_State_TRACKING;
    self->lastStrongBeatTime = time;
    self->isHoldingTaps = true;
    if (self->confidence < SIG_TEMPO_TAP_CONFIDENCE) {
        self->confidence = SIG_TEMPO_TAP_CONFIDENCE;
    }
}

extern inline float sig_tempo_BeatTracker_follow(
    struct sig_tempo_BeatTracker* self);

extern inline float sig_tempo_BeatTracker_tempo(
    struct sig_tempo_BeatTracker* self);

extern inline float sig_tempo_BeatTracker_confidence(
    struct sig_tempo_BeatTracker* self);

extern inline enum sig_tempo_BeatTracker_State sig_tempo_BeatTracker_state(
    struct sig_tempo_BeatTracker* self);

extern inline void sig_tempo_BeatTracker_setResponsiveness(
    struct sig_tempo_BeatTracker* self, float responsiveness);

extern inline void sig_tempo_BeatTracker_setTempoRange(
    struct sig_tempo_BeatTracker* self, float minTempo, float maxTempo);

extern inline void sig_tempo_BeatTracker_setSilenceTimeout(
    struct sig_tempo_BeatTracker* self, float silenceTimeout);

void sig_tempo_BeatTracker_destroy(struct sig_Allocator* allocator,
    struct sig_tempo_BeatTracker* self) {
    allocator->impl->free(allocator, self->envelope);
    allocator->impl->free(allocator, self->scores);
    allocator->impl->free(allocator, self->predecessors);
    allocator->impl->free(allocator, self->penalties);
    allocator->impl->free(allocator, self);
}

void sig_tempo_ClockGenerator_init(struct sig_tempo_ClockGenerator* self,
    float sampleRate) {
    self->sampleRate = sampleRate;
    self->lead = 0.0f;
    self->silenceBehaviour = 0.0f;
    self->triggerSamplesRemaining = 0;
    self->previousPhase = 0.0f;
    self->wasActive = false;
    self->isArmed = false;
    sig_tempo_ClockGenerator_setTriggerDuration(self, 0.01f);
}

extern inline void sig_tempo_ClockGenerator_setLead(
    struct sig_tempo_ClockGenerator* self, float lead);

extern inline void sig_tempo_ClockGenerator_setTriggerDuration(
    struct sig_tempo_ClockGenerator* self, float triggerDuration);

extern inline void sig_tempo_ClockGenerator_setSilenceBehaviour(
    struct sig_tempo_ClockGenerator* self, float silenceBehaviour);

extern inline void sig_tempo_ClockGenerator_generate(
    struct sig_tempo_ClockGenerator* self, float phase, float tempo,
    float confidence, enum sig_tempo_BeatTracker_State state,
    struct sig_tempo_ClockGenerator_Outputs* outputs);

void sig_dsp_Signal_init(void* signal,
    struct sig_SignalContext* context,
    sig_dsp_generateFn generate) {
    struct sig_dsp_Signal* self = (struct sig_dsp_Signal*) signal;

    self->audioSettings = context->audioSettings;
    self->generate = generate;
}

void sig_dsp_Signal_noOp(void* signal) {};

// TODO: Should self be defined as a struct_dsp_Signal* in the signature?
void sig_dsp_Signal_destroy(struct sig_Allocator* allocator,
    void* self) {
    struct sig_dsp_Signal* signal = (struct sig_dsp_Signal*) self;
    allocator->impl->free(allocator, signal);
}


void sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_Signal_SingleMonoOutput* outputs) {
    outputs->main = sig_AudioBlock_newSilent(allocator, audioSettings);
}

void sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_Signal_SingleMonoOutput* outputs) {
    sig_AudioBlock_destroy(allocator, outputs->main);
}


extern inline void sig_dsp_evaluateSignals(
    struct sig_List* signalList) {
    for (size_t i = 0; i < signalList->length; i++) {
        struct sig_dsp_Signal* signal =
            (struct sig_dsp_Signal*) signalList->items[i];
        signal->generate(signal);
    }
}

struct sig_dsp_SignalListEvaluator* sig_dsp_SignalListEvaluator_new(
    struct sig_Allocator* allocator, struct sig_List* signalList) {
    struct sig_dsp_SignalListEvaluator* self = sig_MALLOC(allocator,
        struct sig_dsp_SignalListEvaluator);
    sig_dsp_SignalListEvaluator_init(self, signalList);

    return self;
}

void sig_dsp_SignalListEvaluator_init(
    struct sig_dsp_SignalListEvaluator* self,
    struct sig_List* signalList) {
    self->evaluate = sig_dsp_SignalListEvaluator_evaluate;
    self->signalList = signalList;
}

void sig_dsp_SignalListEvaluator_evaluate(
    struct sig_dsp_SignalEvaluator* evaluator) {
    struct sig_dsp_SignalListEvaluator* self =
        (struct sig_dsp_SignalListEvaluator*) evaluator;

    sig_dsp_evaluateSignals(self->signalList);
}

void sig_dsp_SignalListEvaluator_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_SignalListEvaluator* self) {
    allocator->impl->free(allocator, self);
}

struct sig_dsp_Value* sig_dsp_Value_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Value* self = sig_MALLOC(allocator, struct sig_dsp_Value);
    sig_dsp_Value_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Value_init(struct sig_dsp_Value* self,
    struct sig_SignalContext* context) {

    struct sig_dsp_Value_Parameters params = {
        .value = 1.0f
    };

    sig_dsp_Signal_init(self, context, *sig_dsp_Value_generate);

    self->parameters = params;
}

void sig_dsp_Value_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Value* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}

void sig_dsp_Value_generate(void* signal) {
    struct sig_dsp_Value* self = (struct sig_dsp_Value*) signal;

    if (self->parameters.value == self->lastSample) {
        return;
    }

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float value = self->parameters.value;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = value;
    }

    self->lastSample = self->parameters.value;
}


struct sig_dsp_ConstantValue* sig_dsp_ConstantValue_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context,
    float value) {
    struct sig_dsp_ConstantValue* self = sig_MALLOC(allocator,
        struct sig_dsp_ConstantValue);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);
    sig_dsp_ConstantValue_init(self, context, value);

    return self;
}

void sig_dsp_ConstantValue_init(struct sig_dsp_ConstantValue* self,
    struct sig_SignalContext* context, float value) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Signal_noOp);
    sig_fillWithValue(self->outputs.main, context->audioSettings->blockSize,
        value);
};

void sig_dsp_ConstantValue_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ConstantValue* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
};


struct sig_dsp_Abs* sig_dsp_Abs_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Abs* self = sig_MALLOC(allocator,
        struct sig_dsp_Abs);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);
    sig_dsp_Abs_init(self, context);

    return self;
}

void sig_dsp_Abs_init(struct sig_dsp_Abs* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Abs_generate);
    sig_CONNECT_TO_SILENCE(self, source, context);
}

void sig_dsp_Abs_generate(void* signal) {
    struct sig_dsp_Abs* self = (struct sig_dsp_Abs*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = fabsf(in[i]);
    }
}

void sig_dsp_Abs_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Abs* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


struct sig_dsp_Clamp* sig_dsp_Clamp_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Clamp* self = sig_MALLOC(allocator,
        struct sig_dsp_Clamp);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);
    sig_dsp_Clamp_init(self, context);

    return self;
}

void sig_dsp_Clamp_init(struct sig_dsp_Clamp* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Clamp_generate);

    self->parameters.min = 0.0f;
    self->parameters.max = 1.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
}

void sig_dsp_Clamp_generate(void* signal) {
    struct sig_dsp_Clamp* self = (struct sig_dsp_Clamp*) signal;
    const float min = self->parameters.min;
    const float max = self->parameters.max;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_clamp(in[i], min, max);
    }
}

void sig_dsp_Clamp_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Clamp* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}

struct sig_dsp_SnapToInteger* sig_dsp_SnapToInteger_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context) {
    struct sig_dsp_SnapToInteger* self = sig_MALLOC(allocator,
        struct sig_dsp_SnapToInteger);
    sig_dsp_SnapToInteger_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_SnapToInteger_init(struct sig_dsp_SnapToInteger* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_SnapToInteger_generate);
    self->parameters.tolerance = 0.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
}

void sig_dsp_SnapToInteger_generate(void* signal) {
    struct sig_dsp_SnapToInteger* self =
        (struct sig_dsp_SnapToInteger*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float tolerance = self->parameters.tolerance;
    const float* SIG_RESTRICT source = FLOAT_ARRAY(self->inputs.source);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);

    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_snapToInteger(source[i], tolerance);
    }
}

void sig_dsp_SnapToInteger_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SnapToInteger* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


struct sig_dsp_ScaleOffset* sig_dsp_ScaleOffset_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_ScaleOffset* self = sig_MALLOC(allocator,
        struct sig_dsp_ScaleOffset);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);
    sig_dsp_ScaleOffset_init(self, context);

    return self;
}

void sig_dsp_ScaleOffset_init(struct sig_dsp_ScaleOffset* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_ScaleOffset_generate);

    struct sig_dsp_ScaleOffset_Parameters parameters = {
        .scale = 1.0,
        .offset= 0.0f
    };
    self->parameters = parameters;

    sig_CONNECT_TO_SILENCE(self, source, context);
}

void sig_dsp_ScaleOffset_generate(void* signal) {
    struct sig_dsp_ScaleOffset* self = (struct sig_dsp_ScaleOffset*) signal;

    const float scale = self->parameters.scale;
    const float offset = self->parameters.offset;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = in[i] * scale + offset;
    }
}

void sig_dsp_ScaleOffset_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ScaleOffset* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


struct sig_dsp_Sine* sig_dsp_Sine_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Sine* self = sig_MALLOC(allocator,
        struct sig_dsp_Sine);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);
    sig_dsp_Sine_init(self, context);

    return self;
};

void sig_dsp_Sine_init(struct sig_dsp_Sine* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Sine_generate);

    sig_CONNECT_TO_SILENCE(self, source, context);
}

void sig_dsp_Sine_generate(void* signal) {
    struct sig_dsp_Sine* self = (struct sig_dsp_Sine*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sinf(in[i]);
    }
}

void sig_dsp_Sine_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Sine* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


struct sig_dsp_BinaryOp* sig_dsp_BinaryOp_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context,
    sig_dsp_generateFn generate) {
    struct sig_dsp_BinaryOp* self = sig_MALLOC(allocator,
        struct sig_dsp_BinaryOp);
    sig_dsp_BinaryOp_init(self, context, generate);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_BinaryOp_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context,
    sig_dsp_generateFn generate) {
    sig_dsp_Signal_init(self, context, generate);

    sig_CONNECT_TO_SILENCE(self, left, context);
    sig_CONNECT_TO_SILENCE(self, right, context);
}

void sig_dsp_BinaryOp_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_BinaryOp* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}

struct sig_dsp_BinaryOp* sig_dsp_Add_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    return sig_dsp_BinaryOp_new(allocator, context, *sig_dsp_Add_generate);
}

void sig_dsp_Add_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context) {
    sig_dsp_BinaryOp_init(self, context, *sig_dsp_Add_generate);
}

// TODO: Unit tests.
void sig_dsp_Add_generate(void* signal) {
    struct sig_dsp_BinaryOp* self = (struct sig_dsp_BinaryOp*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT left = FLOAT_ARRAY(self->inputs.left);
    const float* SIG_RESTRICT right = FLOAT_ARRAY(self->inputs.right);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = left[i] + right[i];
    }
}

void sig_dsp_Add_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_BinaryOp* self) {
    sig_dsp_BinaryOp_destroy(allocator, self);
}


struct sig_dsp_BinaryOp* sig_dsp_Sub_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    return sig_dsp_BinaryOp_new(allocator, context, *sig_dsp_Sub_generate);
}

void sig_dsp_Sub_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context) {
    sig_dsp_BinaryOp_init(self, context, *sig_dsp_Sub_generate);
}

// TODO: Unit tests.
void sig_dsp_Sub_generate(void* signal) {
    struct sig_dsp_BinaryOp* self = (struct sig_dsp_BinaryOp*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT left = FLOAT_ARRAY(self->inputs.left);
    const float* SIG_RESTRICT right = FLOAT_ARRAY(self->inputs.right);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = left[i] - right[i];
    }
}

void sig_dsp_Sub_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_BinaryOp* self) {
    sig_dsp_BinaryOp_destroy(allocator, self);
}


struct sig_dsp_BinaryOp* sig_dsp_Mul_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    return sig_dsp_BinaryOp_new(allocator, context, *sig_dsp_Mul_generate);
}

void sig_dsp_Mul_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context) {
    sig_dsp_BinaryOp_init(self, context, *sig_dsp_Mul_generate);
};

void sig_dsp_Mul_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_BinaryOp* self) {
    sig_dsp_BinaryOp_destroy(allocator, self);
};

void sig_dsp_Mul_generate(void* signal) {
    struct sig_dsp_BinaryOp* self = (struct sig_dsp_BinaryOp*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT left = FLOAT_ARRAY(self->inputs.left);
    const float* SIG_RESTRICT right = FLOAT_ARRAY(self->inputs.right);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = left[i] * right[i];
    }
}


struct sig_dsp_BinaryOp* sig_dsp_Div_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    return sig_dsp_BinaryOp_new(allocator, context, *sig_dsp_Div_generate);
}

void sig_dsp_Div_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context) {
    sig_dsp_BinaryOp_init(self, context, *sig_dsp_Div_generate);
}

void sig_dsp_Div_generate(void* signal) {
    struct sig_dsp_BinaryOp* self = (struct sig_dsp_BinaryOp*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT left = FLOAT_ARRAY(self->inputs.left);
    const float* SIG_RESTRICT right = FLOAT_ARRAY(self->inputs.right);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = left[i] / right[i];
    }
}

void sig_dsp_Div_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_BinaryOp* self) {
    sig_dsp_BinaryOp_destroy(allocator, self);
}


struct sig_dsp_Invert* sig_dsp_Invert_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Invert* self = sig_MALLOC(allocator, struct sig_dsp_Invert);
    sig_dsp_Invert_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Invert_init(struct sig_dsp_Invert* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Invert_generate);
    self->inputs.source = context->silence->outputs.main;
}

// TODO: Unit tests.
void sig_dsp_Invert_generate(void* signal) {
    struct sig_dsp_Invert* self = (struct sig_dsp_Invert*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = -in[i];
    }
}

void sig_dsp_Invert_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Invert* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_Accumulate* sig_dsp_Accumulate_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Accumulate* self = sig_MALLOC(allocator,
        struct sig_dsp_Accumulate);
    sig_dsp_Accumulate_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);
    return self;
}

void sig_dsp_Accumulate_init(struct sig_dsp_Accumulate* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Accumulate_generate);

    struct sig_dsp_Accumulate_Parameters parameters = {
        .accumulatorStart = 1.0f,
        .wrap = 0.0f,
        .maxValue = 1.0f
    };
    self->parameters = parameters;

    // FIXME: This happens too early for users to
    // override the parameter, and thus the accumulator
    // is always initialized to 1.
    self->accumulator = parameters.accumulatorStart;
    self->previousReset = 0.0f;

    self->inputs.source = context->silence->outputs.main;
    self->inputs.reset = context->silence->outputs.main;
}

// TODO: Implement an audio rate version of this signal.
// TODO: Unit tests
void sig_dsp_Accumulate_generate(void* signal) {
    struct sig_dsp_Accumulate* self =
        (struct sig_dsp_Accumulate*) signal;

    self->accumulator += FLOAT_ARRAY(self->inputs.source)[0];

    float reset = FLOAT_ARRAY(self->inputs.reset)[0];
    const float wrap = self->parameters.wrap;
    const float maxValue = self->parameters.maxValue;

    // Reset the accumulator if we received a trigger
    // or if we're wrapping and we've gone past the min/max value.
    if (reset > 0.0f && self->previousReset <= 0.0f) {
        self->accumulator = self->parameters.accumulatorStart;
    } else if (wrap > 0.0f) {
        if (self->accumulator > maxValue) {
            self->accumulator = self->parameters.accumulatorStart;
        } else if (self->accumulator < self->parameters.accumulatorStart) {
            self->accumulator = self->parameters.maxValue;
        }
    }

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = self->accumulator;
    }

    self->previousReset = reset;
}

void sig_dsp_Accumulate_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Accumulate* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


struct sig_dsp_GatedTimer* sig_dsp_GatedTimer_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_GatedTimer* self = sig_MALLOC(allocator,
        struct sig_dsp_GatedTimer);
    sig_dsp_GatedTimer_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);
    return self;
}

void sig_dsp_GatedTimer_init(struct sig_dsp_GatedTimer* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_GatedTimer_generate);

    self->timer = 0;
    self->hasFired = false;
    self->prevGate = 0.0f;

    sig_CONNECT_TO_SILENCE(self, gate, context);
    sig_CONNECT_TO_SILENCE(self, duration, context);
    sig_CONNECT_TO_SILENCE(self, loop, context);
}

// TODO: Unit tests
void sig_dsp_GatedTimer_generate(void* signal) {
    struct sig_dsp_GatedTimer* self =
        (struct sig_dsp_GatedTimer*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT durationIn = FLOAT_ARRAY(self->inputs.duration);
    const float* SIG_RESTRICT gateIn = FLOAT_ARRAY(self->inputs.gate);
    const float* SIG_RESTRICT loopIn = FLOAT_ARRAY(self->inputs.loop);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    unsigned long timer = self->timer;
    bool hasFired = self->hasFired;
    float prevGate = self->prevGate;

    for (size_t i = 0; i < blockSize; i++) {
        // TODO: MSVC compiler warning loss of precision.
        unsigned long durationSamps = (unsigned long)
            (durationIn[i] *
            sampleRate);
        float gate = gateIn[i];

        if (gate > 0.0f) {
            // Gate is open.
            if (!hasFired ||
                loopIn[i] > 0.0f) {
                timer++;
            }

            if (timer >= durationSamps) {
                // We reached the duration time.
                out[i] = 1.0f;

                // Reset the timer counter and note
                // that we've already fired while
                // this gate was open.
                timer = 0;
                hasFired = true;

                continue;
            }
        } else if (gate <= 0.0f && prevGate > 0.0f) {
            // Gate just closed. Reset all timer state.
            timer = 0;
            hasFired = false;
        }

        out[i] = 0.0f;
        prevGate = gate;
    }

    self->timer = timer;
    self->hasFired = hasFired;
    self->prevGate = prevGate;
}

void sig_dsp_GatedTimer_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_GatedTimer* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}

struct sig_dsp_TimedTriggerCounter* sig_dsp_TimedTriggerCounter_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_TimedTriggerCounter* self = sig_MALLOC(allocator,
        struct sig_dsp_TimedTriggerCounter);
    sig_dsp_TimedTriggerCounter_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_TimedTriggerCounter_init(
    struct sig_dsp_TimedTriggerCounter* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_TimedTriggerCounter_generate);

    self->numTriggers = 0;
    self->timer = 0;
    self->isTimerActive = false;
    self->previousSource = 0.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, duration, context);
    sig_CONNECT_TO_SILENCE(self, count, context);
}

void sig_dsp_TimedTriggerCounter_generate(void* signal) {
    struct sig_dsp_TimedTriggerCounter* self =
        (struct sig_dsp_TimedTriggerCounter*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT sourceIn = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT durationIn = FLOAT_ARRAY(self->inputs.duration);
    const float* SIG_RESTRICT countIn = FLOAT_ARRAY(self->inputs.count);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    int numTriggers = self->numTriggers;
    long timer = self->timer;
    bool isTimerActive = self->isTimerActive;
    float previousSource = self->previousSource;

    for (size_t i = 0; i < blockSize; i++) {
        float source = sourceIn[i];
        float outputSample = 0.0f;

        if (source > 0.0f && previousSource == 0.0f) {
            // Received the rising edge of a trigger.
            if (!isTimerActive) {
                // It's the first trigger,
                // so start the timer.
                isTimerActive = true;
            }
        }

        if (isTimerActive) {
            // The timer is running.
            if (source <= 0.0f && previousSource > 0.0f) {
                // Received the falling edge of a trigger,
                // so count it.
                numTriggers++;
            }

            timer++;

            // Truncate the duration to the nearest sample.
            long durSamps = (long) (durationIn[i] *
                sampleRate);

            if (timer >= durSamps) {
                // Time's up.
                // Fire a trigger if we've the right number of
                // incoming triggers, otherwise just reset.
                if (numTriggers ==
                    (int) countIn[i]) {
                    outputSample = 1.0f;
                }

                isTimerActive = false;
                numTriggers = 0;
                timer = 0;
            }
        }

        previousSource = source;
        out[i] = outputSample;
    }

    self->numTriggers = numTriggers;
    self->timer = timer;
    self->isTimerActive = isTimerActive;
    self->previousSource = previousSource;
}

void sig_dsp_TimedTriggerCounter_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_TimedTriggerCounter* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


struct sig_dsp_ToggleGate* sig_dsp_ToggleGate_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_ToggleGate* self = sig_MALLOC(allocator,
        struct sig_dsp_ToggleGate);
    sig_dsp_ToggleGate_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_ToggleGate_init(struct sig_dsp_ToggleGate* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_ToggleGate_generate);

    self->isGateOpen = false;
    self->prevTrig = 0.0f;

    sig_CONNECT_TO_SILENCE(self, trigger, context);
}

// TODO: Unit tests
void sig_dsp_ToggleGate_generate(void* signal) {
    struct sig_dsp_ToggleGate* self =
        (struct sig_dsp_ToggleGate*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT trigger = FLOAT_ARRAY(self->inputs.trigger);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    bool isGateOpen = self->isGateOpen;
    float prevTrig = self->prevTrig;

    for (size_t i = 0; i < blockSize; i++) {
        float trig = trigger[i];
        if (trig > 0.0f && prevTrig <= 0.0f) {
            // Received a trigger, toggle the gate.
            isGateOpen = !isGateOpen;
        }

        out[i] = (float) isGateOpen;

        prevTrig = trig;
    }

    self->isGateOpen = isGateOpen;
    self->prevTrig = prevTrig;
}

void sig_dsp_ToggleGate_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_ToggleGate* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}

void sig_dsp_Oscillator_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_Oscillator_Outputs* outputs) {
    outputs->main = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->eoc = sig_AudioBlock_newSilent(allocator, audioSettings);
}

void sig_dsp_Oscillator_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator_Outputs* outputs) {
    sig_AudioBlock_destroy(allocator, outputs->main);
    outputs->main = NULL;
    sig_AudioBlock_destroy(allocator, outputs->eoc);
    outputs->eoc = NULL;
}

struct sig_dsp_Oscillator* sig_dsp_Oscillator_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context,
    sig_dsp_generateFn generate) {
    struct sig_dsp_Oscillator* self = sig_MALLOC(allocator,
        struct sig_dsp_Oscillator);
    sig_dsp_Oscillator_init(self, context, generate);
    sig_dsp_Oscillator_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Oscillator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator* self) {
    sig_dsp_Oscillator_Outputs_destroyAudioBlocks(allocator, &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}

void sig_dsp_Oscillator_init(struct sig_dsp_Oscillator* self,
    struct sig_SignalContext* context,
    sig_dsp_generateFn generate) {
    sig_dsp_Signal_init(self, context, generate);

    sig_CONNECT_TO_SILENCE(self, freq, context);
    sig_CONNECT_TO_SILENCE(self, phaseOffset, context);
    sig_CONNECT_TO_UNITY(self, mul, context);
    sig_CONNECT_TO_SILENCE(self, add, context);

    sig_osc_Oscillator_init(&self->state);
}

void sig_dsp_SineOscillator_init(struct sig_dsp_Oscillator* self,
    struct sig_SignalContext* context) {
    sig_dsp_Oscillator_init(self, context, *sig_dsp_SineOscillator_generate);
}

struct sig_dsp_Oscillator* sig_dsp_SineOscillator_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    return sig_dsp_Oscillator_new(allocator, context, *sig_dsp_SineOscillator_generate);
}

void sig_dsp_SineOscillator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator* self) {
    sig_dsp_Oscillator_destroy(allocator, self);
}

void sig_dsp_SineOscillator_generate(void* signal) {
    struct sig_dsp_Oscillator* self = (struct sig_dsp_Oscillator*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float* SIG_RESTRICT frequency = FLOAT_ARRAY(self->inputs.freq);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(self->inputs.phaseOffset);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT mainOutput = FLOAT_ARRAY(self->outputs.main);
    float* SIG_RESTRICT eocOutput = FLOAT_ARRAY(self->outputs.eoc);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    float phaseAccumulator = self->state.phaseAccumulator;

    for (size_t i = 0; i < blockSize; i++) {
        float modulatedPhase = phaseAccumulator + phaseOffset[i];
        float eoc = sig_osc_Oscillator_eoc(modulatedPhase);
        modulatedPhase = sig_osc_Oscillator_wrapPhase(modulatedPhase);

        float angularPhase = modulatedPhase * sig_TWOPI;
        float sample = sinf(angularPhase);
        float scaledSample = sample * mul[i] + add[i];

        mainOutput[i] = scaledSample;
        eocOutput[i] = eoc;

        sig_osc_Oscillator_accumulatePhase(&phaseAccumulator,
            frequency[i], sampleRate);
    }

    self->state.phaseAccumulator = phaseAccumulator;
}


void sig_dsp_FastLFSineOscillator_init(
    struct sig_dsp_FastLFSineOscillator* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_FastLFSineOscillator_generate);

    sig_CONNECT_TO_SILENCE(self, frequency, context);
    sig_CONNECT_TO_UNITY(self, mul, context);
    sig_CONNECT_TO_SILENCE(self, add, context);

    sig_osc_FastLFSine_init(&self->state,
        self->signal.audioSettings->sampleRate);
}

struct sig_dsp_FastLFSineOscillator* sig_dsp_FastLFSineOscillator_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_FastLFSineOscillator* self = sig_MALLOC(allocator,
        struct sig_dsp_FastLFSineOscillator);
    sig_dsp_FastLFSineOscillator_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_FastLFSineOscillator_generate(void* signal) {
    struct sig_dsp_FastLFSineOscillator* self =
        (struct sig_dsp_FastLFSineOscillator*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float* SIG_RESTRICT frequency = FLOAT_ARRAY(self->inputs.frequency);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT mainOutput = FLOAT_ARRAY(self->outputs.main);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    struct sig_osc_FastLFSine state = self->state;

    for (size_t i = 0; i < blockSize; i++) {
        sig_osc_FastLFSine_setFrequencyFast(&state, frequency[i]);
        sig_osc_FastLFSine_generate(&state);
        float sample = state.sinZ;
        float scaledSample = sample * mul[i] + add[i];
        mainOutput[i] = scaledSample;
    }

    self->state = state;
}

void sig_dsp_FastLFSineOscillator_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_FastLFSineOscillator* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
    self = NULL;
}


void sig_dsp_LFTriangle_init(struct sig_dsp_Oscillator* self,
    struct sig_SignalContext* context) {
    sig_dsp_Oscillator_init(self, context, *sig_dsp_LFTriangle_generate);
};

struct sig_dsp_Oscillator* sig_dsp_LFTriangle_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    return sig_dsp_Oscillator_new(allocator, context,
        *sig_dsp_LFTriangle_generate);
}

void sig_dsp_LFTriangle_generate(void* signal) {
    struct sig_dsp_Oscillator* self = (struct sig_dsp_Oscillator*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float* SIG_RESTRICT frequency = FLOAT_ARRAY(self->inputs.freq);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(self->inputs.phaseOffset);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT mainOutput = FLOAT_ARRAY(self->outputs.main);
    float* SIG_RESTRICT eocOutput = FLOAT_ARRAY(self->outputs.eoc);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    float phaseAccumulator = self->state.phaseAccumulator;

    for (size_t i = 0; i < blockSize; i++) {
        float modulatedPhase = phaseAccumulator + phaseOffset[i];
        float eoc = sig_osc_Oscillator_eoc(modulatedPhase);
        modulatedPhase = sig_osc_Oscillator_wrapPhase(modulatedPhase) *
            sig_TWOPI;

        float sample = -1.0f + (2.0f * (modulatedPhase * sig_RECIP_TWOPI));
        sample = 2.0f * (fabsf(sample) - 0.5f); // Rectify and scale/offset
        float scaledSample = sample * mul[i] + add[i];

        mainOutput[i] = scaledSample;
        eocOutput[i] = eoc;
        sig_osc_Oscillator_accumulatePhase(&phaseAccumulator,
            frequency[i], sampleRate);
    }

    self->state.phaseAccumulator = phaseAccumulator;
}

void sig_dsp_LFTriangle_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator* self) {
    sig_dsp_Oscillator_destroy(allocator, self);
}


void sig_dsp_WavetableOscillator_init(
    struct sig_dsp_WavetableOscillator* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_WavetableOscillator_generate);

    sig_CONNECT_TO_SILENCE(self, freq, context);
    sig_CONNECT_TO_SILENCE(self, phaseOffset, context);
    sig_CONNECT_TO_UNITY(self, mul, context);
    sig_CONNECT_TO_SILENCE(self, add, context);

    sig_osc_Wavetable_init(&self->state, self->wavetable);
}

struct sig_dsp_WavetableOscillator* sig_dsp_WavetableOscillator_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_WavetableOscillator* self = sig_MALLOC(allocator,
        struct sig_dsp_WavetableOscillator);
    sig_dsp_WavetableOscillator_init(self, context);
    sig_dsp_Oscillator_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_WavetableOscillator_generate(void* signal) {
    struct sig_dsp_WavetableOscillator* self =
        (struct sig_dsp_WavetableOscillator*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float* SIG_RESTRICT frequency = FLOAT_ARRAY(self->inputs.freq);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(self->inputs.phaseOffset);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT eocOutput = FLOAT_ARRAY(self->outputs.eoc);
    float* SIG_RESTRICT mainOutput = FLOAT_ARRAY(self->outputs.main);

    // TODO: This has to be assigned because we don't know when
    // it is been updated by the user.
    self->state.wavetable = self->wavetable;

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    struct sig_osc_Wavetable state = self->state;

    for (size_t i = 0; i < blockSize; i++) {
        float sample = sig_osc_Wavetable_generate(&state,
            frequency[i], phaseOffset[i], sampleRate, &eocOutput[i]);
        float scaledSample = sample * mul[i] + add[i];

        mainOutput[i] = scaledSample;
    }

    self->state = state;
}

void sig_dsp_WavetableOscillator_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_WavetableOscillator* self) {
    sig_dsp_Oscillator_Outputs_destroyAudioBlocks(allocator, &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
    self = NULL;
}


void sig_dsp_WavetableBankOscillator_init(
    struct sig_dsp_WavetableBankOscillator* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_WavetableBankOscillator_generate);

    sig_CONNECT_TO_SILENCE(self, freq, context);
    sig_CONNECT_TO_SILENCE(self, phaseOffset, context);
    sig_CONNECT_TO_UNITY(self, mul, context);
    sig_CONNECT_TO_SILENCE(self, add, context);
    sig_CONNECT_TO_SILENCE(self, tableIndex, context);

    sig_osc_WavetableBank_init(&self->state, self->wavetables);
}

struct sig_dsp_WavetableBankOscillator* sig_dsp_WavetableBankOscillator_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_WavetableBankOscillator* self = sig_MALLOC(allocator,
        struct sig_dsp_WavetableBankOscillator);
    sig_dsp_WavetableBankOscillator_init(self, context);
    sig_dsp_Oscillator_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

extern inline void sig_dsp_WavetableBankOscillator_generate(
    void* signal) {
    struct sig_dsp_WavetableBankOscillator* self =
        (struct sig_dsp_WavetableBankOscillator*) signal;
    self->state.wavetables = self->wavetables;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float* SIG_RESTRICT frequency = FLOAT_ARRAY(self->inputs.freq);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(self->inputs.phaseOffset);
    const float* SIG_RESTRICT tableIndex = FLOAT_ARRAY(self->inputs.tableIndex);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT eocOutput = FLOAT_ARRAY(self->outputs.eoc);
    float* SIG_RESTRICT mainOutput = FLOAT_ARRAY(self->outputs.main);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    struct sig_osc_WavetableBank state = self->state;

    for (size_t i = 0; i < blockSize; i++) {
        float sample = sig_osc_WavetableBank_generate(&state,
            frequency[i], phaseOffset[i], tableIndex[i], sampleRate,
            &eocOutput[i]);
        float scaledSample = sample * mul[i] + add[i];

        mainOutput[i] = scaledSample;
    }

    self->state = state;
}

void sig_dsp_WavetableBankOscillator_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_WavetableBankOscillator* self) {
    sig_dsp_Oscillator_Outputs_destroyAudioBlocks(allocator, &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
    self = NULL;
}

void sig_dsp_AdditiveWavetableBankOscillator_init(
    struct sig_dsp_AdditiveWavetableBankOscillator* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context,
        *sig_dsp_AdditiveWavetableBankOscillator_generate);
    self->wavetables = NULL;
    sig_osc_AdditiveWavetableBank_init(&self->state, self->wavetables);

    sig_CONNECT_TO_SILENCE(self, freq, context);
    sig_CONNECT_TO_SILENCE(self, phaseOffset, context);
    sig_CONNECT_TO_UNITY(self, mul, context);
    sig_CONNECT_TO_SILENCE(self, add, context);
    sig_CONNECT_TO_SILENCE(self, tableIndex, context);
}

struct sig_dsp_AdditiveWavetableBankOscillator*
    sig_dsp_AdditiveWavetableBankOscillator_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context) {
    struct sig_dsp_AdditiveWavetableBankOscillator* self = sig_MALLOC(
        allocator, struct sig_dsp_AdditiveWavetableBankOscillator);
    sig_dsp_AdditiveWavetableBankOscillator_init(self, context);
    sig_dsp_Oscillator_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

extern inline void sig_dsp_AdditiveWavetableBankOscillator_generate(
    void* signal) {
    struct sig_dsp_AdditiveWavetableBankOscillator* self =
        (struct sig_dsp_AdditiveWavetableBankOscillator*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float* SIG_RESTRICT frequency = FLOAT_ARRAY(self->inputs.freq);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(self->inputs.phaseOffset);
    const float* SIG_RESTRICT tableIndex = FLOAT_ARRAY(self->inputs.tableIndex);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT eocOutput = FLOAT_ARRAY(self->outputs.eoc);
    float* SIG_RESTRICT mainOutput = FLOAT_ARRAY(self->outputs.main);

    // The wavetables are assigned after construction,
    // so they have to be read on every block.
    self->state.wavetables = self->wavetables;

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    struct sig_osc_AdditiveWavetableBank state = self->state;

    for (size_t i = 0; i < blockSize; i++) {
        float sample = sig_osc_AdditiveWavetableBank_generate(&state,
            frequency[i], phaseOffset[i], tableIndex[i], sampleRate,
            &eocOutput[i]);
        mainOutput[i] = sample * mul[i] + add[i];
    }

    self->state = state;
}

void sig_dsp_AdditiveWavetableBankOscillator_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_AdditiveWavetableBankOscillator* self) {
    sig_dsp_Oscillator_Outputs_destroyAudioBlocks(allocator, &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}

struct sig_dsp_Phasor* sig_dsp_Phasor_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Phasor* self = sig_MALLOC(allocator, struct sig_dsp_Phasor);
    sig_dsp_Phasor_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Phasor_init(struct sig_dsp_Phasor* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Phasor_generate);
    sig_osc_Oscillator_init(&self->state);

    sig_CONNECT_TO_SILENCE(self, frequency, context);
}

void sig_dsp_Phasor_generate(void* signal) {
    struct sig_dsp_Phasor* self = (struct sig_dsp_Phasor*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float* SIG_RESTRICT frequency = FLOAT_ARRAY(self->inputs.frequency);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    float phase = self->state.phaseAccumulator;

    for (size_t i = 0; i < blockSize; i++) {
        out[i] = phase;
        sig_osc_Oscillator_accumulatePhase(&phase, frequency[i], sampleRate);
    }

    self->state.phaseAccumulator = phase;
}

void sig_dsp_Phasor_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Phasor* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}

struct sig_dsp_RatioPhasor* sig_dsp_RatioPhasor_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context) {
    struct sig_dsp_RatioPhasor* self = sig_MALLOC(allocator,
        struct sig_dsp_RatioPhasor);
    sig_dsp_RatioPhasor_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_RatioPhasor_init(struct sig_dsp_RatioPhasor* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_RatioPhasor_generate);
    sig_osc_RatioPhasor_init(&self->state);
    self->parameters.glide = 0.0f;
    self->previousGlide = 0.0f;
    self->parameters.maxFrequency = self->signal.audioSettings->sampleRate *
        0.5f;
    self->previousMaxFrequency = self->parameters.maxFrequency;

    sig_CONNECT_TO_SILENCE(self, reference, context);
    sig_CONNECT_TO_UNITY(self, numerator, context);
    sig_CONNECT_TO_UNITY(self, denominator, context);
    sig_CONNECT_TO_SILENCE(self, octaves, context);
}

void sig_dsp_RatioPhasor_generate(void* signal) {
    struct sig_dsp_RatioPhasor* self = (struct sig_dsp_RatioPhasor*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float glide = self->parameters.glide;
    const float maxFrequency = self->parameters.maxFrequency;
    const float* SIG_RESTRICT reference = FLOAT_ARRAY(self->inputs.reference);
    const float* SIG_RESTRICT numerator = FLOAT_ARRAY(self->inputs.numerator);
    const float* SIG_RESTRICT denominator = FLOAT_ARRAY(
        self->inputs.denominator);
    const float* SIG_RESTRICT octaves = FLOAT_ARRAY(self->inputs.octaves);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);

    if (glide != self->previousGlide) {
        sig_osc_RatioPhasor_calculateCoefficient(&self->state, glide,
            sampleRate);
        self->previousGlide = glide;
    }

    if (maxFrequency != self->previousMaxFrequency) {
        sig_osc_RatioPhasor_setMaxFrequency(&self->state, maxFrequency,
            sampleRate);
        self->previousMaxFrequency = maxFrequency;
    }

    // The phasor's state is copied into a local, which the compiler can keep
    // in registers, instead of storing and reloading it on every sample.
    struct sig_osc_RatioPhasor state = self->state;

    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_osc_RatioPhasor_generate(&state, reference[i],
            numerator[i], denominator[i], octaves[i]);
    }

    self->state = state;
}

void sig_dsp_RatioPhasor_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_RatioPhasor* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}

struct sig_dsp_WavetableReader* sig_dsp_WavetableReader_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context) {
    struct sig_dsp_WavetableReader* self = sig_MALLOC(allocator,
        struct sig_dsp_WavetableReader);
    sig_dsp_WavetableReader_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_WavetableReader_init(struct sig_dsp_WavetableReader* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_WavetableReader_generate);
    self->wavetable = NULL;

    sig_CONNECT_TO_SILENCE(self, phase, context);
    sig_CONNECT_TO_SILENCE(self, phaseOffset, context);
    sig_CONNECT_TO_UNITY(self, mul, context);
    sig_CONNECT_TO_SILENCE(self, add, context);
}

void sig_dsp_WavetableReader_generate(void* signal) {
    struct sig_dsp_WavetableReader* self =
        (struct sig_dsp_WavetableReader*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    struct sig_Buffer* wavetable = self->wavetable;
    const float* SIG_RESTRICT phase = FLOAT_ARRAY(self->inputs.phase);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(
        self->inputs.phaseOffset);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);

    for (size_t i = 0; i < blockSize; i++) {
        float readPhase = sig_osc_Oscillator_wrapPhase(
            phase[i] + phaseOffset[i]);
        out[i] = sig_Buffer_readLinearAtPhase(wavetable, readPhase) * mul[i] +
            add[i];
    }
}

void sig_dsp_WavetableReader_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_WavetableReader* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}

struct sig_dsp_WavetableBankReader* sig_dsp_WavetableBankReader_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context) {
    struct sig_dsp_WavetableBankReader* self = sig_MALLOC(allocator,
        struct sig_dsp_WavetableBankReader);
    sig_dsp_WavetableBankReader_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_WavetableBankReader_init(struct sig_dsp_WavetableBankReader* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_WavetableBankReader_generate);
    self->wavetables = NULL;

    sig_CONNECT_TO_SILENCE(self, phase, context);
    sig_CONNECT_TO_SILENCE(self, phaseOffset, context);
    sig_CONNECT_TO_SILENCE(self, tableIndex, context);
    sig_CONNECT_TO_UNITY(self, mul, context);
    sig_CONNECT_TO_SILENCE(self, add, context);
}

void sig_dsp_WavetableBankReader_generate(void* signal) {
    struct sig_dsp_WavetableBankReader* self =
        (struct sig_dsp_WavetableBankReader*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    struct sig_WavetableBank* wavetables = self->wavetables;
    const float numTables = (float) wavetables->length;
    const float* SIG_RESTRICT phase = FLOAT_ARRAY(self->inputs.phase);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(
        self->inputs.phaseOffset);
    const float* SIG_RESTRICT tableIndex = FLOAT_ARRAY(
        self->inputs.tableIndex);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);

    for (size_t i = 0; i < blockSize; i++) {
        float readPhase = sig_osc_Oscillator_wrapPhase(
            phase[i] + phaseOffset[i]);
        float sample = sig_WavetableBank_readLinearAtPhase(wavetables,
            tableIndex[i] * numTables, readPhase);
        out[i] = sample * mul[i] + add[i];
    }
}

void sig_dsp_WavetableBankReader_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_WavetableBankReader* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}

struct sig_dsp_AdditiveWavetableBankReader*
    sig_dsp_AdditiveWavetableBankReader_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_AdditiveWavetableBankReader* self = sig_MALLOC(allocator,
        struct sig_dsp_AdditiveWavetableBankReader);
    sig_dsp_AdditiveWavetableBankReader_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_AdditiveWavetableBankReader_init(
    struct sig_dsp_AdditiveWavetableBankReader* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context,
        *sig_dsp_AdditiveWavetableBankReader_generate);
    self->wavetables = NULL;
    sig_osc_AdditiveWavetableBank_init(&self->state, self->wavetables);
    self->parameters.peakRelease = 0.0f;
    sig_osc_PeakFrequency_init(&self->peakFrequency);
    self->previousPeakRelease = 0.0f;
    self->levelFrequency = 0.0f;

    sig_CONNECT_TO_SILENCE(self, phase, context);
    sig_CONNECT_TO_SILENCE(self, phaseOffset, context);
    sig_CONNECT_TO_SILENCE(self, frequency, context);
    sig_CONNECT_TO_SILENCE(self, tableIndex, context);
    sig_CONNECT_TO_UNITY(self, mul, context);
    sig_CONNECT_TO_SILENCE(self, add, context);
}

// Generates a block, selecting the octave from the peak rate at which the
// reader reads.
static inline void sig_dsp_AdditiveWavetableBankReader_generateFromPeak(
    struct sig_dsp_AdditiveWavetableBankReader* self,
    struct sig_osc_AdditiveWavetableBank* state) {
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float peakRelease = self->parameters.peakRelease;
    const float* SIG_RESTRICT phase = FLOAT_ARRAY(self->inputs.phase);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(
        self->inputs.phaseOffset);
    const float* SIG_RESTRICT tableIndex = FLOAT_ARRAY(
        self->inputs.tableIndex);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);

    if (peakRelease != self->previousPeakRelease) {
        sig_osc_PeakFrequency_calculateCoefficient(&self->peakFrequency,
            peakRelease, sampleRate);
        self->previousPeakRelease = peakRelease;
    }

    struct sig_osc_PeakFrequency peakFrequency = self->peakFrequency;
    float levelFrequency = self->levelFrequency;

    for (size_t i = 0; i < blockSize; i++) {
        float readPhase = sig_osc_Oscillator_wrapPhase(
            phase[i] + phaseOffset[i]);
        float peak = sig_osc_PeakFrequency_generate(&peakFrequency,
            readPhase, sampleRate);
        if (peak > levelFrequency * 1.01f || peak < levelFrequency * 0.99f) {
            levelFrequency = peak;
        }

        float sample = sig_osc_AdditiveWavetableBank_readAtPhase(state,
            levelFrequency, readPhase, tableIndex[i]);
        out[i] = sample * mul[i] + add[i];
    }

    self->peakFrequency = peakFrequency;
    self->levelFrequency = levelFrequency;
}

void sig_dsp_AdditiveWavetableBankReader_generate(void* signal) {
    struct sig_dsp_AdditiveWavetableBankReader* self =
        (struct sig_dsp_AdditiveWavetableBankReader*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float peakRelease = self->parameters.peakRelease;
    const float* SIG_RESTRICT phase = FLOAT_ARRAY(self->inputs.phase);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(
        self->inputs.phaseOffset);
    const float* SIG_RESTRICT frequency = FLOAT_ARRAY(self->inputs.frequency);
    const float* SIG_RESTRICT tableIndex = FLOAT_ARRAY(
        self->inputs.tableIndex);
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);

    // The reader's state is copied into a local, which the compiler can keep
    // in registers, instead of storing and reloading it on every sample.
    // The wavetables are assigned after construction,
    // so they have to be read on every block.
    struct sig_osc_AdditiveWavetableBank state = self->state;
    state.wavetables = self->wavetables;

    if (peakRelease > 0.0f) {
        sig_dsp_AdditiveWavetableBankReader_generateFromPeak(self, &state);
    } else {
        for (size_t i = 0; i < blockSize; i++) {
            float readPhase = sig_osc_Oscillator_wrapPhase(
                phase[i] + phaseOffset[i]);
            float sample = sig_osc_AdditiveWavetableBank_readAtPhase(&state,
                frequency[i], readPhase, tableIndex[i]);
            out[i] = sample * mul[i] + add[i];
        }
    }

    self->state = state;
}

void sig_dsp_AdditiveWavetableBankReader_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_AdditiveWavetableBankReader* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_Smooth_init(struct sig_dsp_Smooth* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Smooth_generate);
    self->previousTime = -1.0f;
    self->previousSample = 0.0f;
    self->parameters.time = 0.01f;
    self->a1 = 0.0f;
    sig_CONNECT_TO_SILENCE(self, source, context);
}

struct sig_dsp_Smooth* sig_dsp_Smooth_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Smooth* self = sig_MALLOC(allocator,
        struct sig_dsp_Smooth);
    sig_dsp_Smooth_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

// TODO: Unit tests
void sig_dsp_Smooth_generate(void* signal) {
    struct sig_dsp_Smooth* self = (struct sig_dsp_Smooth*) signal;

    float previousSample = self->previousSample;
    float a1 = self->a1;
    float time = self->parameters.time;

    if (self->previousTime != time) {
        self->a1 = a1 = sig_filter_smooth_calculateCoefficient(time,
            self->signal.audioSettings->sampleRate);
        self->previousTime = time;
    }

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = previousSample =
            sig_filter_smooth(
                in[i], previousSample, a1);
    }

    self->previousSample = previousSample;
}

void sig_dsp_Smooth_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Smooth* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


void sig_dsp_DCBlock_init(struct sig_dsp_DCBlock* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_DCBlock_generate);
    self->previousFrequency = 0.0f;
    self->parameters.frequency = 2.0f;
    sig_filter_DCBlock_init(&self->state, self->parameters.frequency,
        context->audioSettings->sampleRate);

    sig_CONNECT_TO_SILENCE(self, source, context);
}

struct sig_dsp_DCBlock* sig_dsp_DCBlock_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_DCBlock* self = sig_MALLOC(allocator,
        struct sig_dsp_DCBlock);
    sig_dsp_DCBlock_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_DCBlock_generate(void* signal) {
    struct sig_dsp_DCBlock* self = (struct sig_dsp_DCBlock*) signal;

    if (self->parameters.frequency != self->previousFrequency) {
        self->state.r = sig_filter_dcBlock_calculateCoefficient(
            self->parameters.frequency,
            self->signal.audioSettings->sampleRate);
        self->previousFrequency = self->parameters.frequency;
    }

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    // The filter's state is copied into a local, which the compiler can keep
    // in registers, instead of storing and reloading it on every sample.
    struct sig_filter_DCBlock state = self->state;

    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_filter_DCBlock_generate(&state, in[i]);
    }

    self->state = state;
}

void sig_dsp_DCBlock_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_DCBlock* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


void sig_dsp_OnePole_init(struct sig_dsp_OnePole* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_OnePole_generate);
    self->parameters.mode = sig_dsp_OnePole_Mode_LOW_PASS;
    self->a1 = 0.0f;
    self->b0 = 1.0f;
    self->previousFrequency = 0.0f;
    self->previousMode = sig_dsp_OnePole_Mode_NOT_SPECIFIED;
    self->previousSample = 0.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, frequency, context);
}

struct sig_dsp_OnePole* sig_dsp_OnePole_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_OnePole* self = sig_MALLOC(allocator,
        struct sig_dsp_OnePole);
    sig_dsp_OnePole_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

extern inline void sig_dsp_OnePole_recalculateCoefficients(
    struct sig_dsp_OnePole* self, float frequency) {
    float sampleRate = self->signal.audioSettings->sampleRate;
    enum sig_dsp_OnePole_Mode mode = self->parameters.mode;

    self->previousFrequency = frequency;
    self->previousMode = mode;

    if (mode == sig_dsp_OnePole_Mode_HIGH_PASS) {
        self->a1 = sig_filter_onepole_HPF_calculateA1(frequency, sampleRate);
        self->b0 = sig_filter_onepole_HPF_calculateB0(self->a1);
    } else {
        self->a1 = sig_filter_onepole_LPF_calculateA1(frequency, sampleRate);
        self->b0 = sig_filter_onepole_LPF_calculateB0(self->a1);
    }
}

void sig_dsp_OnePole_generate(void* signal) {
    struct sig_dsp_OnePole* self = (struct sig_dsp_OnePole*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT freqIn = FLOAT_ARRAY(self->inputs.frequency);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    float previousSample = self->previousSample;
    float previousFrequency = self->previousFrequency;
    bool hasModeChanged = self->previousMode != self->parameters.mode;
    float b0 = self->b0;
    float a1 = self->a1;

    for (size_t i = 0; i < blockSize; i++) {
        float frequency = freqIn[i];
        if (previousFrequency != frequency || hasModeChanged) {
            sig_dsp_OnePole_recalculateCoefficients(self, frequency);
            previousFrequency = frequency;
            hasModeChanged = false;
            b0 = self->b0;
            a1 = self->a1;
        }

        float sample = sig_filter_onepole(in[i], previousSample, b0, a1);
        out[i] = sample;
        previousSample = sample;
    }

    self->previousSample = previousSample;
}

void sig_dsp_OnePole_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_OnePole* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


void sig_dsp_EMA_init(struct sig_dsp_EMA* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_EMA_generate);
    self->parameters.alpha = 0.1f;
    self->previousSample = 0.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
}

struct sig_dsp_EMA* sig_dsp_EMA_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_EMA* self = sig_MALLOC(allocator,
        struct sig_dsp_EMA);
    sig_dsp_EMA_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;

}
void sig_dsp_EMA_generate(void* signal) {
    struct sig_dsp_EMA* self = (struct sig_dsp_EMA*) signal;
    float previousSample = self->previousSample;
    const float alpha = self->parameters.alpha;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    for (size_t i = 0; i < blockSize; i++) {
        float sample = sig_filter_ema(in[i], previousSample, alpha);
        out[i] = sample;
        previousSample = sample;
    }

    self->previousSample = previousSample;
}

void sig_dsp_EMA_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_EMA* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}


void sig_dsp_Tanh_init(struct sig_dsp_Tanh* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Tanh_generate);

    sig_CONNECT_TO_SILENCE(self, source, context);
}

struct sig_dsp_Tanh* sig_dsp_Tanh_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Tanh* self = sig_MALLOC(allocator, struct sig_dsp_Tanh);
    sig_dsp_Tanh_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

// TODO: Unit tests.
void sig_dsp_Tanh_generate(void* signal) {
    struct sig_dsp_Tanh* self = (struct sig_dsp_Tanh*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = tanhf(in[i]);
    }
}

void sig_dsp_Tanh_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Tanh* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_SoftClip_init(struct sig_dsp_SoftClip* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_SoftClip_generate);

    self->parameters.threshold = 0.8f;

    sig_CONNECT_TO_SILENCE(self, source, context);
}

struct sig_dsp_SoftClip* sig_dsp_SoftClip_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_SoftClip* self = sig_MALLOC(allocator,
        struct sig_dsp_SoftClip);
    sig_dsp_SoftClip_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_SoftClip_generate(void* signal) {
    struct sig_dsp_SoftClip* self = (struct sig_dsp_SoftClip*) signal;
    const float threshold = self->parameters.threshold;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);

    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_softClip(in[i], threshold);
    }
}

void sig_dsp_SoftClip_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SoftClip* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_Dust_init(struct sig_dsp_Dust* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Dust_generate);

    struct sig_dsp_Dust_Parameters parameters = {
        .bipolar = 0.0
    };

    self->parameters = parameters;
    self->sampleDuration = 1.0 / context->audioSettings->sampleRate;
    self->previousDensity = 0.0;
    self->threshold = 0.0;

    sig_CONNECT_TO_SILENCE(self, density, context);
}

struct sig_dsp_Dust* sig_dsp_Dust_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {

    struct sig_dsp_Dust* self = sig_MALLOC(allocator, struct sig_dsp_Dust);
    sig_dsp_Dust_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Dust_generate(void* signal) {
    struct sig_dsp_Dust* self = (struct sig_dsp_Dust*) signal;

    const float scaleDiv = self->parameters.bipolar > 0.0f ? 2.0f : 1.0f;
    const float scaleSub = self->parameters.bipolar > 0.0f ? 1.0f : 0.0f;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT densityIn = FLOAT_ARRAY(self->inputs.density);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    float previousDensity = self->previousDensity;
    float threshold = self->threshold;
    float scale = self->scale;

    for (size_t i = 0; i < blockSize; i++) {
        float density = densityIn[i];

        if (density != previousDensity) {
            previousDensity = density;
            threshold = density * self->sampleDuration;
            scale = threshold > 0.0f ?
                scaleDiv / threshold : 0.0f;
        }

        float rand = sig_randf();
        float val = rand < threshold ?
            rand * scale - scaleSub : 0.0f;
        out[i] = val;
    }

    self->previousDensity = previousDensity;
    self->threshold = threshold;
    self->scale = scale;
}

void sig_dsp_Dust_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Dust* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_TimedGate_init(struct sig_dsp_TimedGate* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_TimedGate_generate);

    struct sig_dsp_TimedGate_Parameters parameters = {
        .resetOnTrigger = 0.0,
        .bipolar = 0.0
    };

    self->parameters = parameters;
    self->previousTrigger = 0.0f;
    self->previousDuration = 0.0f;
    self->gateValue = 0.0f;
    self->durationSamps = 0;
    self->samplesRemaining = 0;

    sig_CONNECT_TO_SILENCE(self, trigger, context);
    sig_CONNECT_TO_SILENCE(self, duration, context);
}

struct sig_dsp_TimedGate* sig_dsp_TimedGate_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_TimedGate* self = sig_MALLOC(allocator,
        struct sig_dsp_TimedGate);
    sig_dsp_TimedGate_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

static inline void sig_dsp_TimedGate_outputHigh(
    struct sig_dsp_TimedGate* self, float* SIG_RESTRICT out,
    size_t index) {
    out[index] = self->gateValue;
    self->samplesRemaining--;
}

static inline void sig_dsp_TimedGate_outputLow(
    struct sig_dsp_TimedGate* self, float* SIG_RESTRICT out,
    size_t index) {
    out[index] = 0.0f;
}

void sig_dsp_TimedGate_generate(void* signal) {
    struct sig_dsp_TimedGate* self = (struct sig_dsp_TimedGate*)
        signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT triggerIn = FLOAT_ARRAY(self->inputs.trigger);
    const float* SIG_RESTRICT durationIn = FLOAT_ARRAY(self->inputs.duration);

    for (size_t i = 0; i < blockSize; i++) {
        float currentTrigger = triggerIn[i];
        float duration = durationIn[i];

        if ((currentTrigger > 0.0f && self->previousTrigger <= 0.0f) ||
            (self->parameters.bipolar > 0.0 && currentTrigger < 0.0f && self->previousTrigger >= 0.0f)) {
            // A new trigger was received.
            self->gateValue = currentTrigger;

            if (duration != self->previousDuration) {
                // The duration input has changed.
                self->durationSamps = lroundf(duration *
                    sampleRate);
                self->previousDuration = duration;
            }

            if (self->parameters.resetOnTrigger > 0.0f &&
                self->samplesRemaining > 0) {
                // Gate is open and needs to be reset.
                // Close the gate for one sample,
                // and don't count down the duration
                // until next time.
                sig_dsp_TimedGate_outputLow(self, out, i);
                self->samplesRemaining = self->durationSamps;
            } else {
                self->samplesRemaining = self->durationSamps;
                sig_dsp_TimedGate_outputHigh(self, out, i);
            }
        } else if (self->samplesRemaining > 0) {
            sig_dsp_TimedGate_outputHigh(self, out, i);
        } else {
            sig_dsp_TimedGate_outputLow(self, out, i);
        }

        self->previousTrigger = currentTrigger;
    }
}

void sig_dsp_TimedGate_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_TimedGate* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_DustGate* sig_dsp_DustGate_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_DustGate* self = sig_MALLOC(allocator,
        struct sig_dsp_DustGate);

    self->reciprocalDensity = sig_dsp_Div_new(allocator, context);
    self->densityDurationMultiplier = sig_dsp_Mul_new(allocator, context);
    self->dust = sig_dsp_Dust_new(allocator, context);
    self->gate = sig_dsp_TimedGate_new(allocator, context);
    sig_dsp_DustGate_init(self, context);

    return self;
}


void sig_dsp_DustGate_init(struct sig_dsp_DustGate* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_DustGate_generate);
    self->reciprocalDensity->inputs.left = context->unity->outputs.main;
    self->densityDurationMultiplier->inputs.left =
        self->reciprocalDensity->outputs.main;
    self->gate->inputs.trigger = self->dust->outputs.main;
    self->gate->inputs.duration =
        self->densityDurationMultiplier->outputs.main;
    self->outputs.main = self->gate->outputs.main;

    sig_CONNECT_TO_SILENCE(self, density, context);
    sig_CONNECT_TO_SILENCE(self, durationPercentage, context);
}

void sig_dsp_DustGate_generate(void* signal) {
    struct sig_dsp_DustGate* self = (struct sig_dsp_DustGate*) signal;

    // Bind all parameters (remove this when we have change events).
    self->dust->inputs.density = self->inputs.density;
    self->reciprocalDensity->inputs.right = self->inputs.density;
    self->densityDurationMultiplier->inputs.right =
        self->inputs.durationPercentage;
    self->dust->parameters.bipolar = self->parameters.bipolar;
    self->gate->parameters.bipolar = self->parameters.bipolar;

    // Evaluate all signals.
    self->reciprocalDensity->signal.generate(self->reciprocalDensity);
    self->densityDurationMultiplier->signal.generate(
        self->densityDurationMultiplier);
    self->dust->signal.generate(self->dust);
    self->gate->signal.generate(self->gate);
}


void sig_dsp_DustGate_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_DustGate* self) {
    sig_dsp_TimedGate_destroy(allocator, self->gate);

    sig_dsp_Mul_destroy(allocator, self->densityDurationMultiplier);
    sig_AudioBlock_destroy(allocator, self->reciprocalDensity->inputs.left);
    sig_dsp_Div_destroy(allocator, self->reciprocalDensity);

    sig_dsp_Dust_destroy(allocator, self->dust);

    // We don't call sig_dsp_Signal_destroy
    // because our output is borrowed from self->gate,
    // and it was already freed in TimeGate's destructor.
    allocator->impl->free(allocator, self);
}


void sig_dsp_ClockSource_init(struct sig_dsp_ClockSource* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_ClockSource_generate);

    struct sig_dsp_Clock_Parameters params = {
        .threshold = 0.04f, // 0.2V, assuming 10Vpp
    };
    self->parameters = params;

    self->previousTapValue = 0.0f;
    self->tapCount = 0;
    self->tempoDurationSamples = 0;
    self->samplesSinceLastTap = 0;
    self->isLatching = false;
    self->latchedTapSamplesRemaining = 0;

    sig_CONNECT_TO_SILENCE(self, pulse, context);
    sig_CONNECT_TO_SILENCE(self, tap, context);
}

struct sig_dsp_ClockSource* sig_dsp_ClockSource_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_ClockSource* self = sig_MALLOC(allocator,
        struct sig_dsp_ClockSource);
    sig_dsp_ClockSource_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

static inline void updateTempo(struct sig_dsp_ClockSource* self,
    uint32_t tempoDuration) {
    self->tempoDurationSamples = tempoDuration;
    self->numHighSamples = (uint32_t) (
        (float) self->tempoDurationSamples * 0.25f);
}

static inline void handleTap(struct sig_dsp_ClockSource* self) {
    self->tapCount++;
    if (self->tapCount == 2) {
        // Two taps is enough to output a tempo clock.
        updateTempo(self, self->samplesSinceLastTap);
        self->latchedTapSamplesRemaining = self->tempoDurationSamples;
        self->isLatching = true;
    } else if (self->tapCount > 2) {
        // The second duration should be averaged with the first,
        // assuming it's within 50%.
        uint32_t shortestDuration = (uint32_t) (
            (float) self->tempoDurationSamples / 1.5f);
        uint32_t longestDuration = (uint32_t) (
            (float) self->tempoDurationSamples * 1.5f);

        if (self->samplesSinceLastTap >= shortestDuration &&
            self->samplesSinceLastTap <= longestDuration) {
            float prevTempoDuration = self->tempoDurationSamples;
            uint32_t averagedTempoDuration = (uint32_t) (
                (float) (prevTempoDuration + self->samplesSinceLastTap) / 2.0f);
            updateTempo(self, averagedTempoDuration);

            // Recalculate the tap duration.
            if (self->tempoDurationSamples < prevTempoDuration) {
                if (self->latchedTapSamplesRemaining >
                    self->tempoDurationSamples) {
                    // The current duration is shorter than
                    // the previous time remaining.
                    self->latchedTapSamplesRemaining = self->tempoDurationSamples;
                }
            } else if (self->tempoDurationSamples > prevTempoDuration) {
                // Add the additional time to the duration remaining.
                self->latchedTapSamplesRemaining += (
                    self->tempoDurationSamples - prevTempoDuration);
            }

            self->tapCount = 0;
        } else {
            self->tapCount = 1;
        }
    }

    self->samplesSinceLastTap = 0;
}

void sig_dsp_ClockSource_generate(void* signal) {
    struct sig_dsp_ClockSource* self = (struct sig_dsp_ClockSource*) signal;
    const float threshold = self->parameters.threshold;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT pulse = FLOAT_ARRAY(self->inputs.pulse);
    const float* SIG_RESTRICT tap = FLOAT_ARRAY(self->inputs.tap);

    for (size_t i = 0; i < blockSize; i++) {
        float pulseValue = pulse[i];
        float tapValue = tap[i];

        float sample = 0.0f;
        if (pulseValue >= threshold) {
            self->isLatching = false;
            self->tapCount = 0;
            sample = 1.0f;
        } else if (pulseValue < threshold) {
            if (self->tapCount > 0) {
                self->samplesSinceLastTap++;
            }

            if (tapValue >= threshold && self->previousTapValue < threshold) {
                handleTap(self);
            }

            if (self->isLatching) {
                if (self->latchedTapSamplesRemaining <= self->numHighSamples) {
                    sample = 1.0f;
                }

                self->latchedTapSamplesRemaining--;
                if (self->latchedTapSamplesRemaining <= 0) {
                    self->latchedTapSamplesRemaining =
                        self->tempoDurationSamples;
                }
            }
        }

        out[i] = sample;
        self->previousTapValue = tapValue;
    }
}

void sig_dsp_ClockSource_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ClockSource* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
    self = NULL;
}


void sig_dsp_ClockDetector_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_ClockDetector_Outputs* outputs) {
    outputs->main = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->bpm = sig_AudioBlock_newSilent(allocator, audioSettings);
}

void sig_dsp_ClockDetector_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_ClockDetector_Outputs* outputs) {

    sig_AudioBlock_destroy(allocator, outputs->main);
    sig_AudioBlock_destroy(allocator, outputs->bpm);
}

void sig_dsp_ClockDetector_init(struct sig_dsp_ClockDetector* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_ClockDetector_generate);

    struct sig_dsp_Clock_Parameters params = {
        .threshold = 0.04f // 0.2V, assuming 10Vpp
    };

    self->parameters = params;
    self->previousTrigger = 0.0f;
    self->isRisingEdge = false;
    self->numPulsesDetected = 0;
    self->samplesSinceLastPulse = (uint32_t)
        self->signal.audioSettings->sampleRate;
    self->clockFreq = 0.0f;
    self->pulseDurSamples = 0;

    sig_CONNECT_TO_SILENCE(self, source, context);
}

struct sig_dsp_ClockDetector* sig_dsp_ClockDetector_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context) {
    struct sig_dsp_ClockDetector* self = sig_MALLOC(allocator,
        struct sig_dsp_ClockDetector);
    sig_dsp_ClockDetector_init(self, context);
    sig_dsp_ClockDetector_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

static inline float sig_dsp_ClockDetector_calcClockFreq(
    float sampleRate, uint32_t samplesSinceLastPulse,
    float prevFreq) {
    float freq = sampleRate / (float) samplesSinceLastPulse;

    return freq;
}

void sig_dsp_ClockDetector_generate(void* signal) {
    struct sig_dsp_ClockDetector* self =
        (struct sig_dsp_ClockDetector*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float threshold = self->parameters.threshold;
    float* SIG_RESTRICT mainOut = FLOAT_ARRAY(self->outputs.main);
    float* SIG_RESTRICT bpmOut = FLOAT_ARRAY(self->outputs.bpm);
    const float* SIG_RESTRICT source = FLOAT_ARRAY(self->inputs.source);

    float previousTrigger = self->previousTrigger;
    float clockFreq = self->clockFreq;
    bool isRisingEdge = self->isRisingEdge;
    uint32_t numPulsesDetected = self->numPulsesDetected;
    uint32_t samplesSinceLastPulse = self->samplesSinceLastPulse;
    uint32_t pulseDurSamples = self->pulseDurSamples;

    for (size_t i = 0; i < blockSize; i++) {
        samplesSinceLastPulse++;

        float sourceSamp = source[i];
        if (sourceSamp > 0.0f && previousTrigger < threshold) {
            // Start of rising edge.
            isRisingEdge = true;
        } else if (sourceSamp < previousTrigger) {
            // Failed to reach the threshold before
            // the signal fell again.
            isRisingEdge = false;
        }

        if (isRisingEdge && sourceSamp >= threshold) {
            // Signal is rising and threshold has been reached,
            // so this is a pulse.
            numPulsesDetected++;
            if (numPulsesDetected > 1) {
                // Need to have found at least two pulses before
                // the frequency can be reliably determined.
                clockFreq = sig_dsp_ClockDetector_calcClockFreq(
                    sampleRate, samplesSinceLastPulse, clockFreq);
                numPulsesDetected = 1;
            }
            pulseDurSamples = samplesSinceLastPulse;
            samplesSinceLastPulse = 0;
            isRisingEdge = false;
        }

        mainOut[i] = clockFreq;
        bpmOut[i] = clockFreq * 60.0f;

        previousTrigger = sourceSamp;
    }

    self->previousTrigger = previousTrigger;
    self->clockFreq = clockFreq;
    self->isRisingEdge = isRisingEdge;
    self->numPulsesDetected = numPulsesDetected;
    self->samplesSinceLastPulse = samplesSinceLastPulse;
    self->pulseDurSamples = pulseDurSamples;
}

void sig_dsp_ClockDetector_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ClockDetector* self) {
    sig_dsp_ClockDetector_Outputs_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}

struct sig_dsp_ClockPhasor* sig_dsp_ClockPhasor_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context) {
    struct sig_dsp_ClockPhasor* self = sig_MALLOC(allocator,
        struct sig_dsp_ClockPhasor);
    sig_dsp_ClockPhasor_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_ClockPhasor_init(struct sig_dsp_ClockPhasor* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_ClockPhasor_generate);

    struct sig_dsp_Clock_Parameters params = {
        .threshold = 0.04f // 0.2V, assuming 10Vpp
    };
    self->parameters = params;
    sig_osc_ClockPhasor_init(&self->state);

    sig_CONNECT_TO_SILENCE(self, pulse, context);
}

void sig_dsp_ClockPhasor_generate(void* signal) {
    struct sig_dsp_ClockPhasor* self = (struct sig_dsp_ClockPhasor*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float threshold = self->parameters.threshold;
    const float* SIG_RESTRICT pulse = FLOAT_ARRAY(self->inputs.pulse);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    struct sig_osc_ClockPhasor state = self->state;

    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_osc_ClockPhasor_generate(&state, pulse[i], threshold);
    }

    self->state = state;
}

void sig_dsp_ClockPhasor_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ClockPhasor* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}

// The onset detector's target window length, in seconds.
#define SIG_TEMPO_WINDOW_DURATION 0.0464f
// The target number of envelope values per second.
#define SIG_TEMPO_ENVELOPE_RATE 86.13f
// The number of hops between a frame and the frame that the onset
// detector compares it with.
#define SIG_TEMPO_DIFF_LAG_HOPS 2
// The value at or above which a rise of the sync input taps a beat.
#define SIG_TEMPO_SYNC_THRESHOLD 0.5f

size_t sig_dsp_TempoDetector_calcWindowLength(float sampleRate) {
    float target = SIG_TEMPO_WINDOW_DURATION * sampleRate;
    size_t length = 1;
    while ((float) (length * 2) <= target) {
        length *= 2;
    }

    return target - (float) length <= (float) (length * 2) - target ?
        length : length * 2;
}

size_t sig_dsp_TempoDetector_calcHopLength(float sampleRate) {
    return (size_t) roundf(sampleRate / SIG_TEMPO_ENVELOPE_RATE);
}

void sig_dsp_TempoDetector_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_TempoDetector_Outputs* outputs) {
    outputs->clock = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->phase = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->bpm = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->confidence = sig_AudioBlock_newSilent(allocator,
        audioSettings);
}

void sig_dsp_TempoDetector_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_TempoDetector_Outputs* outputs) {
    sig_AudioBlock_destroy(allocator, outputs->clock);
    sig_AudioBlock_destroy(allocator, outputs->phase);
    sig_AudioBlock_destroy(allocator, outputs->bpm);
    sig_AudioBlock_destroy(allocator, outputs->confidence);
}

void sig_dsp_TempoDetector_init(struct sig_dsp_TempoDetector* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_TempoDetector_generate);

    struct sig_dsp_TempoDetector_Parameters parameters = {
        .threshold = -70.0f,
        .minTempo = sig_dsp_TempoDetector_LOWEST_TEMPO,
        .maxTempo = sig_dsp_TempoDetector_HIGHEST_TEMPO,
        .responsiveness = 0.6f,
        .lead = 0.0f,
        .triggerDuration = 0.01f,
        .silenceBehaviour = 0.0f,
        .silenceTimeout = 4.0f
    };
    self->parameters = parameters;
    self->tempoEstimate = 0.0f;
    self->previousSync = 0.0f;
    sig_tempo_ClockGenerator_init(&self->clockGenerator,
        context->audioSettings->sampleRate);

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, sync, context);
}

struct sig_dsp_TempoDetector* sig_dsp_TempoDetector_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context) {
    float sampleRate = context->audioSettings->sampleRate;
    struct sig_dsp_TempoDetector* self = sig_MALLOC(allocator,
        struct sig_dsp_TempoDetector);

    self->windowLength = sig_dsp_TempoDetector_calcWindowLength(sampleRate);
    self->hopLength = sig_dsp_TempoDetector_calcHopLength(sampleRate);
    self->latency = (float) (self->windowLength - self->hopLength) / 2.0f;
    self->onsetDetector = sig_tempo_OnsetDetector_new(allocator, sampleRate,
        self->windowLength, self->hopLength, SIG_TEMPO_DIFF_LAG_HOPS,
        -70.0f);
    self->tempoEstimator = sig_tempo_TempoEstimator_new(allocator,
        sampleRate / (float) self->hopLength,
        sig_dsp_TempoDetector_LOWEST_TEMPO,
        sig_dsp_TempoDetector_HIGHEST_TEMPO);
    self->beatTracker = sig_tempo_BeatTracker_new(allocator, sampleRate,
        self->hopLength, self->latency, sig_dsp_TempoDetector_LOWEST_TEMPO,
        sig_dsp_TempoDetector_HIGHEST_TEMPO);
    sig_dsp_TempoDetector_init(self, context);
    sig_dsp_TempoDetector_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

// Passes the detector's parameters to its stages.
static void sig_dsp_TempoDetector_applyParameters(
    struct sig_dsp_TempoDetector* self) {
    const struct sig_dsp_TempoDetector_Parameters parameters =
        self->parameters;

    sig_tempo_OnsetDetector_setThreshold(self->onsetDetector,
        sig_clamp(parameters.threshold, -90.0f, -30.0f));
    sig_tempo_TempoEstimator_setTempoRange(self->tempoEstimator,
        parameters.minTempo, parameters.maxTempo);
    sig_tempo_BeatTracker_setTempoRange(self->beatTracker,
        parameters.minTempo, parameters.maxTempo);
    sig_tempo_BeatTracker_setResponsiveness(self->beatTracker,
        parameters.responsiveness);
    sig_tempo_BeatTracker_setSilenceTimeout(self->beatTracker,
        parameters.silenceTimeout);
    sig_tempo_ClockGenerator_setLead(&self->clockGenerator,
        parameters.lead);
    sig_tempo_ClockGenerator_setTriggerDuration(&self->clockGenerator,
        parameters.triggerDuration);
    sig_tempo_ClockGenerator_setSilenceBehaviour(&self->clockGenerator,
        parameters.silenceBehaviour);
}

void sig_dsp_TempoDetector_generate(void* signal) {
    struct sig_dsp_TempoDetector* self =
        (struct sig_dsp_TempoDetector*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float* SIG_RESTRICT source = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT sync = FLOAT_ARRAY(self->inputs.sync);
    float* SIG_RESTRICT clockOut = FLOAT_ARRAY(self->outputs.clock);
    float* SIG_RESTRICT phaseOut = FLOAT_ARRAY(self->outputs.phase);
    float* SIG_RESTRICT bpmOut = FLOAT_ARRAY(self->outputs.bpm);
    float* SIG_RESTRICT confidenceOut = FLOAT_ARRAY(self->outputs.confidence);
    struct sig_tempo_OnsetDetector* onsetDetector = self->onsetDetector;
    struct sig_tempo_BeatTracker* beatTracker = self->beatTracker;
    struct sig_tempo_ClockGenerator_Outputs outputs;
    float previousSync = self->previousSync;

    sig_dsp_TempoDetector_applyParameters(self);

    for (size_t i = 0; i < blockSize; i++) {
        float value = sig_tempo_OnsetDetector_generate(onsetDetector,
            source[i]);
        float phase = sig_tempo_BeatTracker_follow(beatTracker);
        if (sync[i] >= SIG_TEMPO_SYNC_THRESHOLD &&
            previousSync < SIG_TEMPO_SYNC_THRESHOLD) {
            sig_tempo_BeatTracker_tap(beatTracker);
        }
        previousSync = sync[i];

        // The follower has now advanced (i + 1) * hopLength samples for
        // envelope value i, as the beat tracker requires.
        if (sig_tempo_OnsetDetector_hasNewValue(onsetDetector)) {
            self->tempoEstimate = sig_tempo_TempoEstimator_generate(
                self->tempoEstimator, value);
            sig_tempo_BeatTracker_analyse(beatTracker, value,
                self->tempoEstimate);
        }

        sig_tempo_ClockGenerator_generate(&self->clockGenerator, phase,
            sig_tempo_BeatTracker_tempo(beatTracker),
            sig_tempo_BeatTracker_confidence(beatTracker),
            sig_tempo_BeatTracker_state(beatTracker), &outputs);
        clockOut[i] = outputs.clock;
        phaseOut[i] = outputs.phase;
        bpmOut[i] = outputs.bpm;
        confidenceOut[i] = outputs.confidence;
    }

    self->previousSync = previousSync;
}

void sig_dsp_TempoDetector_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_TempoDetector* self) {
    sig_dsp_TempoDetector_Outputs_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_tempo_OnsetDetector_destroy(allocator, self->onsetDetector);
    sig_tempo_TempoEstimator_destroy(allocator, self->tempoEstimator);
    sig_tempo_BeatTracker_destroy(allocator, self->beatTracker);
    sig_dsp_Signal_destroy(allocator, self);
}



struct sig_dsp_LinearToFreq* sig_dsp_LinearToFreq_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_LinearToFreq* self = sig_MALLOC(allocator,
        struct sig_dsp_LinearToFreq);
    sig_dsp_LinearToFreq_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_LinearToFreq_init(struct sig_dsp_LinearToFreq* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_LinearToFreq_generate);
    self->parameters.middleFreq = sig_FREQ_C4;
    sig_CONNECT_TO_SILENCE(self, source, context);
}

void sig_dsp_LinearToFreq_generate(void* signal) {
    struct sig_dsp_LinearToFreq* self = (struct sig_dsp_LinearToFreq*) signal;
    const float middleFreq = self->parameters.middleFreq;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);

    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_linearToFreq(in[i], middleFreq);
    }
}

void sig_dsp_LinearToFreq_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_LinearToFreq* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_Branch* sig_dsp_Branch_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Branch* self = sig_MALLOC(allocator, struct sig_dsp_Branch);
    sig_dsp_Branch_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Branch_init(struct sig_dsp_Branch* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Branch_generate);
    sig_CONNECT_TO_SILENCE(self, off, context);
    sig_CONNECT_TO_SILENCE(self, on, context);
    sig_CONNECT_TO_SILENCE(self, condition, context);
}

void sig_dsp_Branch_generate(void* signal) {
    struct sig_dsp_Branch* self = (struct sig_dsp_Branch*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT condition = FLOAT_ARRAY(self->inputs.condition);
    const float* SIG_RESTRICT off = FLOAT_ARRAY(self->inputs.off);
    const float* SIG_RESTRICT on = FLOAT_ARRAY(self->inputs.on);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = condition[i] > 0.0f ? on[i] : off[i];
    }
}

void sig_dsp_Branch_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Branch* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_List_Outputs_newAudioBlocks(struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_List_Outputs* outputs) {
    outputs->main = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->index = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->length = sig_AudioBlock_newSilent(allocator, audioSettings);
}

void sig_dsp_List_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_List_Outputs* outputs) {
    sig_AudioBlock_destroy(allocator, outputs->main);
    sig_AudioBlock_destroy(allocator, outputs->index);
    sig_AudioBlock_destroy(allocator, outputs->length);
}

struct sig_dsp_List* sig_dsp_List_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_List* self = sig_MALLOC(allocator, struct sig_dsp_List);
    sig_dsp_List_init(self, context);
    sig_dsp_List_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_List_init(struct sig_dsp_List* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_List_generate);
    self->parameters.wrap = 1.0f;
    self->parameters.normalizeIndex = 1.0f;
    self->parameters.interpolate = 0.0f;
    sig_CONNECT_TO_SILENCE(self, index, context);
}

extern inline float sig_dsp_List_constrain(bool shouldWrap,
    float index, float lastIndex, float listLength) {
    if (shouldWrap) {
        while (index < 0.0f) {
            index = listLength + index;
        }

        while (index > lastIndex) {
            index -= lastIndex;
        }
    } else {
        index = sig_clamp(index, 0.0f, lastIndex);
    }

    return index;
}

void sig_dsp_List_generate(void* signal) {
    struct sig_dsp_List* self = (struct sig_dsp_List*) signal;
    struct sig_Buffer* list = self->list;
    const size_t blockSize = self->signal.audioSettings->blockSize;

    // List buffers can only be updated at control rate.
    // TODO: Cache these and only recalculate when
    // the list buffer changes.
    size_t listLength = self->list != NULL ? list->length : 0;
    float listLengthF = (float) listLength;
    size_t lastIndex = listLength - 1;
    float lastIndexF = (float) lastIndex;
    bool shouldWrap = self->parameters.wrap > 0.0f;
    bool shouldInterpolate = self->parameters.interpolate > 0.0f;
    bool shouldNormalize = self->parameters.normalizeIndex > 0.0f;

    float* SIG_RESTRICT mainOut = FLOAT_ARRAY(self->outputs.main);
    float* SIG_RESTRICT indexOut = FLOAT_ARRAY(self->outputs.index);
    float* SIG_RESTRICT lengthOut = FLOAT_ARRAY(self->outputs.length);
    const float* SIG_RESTRICT indexIn = FLOAT_ARRAY(self->inputs.index);

    if (listLength < 1) {
        // There's nothing in the list; just output silence.
        for (size_t i = 0; i < blockSize; i++) {
            mainOut[i] = 0.0f;
            indexOut[i] = -1.0f;
            lengthOut[i] = listLengthF;
        }
    } else {
        for (size_t i = 0; i < blockSize; i++) {
            float index = indexIn[i];
            float sample = 0.0f;

            if (shouldNormalize) {
                index = index * lastIndexF;
            }

            if (shouldInterpolate) {
                index = sig_dsp_List_constrain(shouldWrap, index, lastIndexF,
                    listLengthF);
                sample = sig_interpolate_linear(index, list->samples,
                    listLength);
            } else {
                index = roundf(index);
                index = sig_dsp_List_constrain(shouldWrap, index, lastIndexF,
                    listLengthF);
                sample = FLOAT_ARRAY(list->samples)[(size_t) index];
            }

            mainOut[i] = sample;
            indexOut[i] = index;
            lengthOut[i] = listLengthF;
        }
    }
}

void sig_dsp_List_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_List* self) {
    sig_dsp_List_Outputs_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_Sequencer_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_Sequencer_Outputs* outputs) {
    outputs->main = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->index = sig_AudioBlock_newSilent(allocator, audioSettings);
}

void sig_dsp_Sequencer_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_Sequencer_Outputs* outputs) {
    sig_AudioBlock_destroy(allocator, outputs->main);
    sig_AudioBlock_destroy(allocator, outputs->index);
}

struct sig_dsp_Sequencer* sig_dsp_Sequencer_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Sequencer* self = sig_MALLOC(allocator,
        struct sig_dsp_Sequencer);

    sig_dsp_Sequencer_init(self, context);
    sig_dsp_Sequencer_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Sequencer_init(struct sig_dsp_Sequencer* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Sequencer_generate);
    self->parameters.loop = 1.0f;
    self->parameters.resetOnNext = 0.0f;
    self->parameters.holdLastValue = 0.0f;
    self->samplesRemaining = 0;
    self->stepIndex = -1;
    self->lastValue = 0.0f;

    sig_CONNECT_TO_UNITY(self, mul, context);
    sig_CONNECT_TO_SILENCE(self, add, context);
}

extern inline float sig_dsp_Sequencer_startStage(
    struct sig_dsp_Sequencer* self) {
    float duration = FLOAT_ARRAY(self->durations->samples)[self->stepIndex];
    self->samplesRemaining = (size_t) floor(duration *
        self->signal.audioSettings->sampleRate);
    self->samplesRemaining--;

    return self->parameters.resetOnNext ?
        0.0f : FLOAT_ARRAY(self->values->samples)[self->stepIndex];
};

void sig_dsp_Sequencer_generate(void* signal) {
    struct sig_dsp_Sequencer* self = (struct sig_dsp_Sequencer*) signal;
    const float* SIG_RESTRICT mul = FLOAT_ARRAY(self->inputs.mul);
    const float* SIG_RESTRICT add = FLOAT_ARRAY(self->inputs.add);
    struct sig_Buffer* durations = self->durations;
    struct sig_Buffer* values = self->values;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const size_t valuesLength = values->length;
    const size_t durationsLength = durations->length;
    const bool hasValidSequence = valuesLength == durationsLength &&
        durationsLength > 0 && valuesLength > 0;
    float* SIG_RESTRICT mainOut = FLOAT_ARRAY(self->outputs.main);
    float* SIG_RESTRICT indexOut = FLOAT_ARRAY(self->outputs.index);

    for (size_t i = 0; i < blockSize; i++) {
        float sample = 0.0f;
        if (hasValidSequence) {
            if (self->samplesRemaining == 0) {
                // We're at the start or end of a stage.
                if (self->stepIndex < 0 ||
                    (size_t)self->stepIndex < durationsLength - 1) {
                    // Continue to the next value/duration pair.
                    self->stepIndex++;
                    sample = sig_dsp_Sequencer_startStage(self);
                } else if (self->parameters.loop > 0.0f) {
                    // Loop back to the first value/duration pair.
                    self->stepIndex = 0;
                    sample = sig_dsp_Sequencer_startStage(self);
                } else {
                    // Nothing left to do.
                    sample = self->parameters.holdLastValue ?
                        self->lastValue : 0.0;
                }
            } else {
                // Still in the midst of a stage.
                sample = FLOAT_ARRAY(values->samples)[self->stepIndex];
                self->samplesRemaining--;
            }
        }

        mainOut[i] = self->lastValue = sample *
            mul[i] + add[i];
        indexOut[i] = (float) self->stepIndex;
    }
}

void sig_dsp_Sequencer_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Sequencer* self) {
    sig_dsp_Sequencer_Outputs_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_LinearMap* sig_dsp_LinearMap_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_LinearMap* self = sig_MALLOC(allocator,
        struct sig_dsp_LinearMap);
    sig_dsp_LinearMap_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}
void sig_dsp_LinearMap_init(struct sig_dsp_LinearMap* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_LinearMap_generate);
    self->parameters.fromMin = -1.0f;
    self->parameters.fromMax = 1.0f;
    self->parameters.toMin = -5.0f;
    self->parameters.toMax = 5.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
}

void sig_dsp_LinearMap_generate(void* signal) {
    struct sig_dsp_LinearMap* self = (struct sig_dsp_LinearMap*) signal;

    const float fromMin = self->parameters.fromMin;
    const float fromMax = self->parameters.fromMax;
    const float toMin = self->parameters.toMin;
    const float toMax = self->parameters.toMax;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_linearMap(in[i], fromMin, fromMax, toMin, toMax);
    }
}

void sig_dsp_LinearMap_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_LinearMap* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_TwoOpFM_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_TwoOpFM_Outputs* outputs) {
    outputs->main = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->carrierEOC = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->modulator = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->modulatorEOC = sig_AudioBlock_newSilent(allocator,
        audioSettings);
}

void sig_dsp_TwoOpFM_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_TwoOpFM_Outputs* outputs) {
    sig_AudioBlock_destroy(allocator, outputs->main);
    outputs->main = NULL;
    sig_AudioBlock_destroy(allocator, outputs->carrierEOC);
    outputs->carrierEOC = NULL;
    sig_AudioBlock_destroy(allocator, outputs->modulator);
    outputs->modulator = NULL;
    sig_AudioBlock_destroy(allocator, outputs->modulatorEOC);
    outputs->modulatorEOC = NULL;
}

struct sig_dsp_TwoOpFM* sig_dsp_TwoOpFM_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_TwoOpFM* self = sig_MALLOC(allocator,
        struct sig_dsp_TwoOpFM);
    sig_dsp_TwoOpFM_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);
    self->sineTable = sig_Buffer_new(allocator, 8192);
    self->feedbackDelay.buffer = sig_Buffer_new(allocator, 2);

    sig_dsp_TwoOpFM_init(self, context);

    return self;
}

void sig_dsp_TwoOpFM_init(struct sig_dsp_TwoOpFM* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_TwoOpFM_generate);

    sig_Buffer_fillWithWaveform(self->sineTable, sig_waveform_sine,
        self->sineTable->length, 0.0f, 1.0f);
    sig_osc_Wavetable_init(&self->carrierState, self->sineTable);
    sig_osc_Wavetable_init(&self->modulatorState, self->sineTable);
    sig_DelayLine_init(&self->feedbackDelay);

    sig_CONNECT_TO_SILENCE(self, frequency, context);
    sig_CONNECT_TO_SILENCE(self, index, context);
    sig_CONNECT_TO_SILENCE(self, ratio, context);
    sig_CONNECT_TO_SILENCE(self, phaseOffset, context);
    sig_CONNECT_TO_SILENCE(self, modulatorPhaseOffset, context);
    sig_CONNECT_TO_SILENCE(self, feedbackGain, context);
}

void sig_dsp_TwoOpFM_generate(void* signal) {
    struct sig_dsp_TwoOpFM* self = (struct sig_dsp_TwoOpFM*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float* SIG_RESTRICT frequency = FLOAT_ARRAY(self->inputs.frequency);
    const float* SIG_RESTRICT index = FLOAT_ARRAY(self->inputs.index);
    const float* SIG_RESTRICT ratio = FLOAT_ARRAY(self->inputs.ratio);
    const float* SIG_RESTRICT feedbackGain = FLOAT_ARRAY(self->inputs.feedbackGain);
    const float* SIG_RESTRICT phaseOffset = FLOAT_ARRAY(self->inputs.phaseOffset);
    const float* SIG_RESTRICT modulatorPhaseOffset = FLOAT_ARRAY(
        self->inputs.modulatorPhaseOffset);
    float* SIG_RESTRICT modulatorEOCOut = FLOAT_ARRAY(self->outputs.modulatorEOC);
    float* SIG_RESTRICT carrierEOCOut = FLOAT_ARRAY(self->outputs.carrierEOC);
    float* SIG_RESTRICT mainOut = FLOAT_ARRAY(self->outputs.main);
    float* SIG_RESTRICT modulatorOut = FLOAT_ARRAY(self->outputs.modulator);
    struct sig_Buffer* feedbackBuffer = self->feedbackDelay.buffer;

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    struct sig_osc_Wavetable modulatorState = self->modulatorState;
    struct sig_osc_Wavetable carrierState = self->carrierState;
    struct sig_DelayLine feedbackDelay = self->feedbackDelay;

    for (size_t i = 0; i < blockSize; i++) {
        float fundamental = frequency[i];
        float feedback = sig_filter_mean(feedbackBuffer->samples,
            feedbackBuffer->length) * feedbackGain[i];
        float modulatorFrequency = fundamental * ratio[i];
        float modulatorSample = sig_osc_Wavetable_generate(
            &modulatorState, modulatorFrequency,
            feedback + modulatorPhaseOffset[i], sampleRate,
            &modulatorEOCOut[i]);
        float modulatorSampleScaled = modulatorSample * index[i];
        float carrierPhaseOffset = modulatorSampleScaled + phaseOffset[i];
        float carrierSample = sig_osc_Wavetable_generate(&carrierState,
            fundamental, carrierPhaseOffset, sampleRate, &carrierEOCOut[i]);

        sig_DelayLine_write(&feedbackDelay, carrierSample);
        mainOut[i] = carrierSample;
        modulatorOut[i] = modulatorSample;
    }

    self->modulatorState = modulatorState;
    self->carrierState = carrierState;
    self->feedbackDelay = feedbackDelay;
}

void sig_dsp_TwoOpFM_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_TwoOpFM* self) {
    sig_dsp_TwoOpFM_Outputs_destroyAudioBlocks(allocator, &self->outputs);
    sig_Buffer_destroy(allocator, self->sineTable);
    self->sineTable = NULL;
    sig_Buffer_destroy(allocator, self->feedbackDelay.buffer);
    self->feedbackDelay.buffer = NULL;
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_FourPoleFilter_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_FourPoleFilter_Outputs* outputs) {
    outputs->main = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->twoPole = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->fourPole = sig_AudioBlock_newSilent(allocator, audioSettings);
}

void sig_dsp_FourPoleFilter_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_FourPoleFilter_Outputs* outputs) {
    sig_AudioBlock_destroy(allocator, outputs->main);
    sig_AudioBlock_destroy(allocator, outputs->twoPole);
    sig_AudioBlock_destroy(allocator, outputs->fourPole);
}


struct sig_dsp_Ladder* sig_dsp_Ladder_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Ladder* self = sig_MALLOC(allocator,
        struct sig_dsp_Ladder);
    sig_dsp_Ladder_init(self, context);
    sig_dsp_FourPoleFilter_Outputs_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Ladder_init(
    struct sig_dsp_Ladder* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Ladder_generate);

    struct sig_dsp_Ladder_Parameters parameters = {
        .passbandGain = 0.5f
    };
    self->parameters = parameters;

    sig_filter_Ladder_init(&self->state, 4);
    self->previousFrequency = -1.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, frequency, context);
    sig_CONNECT_TO_SILENCE(self, resonance, context);
    sig_CONNECT_TO_SILENCE(self, inputGain, context);
    sig_CONNECT_TO_SILENCE(self, pole1Gain, context);
    sig_CONNECT_TO_SILENCE(self, pole2Gain, context);
    sig_CONNECT_TO_SILENCE(self, pole3Gain, context);
    sig_CONNECT_TO_UNITY(self, pole4Gain, context);
}

void sig_dsp_Ladder_generate(void* signal) {
    struct sig_dsp_Ladder* self = (struct sig_dsp_Ladder*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    const float passbandGain = self->parameters.passbandGain;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    float* SIG_RESTRICT twoPoleOut = FLOAT_ARRAY(self->outputs.twoPole);
    float* SIG_RESTRICT fourPoleOut = FLOAT_ARRAY(self->outputs.fourPole);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT freqIn = FLOAT_ARRAY(self->inputs.frequency);
    const float* SIG_RESTRICT resIn = FLOAT_ARRAY(self->inputs.resonance);
    const float* SIG_RESTRICT inputGainIn = FLOAT_ARRAY(self->inputs.inputGain);
    const float* SIG_RESTRICT pole1GainIn = FLOAT_ARRAY(self->inputs.pole1Gain);
    const float* SIG_RESTRICT pole2GainIn = FLOAT_ARRAY(self->inputs.pole2Gain);
    const float* SIG_RESTRICT pole3GainIn = FLOAT_ARRAY(self->inputs.pole3Gain);
    const float* SIG_RESTRICT pole4GainIn = FLOAT_ARRAY(self->inputs.pole4Gain);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    struct sig_filter_Ladder state = self->state;
    float previousFrequency = self->previousFrequency;

    for (size_t i = 0; i < blockSize; i++) {
        float frequency = freqIn[i];

        // Recalculate coefficients if the frequency has changed.
        if (frequency != previousFrequency) {
            sig_filter_Ladder_calculateCoefficients(&state, frequency,
                sampleRate);
            previousFrequency = frequency;
        }

        struct sig_filter_Ladder_Taps taps;
        sig_filter_Ladder_generate(&state, in[i], resIn[i],
            passbandGain, &taps);
        out[i] =
            (taps.input * inputGainIn[i]) +
            (taps.pole1 * pole1GainIn[i]) +
            (taps.pole2 * pole2GainIn[i]) +
            (taps.pole3 * pole3GainIn[i]) +
            (taps.pole4 * pole4GainIn[i]);
        twoPoleOut[i] = taps.pole2;
        fourPoleOut[i] = taps.pole4;
    }

    self->state = state;
    self->previousFrequency = previousFrequency;
}

void sig_dsp_Ladder_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Ladder* self) {
    sig_dsp_FourPoleFilter_Outputs_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_TiltEQ* sig_dsp_TiltEQ_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_TiltEQ* self = sig_MALLOC(allocator,
        struct sig_dsp_TiltEQ);
    sig_dsp_TiltEQ_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_TiltEQ_init(struct sig_dsp_TiltEQ* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_TiltEQ_generate);
    self->sr3 = 3.0f * self->signal.audioSettings->sampleRate;
    self->lpOut = 0.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, frequency, context);
    sig_CONNECT_TO_SILENCE(self, gain, context);
}

void sig_dsp_TiltEQ_generate(void* signal) {
    struct sig_dsp_TiltEQ* self = (struct sig_dsp_TiltEQ*) signal;
    const float amp = 8.656170f; // 6.0f / log(2)
    const float gfactor = 5.0f; // Proportional gain.
    const float sr3 = self->sr3;
    float lpOut = self->lpOut;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT freqIn = FLOAT_ARRAY(self->inputs.frequency);
    const float* SIG_RESTRICT gainIn = FLOAT_ARRAY(self->inputs.gain);

    for (size_t i = 0; i < blockSize; i++) {
        float source = in[i];
        float f0 = freqIn[i];
        float gain = gainIn[i] * 6.0f; // Convert to dB.
        float g1, g2;

        if (gain > 0.0f) {
            g1 = -gfactor * gain;
            g2 = gain;
        } else {
            g1 = -gain;
            g2 = gfactor * gain;
        }

        float lgain = exp(g1 / amp) - 1.0f;
        float hgain = exp(g2 / amp) - 1.0f;

        float omega = 2.0f * sig_PI * f0;
        float n = 1.0 / (sr3 + omega);
        float a0 = 2.0 * omega * n;
        float b1 = (sr3 - omega) * n;

        lpOut = a0 * source + b1 * lpOut;
        out[i] = source + lgain * lpOut +
            hgain * (source - lpOut);
    }

    self->lpOut = lpOut;
}

void sig_dsp_TiltEQ_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_TiltEQ* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_SVF_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_SVF_Outputs* outputs) {
    outputs->low = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->band = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->high = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->notch = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->peak = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->all = sig_AudioBlock_newSilent(allocator, audioSettings);
}

void sig_dsp_SVF_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_SVF_Outputs* outputs) {
    sig_AudioBlock_destroy(allocator, outputs->low);
    sig_AudioBlock_destroy(allocator, outputs->band);
    sig_AudioBlock_destroy(allocator, outputs->high);
    sig_AudioBlock_destroy(allocator, outputs->notch);
    sig_AudioBlock_destroy(allocator, outputs->peak);
    sig_AudioBlock_destroy(allocator, outputs->all);
}

// Updates a state variable filter node's coefficients when either of its
// control inputs has changed since the previous sample.
static inline void sig_dsp_SVF_updateCoefficients(struct sig_filter_SVF* state,
    float frequency, float resonance, float sampleRate,
    float* previousFrequency, float* previousResonance) {
    if (frequency != *previousFrequency || resonance != *previousResonance) {
        sig_filter_SVF_calculateCoefficients(state, frequency, resonance,
            sampleRate);
        *previousFrequency = frequency;
        *previousResonance = resonance;
    }
}

struct sig_dsp_SVF* sig_dsp_SVF_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_SVF* self = sig_MALLOC(allocator, struct sig_dsp_SVF);
    sig_dsp_SVF_init(self, context);
    sig_dsp_SVF_Outputs_newAudioBlocks(allocator, context->audioSettings,
        &self->outputs);

    return self;
}

void sig_dsp_SVF_init(struct sig_dsp_SVF* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_SVF_generate);
    sig_filter_SVF_init(&self->state);
    self->previousFrequency = -1.0f;
    self->previousResonance = -1.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, frequency, context);
    sig_CONNECT_TO_SILENCE(self, resonance, context);
}

void sig_dsp_SVF_generate(void* signal) {
    struct sig_dsp_SVF* self = (struct sig_dsp_SVF*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    float* SIG_RESTRICT lowOut = FLOAT_ARRAY(self->outputs.low);
    float* SIG_RESTRICT bandOut = FLOAT_ARRAY(self->outputs.band);
    float* SIG_RESTRICT highOut = FLOAT_ARRAY(self->outputs.high);
    float* SIG_RESTRICT notchOut = FLOAT_ARRAY(self->outputs.notch);
    float* SIG_RESTRICT peakOut = FLOAT_ARRAY(self->outputs.peak);
    float* SIG_RESTRICT allOut = FLOAT_ARRAY(self->outputs.all);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT freqIn = FLOAT_ARRAY(self->inputs.frequency);
    const float* SIG_RESTRICT resIn = FLOAT_ARRAY(self->inputs.resonance);
    // The filter's state is copied into locals, which the compiler can keep
    // in registers, instead of storing and reloading it on every sample.
    struct sig_filter_SVF state = self->state;
    float previousFrequency = self->previousFrequency;
    float previousResonance = self->previousResonance;

    for (size_t i = 0; i < blockSize; i++) {
        float frequency = freqIn[i];
        float resonance = resIn[i];

        sig_dsp_SVF_updateCoefficients(&state, frequency, resonance,
            sampleRate, &previousFrequency, &previousResonance);

        struct sig_filter_SVF_Taps taps;
        sig_filter_SVF_generate(&state, in[i], &taps);
        float k = state.k;
        float input = taps.input;
        float band = taps.band;
        float low = taps.low;
        lowOut[i] = low;
        bandOut[i] = band;
        highOut[i] = sig_filter_svf_highPass(input, band, low, k);
        notchOut[i] = sig_filter_svf_notch(input, band, low, k);
        peakOut[i] = sig_filter_svf_peak(input, band, low, k);
        allOut[i] = sig_filter_svf_allPass(input, band, low, k);
    }

    self->state = state;
    self->previousFrequency = previousFrequency;
    self->previousResonance = previousResonance;
}

void sig_dsp_SVF_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SVF* self) {
    sig_dsp_SVF_Outputs_destroyAudioBlocks(allocator, &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}

struct sig_dsp_SVFLowPass* sig_dsp_SVFLowPass_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context) {
    struct sig_dsp_SVFLowPass* self = sig_MALLOC(allocator,
        struct sig_dsp_SVFLowPass);
    sig_dsp_SVFLowPass_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_SVFLowPass_init(struct sig_dsp_SVFLowPass* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_SVFLowPass_generate);
    sig_filter_SVF_init(&self->state);
    self->previousFrequency = -1.0f;
    self->previousResonance = -1.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, frequency, context);
    sig_CONNECT_TO_SILENCE(self, resonance, context);
}

void sig_dsp_SVFLowPass_generate(void* signal) {
    struct sig_dsp_SVFLowPass* self = (struct sig_dsp_SVFLowPass*) signal;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT freqIn = FLOAT_ARRAY(self->inputs.frequency);
    const float* SIG_RESTRICT resIn = FLOAT_ARRAY(self->inputs.resonance);

    // The filter's state is copied into locals, which the compiler can keep
    // in registers, instead of storing and reloading it on every sample.
    struct sig_filter_SVF state = self->state;
    float previousFrequency = self->previousFrequency;
    float previousResonance = self->previousResonance;

    for (size_t i = 0; i < blockSize; i++) {
        float frequency = freqIn[i];
        float resonance = resIn[i];

        sig_dsp_SVF_updateCoefficients(&state, frequency, resonance,
            sampleRate, &previousFrequency, &previousResonance);

        struct sig_filter_SVF_Taps taps;
        sig_filter_SVF_generate(&state, in[i], &taps);
        out[i] = taps.low;
    }

    self->state = state;
    self->previousFrequency = previousFrequency;
    self->previousResonance = previousResonance;
}

void sig_dsp_SVFLowPass_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SVFLowPass* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_Delay* sig_dsp_Delay_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Delay* self = sig_MALLOC(allocator, struct sig_dsp_Delay);
    // TODO: Improve buffer management throughout Signaletic.
    self->delayLine = context->oneSampleDelayLine;
    sig_dsp_Delay_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Delay_init(struct sig_dsp_Delay* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Delay_generate);

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, delayTime, context);
}

extern inline void sig_dsp_Delay_read(struct sig_dsp_Delay* self,
    float source, size_t i) {
    float delayTime = FLOAT_ARRAY(self->inputs.delayTime)[i];

    FLOAT_ARRAY(self->outputs.main)[i] = sig_DelayLine_cubicReadAtTime(
        self->delayLine,
        source,
        delayTime,
        self->signal.audioSettings->sampleRate);
}

void sig_dsp_Delay_generate(void* signal) {
    struct sig_dsp_Delay* self = (struct sig_dsp_Delay*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT delayTime = FLOAT_ARRAY(self->inputs.delayTime);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float sampleRate = self->signal.audioSettings->sampleRate;

    for (size_t i = 0; i < blockSize; i++) {
        float source = in[i];
        out[i] = sig_DelayLine_cubicReadAtTime(
            self->delayLine,
            source,
            delayTime[i],
            sampleRate);
        sig_DelayLine_write(self->delayLine, source);
    }
}

void sig_dsp_Delay_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Delay* self) {
    // Don't destroy the delay line; it isn't owned.
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_Delay* sig_dsp_DelayTap_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    // TODO: Copy-pasted from sig_dsp_Delay but we have a different
    // initialization function we need to call here. Seems like it's time to
    // decompose Signals into more function pointers!
    struct sig_dsp_Delay* self = sig_MALLOC(allocator, struct sig_dsp_Delay);
    // TODO: Improve buffer management throughout Signaletic.
    self->delayLine = context->oneSampleDelayLine;

    sig_dsp_Delay_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_DelayTap_init(struct sig_dsp_Delay* self,
    struct sig_SignalContext* context) {
    sig_dsp_Delay_init(self, context);
    sig_dsp_Signal_init(self, context, *sig_dsp_DelayTap_generate);
}

void sig_dsp_DelayTap_generate(void* signal) {
    struct sig_dsp_Delay* self = (struct sig_dsp_Delay*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT delayTime = FLOAT_ARRAY(self->inputs.delayTime);
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float sampleRate = self->signal.audioSettings->sampleRate;

    for (size_t i = 0; i < blockSize; i++) {
        float source = in[i];
        out[i] = sig_DelayLine_cubicReadAtTime(
            self->delayLine,
            source,
            delayTime[i],
            sampleRate);
    }
}

void sig_dsp_DelayTap_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Delay* self) {
    // Don't destroy the delay line; it isn't owned.
    sig_dsp_Delay_destroy(allocator, self);
}


struct sig_dsp_DelayWrite* sig_dsp_DelayWrite_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_DelayWrite* self = sig_MALLOC(allocator,
        struct sig_dsp_DelayWrite);
    // TODO: Improve buffer management throughout Signaletic.
    self->delayLine = context->oneSampleDelayLine;

    sig_dsp_DelayWrite_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_DelayWrite_init(struct sig_dsp_DelayWrite* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_DelayWrite_generate);

    sig_CONNECT_TO_SILENCE(self, source, context);
}

void sig_dsp_DelayWrite_generate(void* signal) {
    struct sig_dsp_DelayWrite* self = (struct sig_dsp_DelayWrite*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);

    for (size_t i = 0; i < blockSize; i++) {
        sig_DelayLine_write(self->delayLine, in[i]);
    }

}

void sig_dsp_DelayWrite_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_DelayWrite* self) {
    // Don't destroy the delay line; it isn't owned.
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


// TODO: Resolve duplication with sig_dsp_Delay
struct sig_dsp_Comb* sig_dsp_Comb_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Comb* self = sig_MALLOC(allocator, struct sig_dsp_Comb);
     // TODO: Improve buffer management throughout Signaletic.
    self->delayLine = context->oneSampleDelayLine;

    sig_dsp_Comb_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Comb_init(struct sig_dsp_Comb* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Comb_generate);
    self->previousSample = 0.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, delayTime, context);
    sig_CONNECT_TO_SILENCE(self, feedbackGain, context);
    sig_CONNECT_TO_SILENCE(self, lpfCoefficient, context);
}

void sig_dsp_Comb_generate(void* signal) {
    struct sig_dsp_Comb* self = (struct sig_dsp_Comb*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float maxDelayLength = (float) self->delayLine->buffer->length;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT feedbackGainIn = FLOAT_ARRAY(self->inputs.feedbackGain);
    const float* SIG_RESTRICT delayTimeIn = FLOAT_ARRAY(self->inputs.delayTime);
    const float* SIG_RESTRICT lpfCoefficientIn = FLOAT_ARRAY(self->inputs.lpfCoefficient);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    float previousSample = self->previousSample;

    for (size_t i = 0; i < blockSize; i++) {
        float source = in[i];
        float feedbackGain = feedbackGainIn[i];
        float delayTime = delayTimeIn[i];
        float lpfCoefficient = lpfCoefficientIn[i];
        float readPos = (delayTime * sampleRate);
        if (readPos >= maxDelayLength) {
            readPos = maxDelayLength - 1;
        }

        delayTime = sig_fmaxf(delayTime, 0.00001f); // Delay time can't be zero.
        float read = sig_DelayLine_linearReadAt(self->delayLine, readPos);
        float outputSample = sig_filter_smooth(read, previousSample,
            lpfCoefficient);
        float toWrite = sig_DelayLine_feedback(source, outputSample,
            feedbackGain);
        sig_DelayLine_write(self->delayLine, toWrite);
        out[i] = outputSample;
        previousSample = outputSample;
    }

    self->previousSample = previousSample;
}

void sig_dsp_Comb_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Comb* self) {
    // Don't destroy the delay line; it isn't owned.
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


// TODO: Resolve duplication with sig_dsp_Delay and Comb
struct sig_dsp_Allpass* sig_dsp_Allpass_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Allpass* self = sig_MALLOC(allocator,
        struct sig_dsp_Allpass);
     // TODO: Improve buffer management throughout Signaletic.
    self->delayLine = context->oneSampleDelayLine;

    sig_dsp_Allpass_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_Allpass_init(struct sig_dsp_Allpass* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Allpass_generate);

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, delayTime, context);
    sig_CONNECT_TO_SILENCE(self, g, context);
}

void sig_dsp_Allpass_generate(void* signal) {
    struct sig_dsp_Allpass* self = (struct sig_dsp_Allpass*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float maxDelayLength = (float) self->delayLine->buffer->length;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT delayTimeIn = FLOAT_ARRAY(self->inputs.delayTime);
    const float* SIG_RESTRICT gIn = FLOAT_ARRAY(self->inputs.g);

    for (size_t i = 0; i < blockSize; i++) {
        float source = in[i];
        float delayTime = delayTimeIn[i];
        float g = gIn[i];
        float readPos = (delayTime * sampleRate);
        if (readPos >= maxDelayLength) {
            readPos = maxDelayLength - 1;
        }

        if (delayTime <= 0.0f || g <= 0.0f) {
            out[i] = source;
        } else {
            out[i] = sig_DelayLine_linearAllpass(
                self->delayLine, source, readPos, g);
        }
    }
}

void sig_dsp_Allpass_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Allpass* self) {
    // Don't destroy the delay line; it isn't owned.
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


void sig_dsp_Chorus_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_Chorus_Outputs* outputs) {
    outputs->main = sig_AudioBlock_newSilent(allocator, audioSettings);
    outputs->modulator = sig_AudioBlock_newSilent(allocator, audioSettings);
}

void sig_dsp_Chorus_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_Chorus_Outputs* outputs) {
    sig_AudioBlock_destroy(allocator, outputs->main);
    sig_AudioBlock_destroy(allocator, outputs->modulator);
}

struct sig_dsp_Chorus* sig_dsp_Chorus_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Chorus* self = sig_MALLOC(allocator,
        struct sig_dsp_Chorus);
     // TODO: Improve buffer management throughout Signaletic.
    self->delayLine = context->oneSampleDelayLine;

    sig_dsp_Chorus_init(self, context);
    sig_dsp_Chorus_Outputs_newAudioBlocks(allocator, context->audioSettings,
        &self->outputs);

    return self;
}

void sig_dsp_Chorus_init(struct sig_dsp_Chorus* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Chorus_generate);

    sig_osc_FastLFSine_init(&self->modulator,
        self->signal.audioSettings->sampleRate);

    self->previousFixedOutput = 0.0f;
    self->previousModulatedOutput = 0.0f;

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, delayTime, context);
    sig_CONNECT_TO_SILENCE(self, speed, context);
    sig_CONNECT_TO_SILENCE(self, width, context);
    sig_CONNECT_TO_SILENCE(self, feedbackGain, context);
    sig_CONNECT_TO_SILENCE(self, feedforwardGain, context);
    sig_CONNECT_TO_SILENCE(self, blend, context);
}

void sig_dsp_Chorus_generate(void* signal) {
    struct sig_dsp_Chorus* self = (struct sig_dsp_Chorus*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    const float sampleRate = self->signal.audioSettings->sampleRate;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    float* SIG_RESTRICT modulatorOut = FLOAT_ARRAY(self->outputs.modulator);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT delayTimeIn = FLOAT_ARRAY(self->inputs.delayTime);
    const float* SIG_RESTRICT speedIn = FLOAT_ARRAY(self->inputs.speed);
    const float* SIG_RESTRICT widthIn = FLOAT_ARRAY(self->inputs.width);
    const float* SIG_RESTRICT feedbackGainIn = FLOAT_ARRAY(self->inputs.feedbackGain);
    const float* SIG_RESTRICT feedforwardGainIn = FLOAT_ARRAY(self->inputs.feedforwardGain);
    const float* SIG_RESTRICT blendIn = FLOAT_ARRAY(self->inputs.blend);

    // The state is copied into locals, which the compiler can keep in
    // registers, instead of storing and reloading it on every sample.
    struct sig_osc_FastLFSine modulator = self->modulator;
    float previousFixedOutput = self->previousFixedOutput;
    float previousModulatedOutput = self->previousModulatedOutput;

    for (size_t i = 0; i < blockSize; i++) {
        float source = in[i];
        float delayTime = delayTimeIn[i];
        float speed = speedIn[i];
        float width = widthIn[i];
        float feedbackGain = feedbackGainIn[i];
        float feedforwardGain = feedforwardGainIn[i];
        float blend = blendIn[i];

        sig_osc_FastLFSine_setFrequencyFast(&modulator, speed);
        sig_osc_FastLFSine_generate(&modulator);
        modulatorOut[i] = modulator.sinZ;

        // TODO: Add one pole low pass filters in both the feedback and
        // feedforward lines to support echo.

        float fixedRead = sig_DelayLine_allpassReadAtTime(self->delayLine,
            source, delayTime, sampleRate,
            previousFixedOutput);
        previousFixedOutput = fixedRead;
        float toWrite = source - (fixedRead * feedbackGain);
        sig_DelayLine_write(self->delayLine, toWrite);

        float modulatedDelayTime = modulator.sinZ * width + delayTime;
        float modulatedRead = sig_DelayLine_allpassReadAtTime(self->delayLine,
            source, modulatedDelayTime, sampleRate,
            previousModulatedOutput);
        previousModulatedOutput = modulatedRead;
        float feedforwardSample = modulatedRead * feedforwardGain;
        float output = (toWrite * blend) + feedforwardSample;

        // TODO: What kind of gain staging should we do here?
        // It seems likely that we can have up to 3x gain depending on
        // the values of feedbackGain, feedforwardGain, and blend.
        out[i] = sig_fastTanhf(output / 3.0f);
    }

    self->modulator = modulator;
    self->previousFixedOutput = previousFixedOutput;
    self->previousModulatedOutput = previousModulatedOutput;
}

void sig_dsp_Chorus_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Chorus* self) {
    // Don't destroy the delay line; it isn't owned.
    sig_dsp_Chorus_Outputs_destroyAudioBlocks(allocator, &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_LinearXFade* sig_dsp_LinearXFade_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_LinearXFade* self = sig_MALLOC(allocator,
        struct sig_dsp_LinearXFade);
    sig_dsp_LinearXFade_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_LinearXFade_init(struct sig_dsp_LinearXFade* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_LinearXFade_generate);

    sig_CONNECT_TO_SILENCE(self, left, context);
    sig_CONNECT_TO_SILENCE(self, right, context);
    sig_CONNECT_TO_SILENCE(self, mix, context);
}

void sig_dsp_LinearXFade_generate(void* signal) {
    struct sig_dsp_LinearXFade* self = (struct sig_dsp_LinearXFade*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT left = FLOAT_ARRAY(self->inputs.left);
    const float* SIG_RESTRICT right = FLOAT_ARRAY(self->inputs.right);
    const float* SIG_RESTRICT mix = FLOAT_ARRAY(self->inputs.mix);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_linearXFade(left[i], right[i], mix[i]);
    }
}

void sig_dsp_LinearXFade_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_LinearXFade* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_Calibrator* sig_dsp_Calibrator_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_Calibrator* self = sig_MALLOC(allocator,
        struct sig_dsp_Calibrator);
    sig_dsp_Calibrator_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

extern inline void sig_dsp_Calibrator_Node_init(
    struct sig_dsp_Calibrator_Node* nodes,
    float_array_ptr targetValues, size_t numNodes) {
    for (size_t i = 0; i < numNodes; i++) {
        struct sig_dsp_Calibrator_Node* node = &nodes[i];
        node->target = node->avg = FLOAT_ARRAY(targetValues)[i];
        node->numSamplesRecorded = 0;
        node->min = INFINITY;
        node->max = -INFINITY;
        node->sum = 0.0f;
        node->diff = 0.0f;
    }
}

void sig_dsp_Calibrator_init(struct sig_dsp_Calibrator* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_Calibrator_generate);
    self->previousGate = 0.0f;
    self->stage = 0;

    float targetValues[sig_dsp_Calibrator_NUM_STAGES] =
        sig_dsp_Calibrator_TARGET_VALUES;
    sig_dsp_Calibrator_Node_init(self->nodes,
        targetValues, sig_dsp_Calibrator_NUM_STAGES);

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, gate, context);
}

extern inline size_t sig_dsp_Calibrator_locateIntervalForValue(
    float x, struct sig_dsp_Calibrator_Node* nodes,
    size_t numNodes) {
    size_t lastNodeIdx = numNodes - 1;
    size_t intervalEndIdx = lastNodeIdx;
    for (size_t i = 0; i < lastNodeIdx - 1; i++) {
        size_t nextIdx = i + 1;
        struct sig_dsp_Calibrator_Node nextState = nodes[nextIdx];
        if (x < nextState.avg) {
            intervalEndIdx = nextIdx;
            break;
        }
    }

    return intervalEndIdx;
}

extern inline float sig_dsp_Calibrator_fitValueToCalibrationData(
    float x, struct sig_dsp_Calibrator_Node* nodes,
    size_t numNodes) {
        // Calibrate using piecewise linear fit from
        // readings sampled at 0.0, 1.0, 2.0, 3.0, 4.0 and 4.75V.
        // The ADC tops out slightly before 5 volts,
        // So 4.75V is 9 semitones above the fourth octave.

        // Find the segment in which the current value is located.
        size_t intervalEndIdx = sig_dsp_Calibrator_locateIntervalForValue(x,
            nodes, numNodes);
        struct sig_dsp_Calibrator_Node start = nodes[intervalEndIdx - 1];
        struct sig_dsp_Calibrator_Node end = nodes[intervalEndIdx];

        // y is the interval values measured during the calibration process.
        float yk = start.avg;
        float ykplus1 = end.avg;

        // t is the target interval values.
        float tk = start.target;
        float tkplus1 = end.target;

        // Interpolate the calibrated value.
        return tk + ((tkplus1 - tk) / (ykplus1 - yk)) * (x - yk);
};

void sig_dsp_Calibrator_generate(void* signal) {
    struct sig_dsp_Calibrator* self = (struct sig_dsp_Calibrator*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT sourceIn = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT gateIn = FLOAT_ARRAY(self->inputs.gate);

    for (size_t i = 0; i < blockSize; i++) {
        float source = sourceIn[i];
        float gate = gateIn[i];
        size_t stage = self->stage;
        struct sig_dsp_Calibrator_Node* node = &self->nodes[stage];

        if (gate <= 0.0f && self->previousGate > 0.0f) {
            // Gate is low; recording has just stopped.
            // Calculate offset by discarding the highest and lowest values,
            // and then averaging the rest.
            node->sum -= node->min;
            node->sum -= node->max;
            node->avg = node->sum /
                (node->numSamplesRecorded - 2);
            node->diff = -(node->target - node->avg);
            self->stage = (stage + 1) % sig_dsp_Calibrator_NUM_STAGES;
        } else if (gate > 0.0f) {
            // Gate is high; we're recording.
            node->sum += source;
            node->numSamplesRecorded++;

            if (source < node->min) {
                node->min = source;
            }

            if (source > node->max) {
                node->max = source;
            }
        }

        float px = sig_dsp_Calibrator_fitValueToCalibrationData(source,
            self->nodes, sig_dsp_Calibrator_NUM_STAGES);
        out[i] = px;

        self->previousGate = gate;
    }
}

void sig_dsp_Calibrator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Calibrator* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}

struct sig_dsp_SineWavefolder* sig_dsp_SineWavefolder_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_SineWavefolder* self = sig_MALLOC(allocator,
        struct sig_dsp_SineWavefolder);
    sig_dsp_SineWavefolder_init(self, context);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);

    return self;
}

void sig_dsp_SineWavefolder_init(
    struct sig_dsp_SineWavefolder* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_SineWavefolder_generate);

    sig_CONNECT_TO_SILENCE(self, source, context);
    sig_CONNECT_TO_SILENCE(self, gain, context);
    sig_CONNECT_TO_SILENCE(self, factor, context);
}

void sig_dsp_SineWavefolder_generate(void* signal) {
    struct sig_dsp_SineWavefolder* self =
        (struct sig_dsp_SineWavefolder*) signal;

    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT gain = FLOAT_ARRAY(self->inputs.gain);
    const float* SIG_RESTRICT factor = FLOAT_ARRAY(self->inputs.factor);
    for (size_t i = 0; i < blockSize; i++) {
        out[i] = sig_sineWavefolder(in[i], gain[i], factor[i]);
    }
}

void sig_dsp_SineWavefolder_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SineWavefolder* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, self);
}


struct sig_dsp_NoiseGate* sig_dsp_NoiseGate_new(
    struct sig_Allocator* allocator,
    struct sig_SignalContext* context) {
    struct sig_dsp_NoiseGate* self = sig_MALLOC(allocator,
        struct sig_dsp_NoiseGate);
    sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(allocator,
        context->audioSettings, &self->outputs);
    sig_dsp_NoiseGate_init(self, context);

    return self;
}

void sig_dsp_NoiseGate_init(struct sig_dsp_NoiseGate* self,
    struct sig_SignalContext* context) {
    sig_dsp_Signal_init(self, context, *sig_dsp_NoiseGate_generate);

    struct sig_dsp_ScaleOffset_Parameters parameters = {
        .scale = 1.0,
        .offset= 0.0f
    };
    self->parameters = parameters;

    sig_CONNECT_TO_SILENCE(self, source, context);
}

void sig_dsp_NoiseGate_generate(void* signal) {
    struct sig_dsp_NoiseGate* self = (struct sig_dsp_NoiseGate*) signal;

    const float scale = self->parameters.scale;
    const float offset = self->parameters.offset;
    const size_t blockSize = self->signal.audioSettings->blockSize;
    float* SIG_RESTRICT out = FLOAT_ARRAY(self->outputs.main);
    const float* SIG_RESTRICT in = FLOAT_ARRAY(self->inputs.source);
    const float* SIG_RESTRICT threshold = FLOAT_ARRAY(self->inputs.threshold);
    for (size_t i = 0; i < blockSize; i++) {
        float source = in[i];

        float sample = source >= threshold[i] ? source * scale + offset : 0.0f;
        out[i] = sample;
    }
}

void sig_dsp_NoiseGate_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_NoiseGate* self) {
    sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(allocator,
        &self->outputs);
    sig_dsp_Signal_destroy(allocator, (void*) self);
}
