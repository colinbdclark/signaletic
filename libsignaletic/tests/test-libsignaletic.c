#include <math.h>
#include <unity.h>
#include <tlsf.h>
#include <libsignaletic.h>
#include <buffer-test-utils.h>

#define FLOAT_EPSILON powf(2, -23)
#define HEAP_SIZE 26214400 // 25 MB
#define BLOCK_SIZE 64

uint8_t heapMemory[HEAP_SIZE];

struct sig_AllocatorHeap heap = {
    .length = HEAP_SIZE,
    .memory = (void*) heapMemory
};

struct sig_Allocator allocator = {
    .impl = &sig_TLSFAllocatorImpl,
    .heap = &heap
};

struct sig_AudioSettings mono441kAudioSettings = {
    .blockSize = BLOCK_SIZE,
    .numChannels = 1,
    .sampleRate = 44100.0
};

struct sig_AudioSettings* audioSettings;
struct sig_SignalContext* context;

void setUp(void) {
    allocator.impl->init(&allocator);
    audioSettings = sig_AudioSettings_new(&allocator);
    context = sig_SignalContext_new(&allocator, audioSettings);
}

void tearDown(void) {
    sig_AudioSettings_destroy(&allocator, audioSettings);
    sig_SignalContext_destroy(&allocator, context);
}

void test_sig_flooredfmodf(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.7f, sig_flooredfmodf(-0.3f, 1.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, sig_flooredfmodf(-1.5f, 1.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sig_flooredfmodf(-2.0f, 1.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.3f, sig_flooredfmodf(0.3f, 1.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.8f, sig_flooredfmodf(1.8f, 1.0f));
}

void test_sig_fastMod1f(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.7f, sig_fastMod1f(-0.3f));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, sig_fastMod1f(-1.5f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sig_fastMod1f(-2.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.3f, sig_fastMod1f(0.3f));
    TEST_ASSERT_EQUAL_FLOAT(0.8f, sig_fastMod1f(1.8f));
}

void test_sig_clamp(void) {
    // Value within range.
    TEST_ASSERT_EQUAL_FLOAT(0.5f, sig_clamp(0.5f, -1.0f, 1.0f));

    // Too small.
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, sig_clamp(-2.0f, -1.0f, 1.0f));

    // Too big.
    TEST_ASSERT_EQUAL_FLOAT(1.0f, sig_clamp(2.0f, -1.0f, 1.0f));

    // Equal to min.
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, sig_clamp(-1.0f, -1.0f, 1.0f));

    // Equal to max.
    TEST_ASSERT_EQUAL_FLOAT(1.0f, sig_clamp(1.0f, -1.0f, 1.0f));

    // Negative range
    TEST_ASSERT_EQUAL_FLOAT(-0.5f, sig_clamp(-0.5f, -1.0f, 0.0f));

    // Negative value in a positive-only range
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sig_clamp(-0.5f, 0.0f, 1.0f));

    // Min equals max.
    TEST_ASSERT_EQUAL_FLOAT(3.0f, sig_clamp(100.0f, 3.0f, 3.0f));
}

void test_sig_linearMap_zeroSourceRange(void) {
    // If the source range's length is zero, linearMap should return toMin.
    TEST_ASSERT_EQUAL_FLOAT(7.0f,
        sig_linearMap(100.0f, 3.0f, 3.0f, 7.0f, 42.0f));
    TEST_ASSERT_EQUAL_FLOAT(7.0f,
        sig_linearMap(3.0f, 3.0f, 3.0f, 7.0f, 42.0f));
    TEST_ASSERT_EQUAL_FLOAT(7.0f,
        sig_linearMap(-1000.0f, 3.0f, 3.0f, 7.0f, 42.0f));
}

void test_sig_linearMap_zeroTargetRange(void) {
    // Mapping to a zero-length target range should return toMin.
    TEST_ASSERT_EQUAL_FLOAT(5.0f,
        sig_linearMap(0.0f, 0.0f, 1.0f, 5.0f, 5.0f));
    TEST_ASSERT_EQUAL_FLOAT(5.0f,
        sig_linearMap(0.5f, 0.0f, 1.0f, 5.0f, 5.0f));
    TEST_ASSERT_EQUAL_FLOAT(5.0f,
        sig_linearMap(1.0f, 0.0f, 1.0f, 5.0f, 5.0f));
}

void test_sig_linearMap_identity(void) {
    // [0, 1] -> [0, 1] is an identity transform.
    TEST_ASSERT_EQUAL_FLOAT(0.0f,
        sig_linearMap(0.0f, 0.0f, 1.0f, 0.0f, 1.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.5f,
        sig_linearMap(0.5f, 0.0f, 1.0f, 0.0f, 1.0f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f,
        sig_linearMap(1.0f, 0.0f, 1.0f, 0.0f, 1.0f));
}

void test_sig_linearMap_scaling(void) {
    // Map [0, 1] -> [0, 100].
    TEST_ASSERT_EQUAL_FLOAT(0.0f,
        sig_linearMap(0.0f, 0.0f, 1.0f, 0.0f, 100.0f));
    TEST_ASSERT_EQUAL_FLOAT(50.0f,
        sig_linearMap(0.5f, 0.0f, 1.0f, 0.0f, 100.0f));
    TEST_ASSERT_EQUAL_FLOAT(100.0f,
        sig_linearMap(1.0f, 0.0f, 1.0f, 0.0f, 100.0f));
}

void test_sig_linearMap_offset(void) {
    // Map [0, 1] -> [10, 20].
    TEST_ASSERT_EQUAL_FLOAT(10.0f,
        sig_linearMap(0.0f, 0.0f, 1.0f, 10.0f, 20.0f));
    TEST_ASSERT_EQUAL_FLOAT(15.0f,
        sig_linearMap(0.5f, 0.0f, 1.0f, 10.0f, 20.0f));
    TEST_ASSERT_EQUAL_FLOAT(20.0f,
        sig_linearMap(1.0f, 0.0f, 1.0f, 10.0f, 20.0f));
}

void test_sig_linearMap_inverted(void) {
    // Map [0, 1] -> [1, 0] (descending target range).
    TEST_ASSERT_EQUAL_FLOAT(1.0f,
        sig_linearMap(0.0f, 0.0f, 1.0f, 1.0f, 0.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.5f,
        sig_linearMap(0.5f, 0.0f, 1.0f, 1.0f, 0.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f,
        sig_linearMap(1.0f, 0.0f, 1.0f, 1.0f, 0.0f));
}

void test_sig_linearMap_negativeRange(void) {
    // Map [-1, 1] -> [0, 10].
    TEST_ASSERT_EQUAL_FLOAT(0.0f,
        sig_linearMap(-1.0f, -1.0f, 1.0f, 0.0f, 10.0f));
    TEST_ASSERT_EQUAL_FLOAT(5.0f,
        sig_linearMap(0.0f, -1.0f, 1.0f, 0.0f, 10.0f));
    TEST_ASSERT_EQUAL_FLOAT(10.0f,
        sig_linearMap(1.0f, -1.0f, 1.0f, 0.0f, 10.0f));
}

void test_sig_linearMap_bothNegative(void) {
    // Map [-5, -1] -> [-10, -4].
    TEST_ASSERT_EQUAL_FLOAT(-10.0f,
        sig_linearMap(-5.0f, -5.0f, -1.0f, -10.0f, -4.0f));
    TEST_ASSERT_EQUAL_FLOAT(-7.0f,
        sig_linearMap(-3.0f, -5.0f, -1.0f, -10.0f, -4.0f));
    TEST_ASSERT_EQUAL_FLOAT(-4.0f,
        sig_linearMap(-1.0f, -5.0f, -1.0f, -10.0f, -4.0f));
}

void test_sig_linearMap_clampedBelow(void) {
    // Value below fromMin should clamp to toMin.
    TEST_ASSERT_EQUAL_FLOAT(10.0f,
        sig_linearMap(-10.0f, 0.0f, 1.0f, 10.0f, 20.0f));
    TEST_ASSERT_EQUAL_FLOAT(10.0f,
        sig_linearMap(-1.0f, 0.0f, 1.0f, 10.0f, 20.0f));
}

void test_sig_linearMap_clampedAbove(void) {
    // Value above fromMax should clamp to toMax.
    TEST_ASSERT_EQUAL_FLOAT(20.0f,
        sig_linearMap(2.0f, 0.0f, 1.0f, 10.0f, 20.0f));
    TEST_ASSERT_EQUAL_FLOAT(20.0f,
        sig_linearMap(100.0f, 0.0f, 1.0f, 10.0f, 20.0f));
}

void test_sig_unipolarToUint12(void) {
    TEST_ASSERT_EQUAL_UINT16(2047, sig_unipolarToUint12(0.5f));
    TEST_ASSERT_EQUAL_UINT16(0, sig_unipolarToUint12(0.0f));
    TEST_ASSERT_EQUAL_UINT16(4095, sig_unipolarToUint12(1.0f));
}

void test_sig_bipolarToUint12(void) {
    TEST_ASSERT_EQUAL_UINT16(4095, sig_bipolarToUint12(1.0f));
    TEST_ASSERT_EQUAL_UINT16(3071, sig_bipolarToUint12(0.5f));
    TEST_ASSERT_EQUAL_UINT16(2047, sig_bipolarToUint12(0.0f));
    TEST_ASSERT_EQUAL_UINT16(1023, sig_bipolarToUint12(-0.5f));
    TEST_ASSERT_EQUAL_UINT16(0, sig_bipolarToUint12(-1.0f));
}

void test_sig_bipolarToInvUint12(void) {
    TEST_ASSERT_EQUAL_UINT16(0, sig_bipolarToInvUint12(1.0f));
    TEST_ASSERT_EQUAL_UINT16(1023, sig_bipolarToInvUint12(0.5f));
    TEST_ASSERT_EQUAL_UINT16(2047, sig_bipolarToInvUint12(0.0f));
    TEST_ASSERT_EQUAL_UINT16(3071, sig_bipolarToInvUint12(-0.5f));
    TEST_ASSERT_EQUAL_UINT16(4095, sig_bipolarToInvUint12(-1.0f));
}

void test_sig_randf(void) {
    size_t numSamples = 8192;

    // Values should be within 0.0 to 1.0
    struct sig_Buffer* firstRun = sig_Buffer_new(&allocator,
        numSamples);
    sig_Buffer_fill(firstRun, sig_randomFill);
    testAssertBufferValuesInRange(firstRun->samples, numSamples,
        0.0f, 1.0f);

    // Consecutive runs should contain different values.
    struct sig_Buffer* secondRun = sig_Buffer_new(&allocator,
        numSamples);
    sig_Buffer_fill(secondRun, sig_randomFill);
    testAssertBuffersNotEqual(firstRun->samples, secondRun->samples,
        numSamples);

    // Using the same seed for each run
    // should produce identical results.
    struct sig_Buffer* thirdRun = sig_Buffer_new(&allocator,
        numSamples);
    struct sig_Buffer* fourthRun = sig_Buffer_new(&allocator,
        numSamples);
    sig_randf_seed(1);
    sig_Buffer_fill(thirdRun, sig_randomFill);

    sig_randf_seed(1);
    sig_Buffer_fill(fourthRun, sig_randomFill);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(thirdRun->samples,
        fourthRun->samples, numSamples);
}

void test_sig_randf_seed(void) {
    // The precomputed static state behind sig_randf must match a fresh
    // seed of 1, so the Option B initializer can never drift out of
    // sync with sig_Random_seed.
    struct sig_Random expected;
    sig_Random_seed(&expected, 1);

    sig_randf_seed(1);
    for (size_t i = 0; i < 16; i++) {
        // Store values in local variables because
        // Unity's TEST_ASSERT_EQUAL_FLOAT evaluates its
        // expected argument twice.
        float expectedValue = sig_Random_generate(&expected);
        float actualValue = sig_randf();
        TEST_ASSERT_EQUAL_FLOAT(expectedValue, actualValue);
    }
}

void test_sig_Random_withinRange(void) {
    size_t numSamples = 8192;
    struct sig_Random rng;
    sig_Random_init(&rng, 42);

    // All values should be between 0.0 and 1.0, inclusive.
    int32_t failureIdx = -1;
    for (int32_t i = 0; i < numSamples; i++) {
        float value = sig_Random_generate(&rng);
        if (value < 0.0f || value > 1.0f) {
            failureIdx = i;
            break;
        }
    }

    TEST_ASSERT_EQUAL_INT32_MESSAGE(-1, failureIdx,
        "Value at index was not within [0.0, 1.0].");
}

void test_sig_Random_mean(void) {
    size_t numSamples = 32768;
    struct sig_Random rng;
    sig_Random_init(&rng, 42);

    // The mean of a uniform generator between 0.0 and 1.0 should
    // be approximately 0.5.
    double sum = 0.0;
    for (size_t i = 0; i < numSamples; i++) {
        sum += (double) sig_Random_generate(&rng);
    }
    float mean = (float) (sum / (double) numSamples);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 0.5f, mean,
        "The mean of many random values should be approximately 0.5");
}

void test_sig_Random_reproducibility(void) {
    size_t numSamples = 1024;

    // Two generators seeded identically should produce
    // the exact same sequences.
    struct sig_Random* first = sig_Random_new(&allocator, 99);
    struct sig_Random* second = sig_Random_new(&allocator, 99);
    for (size_t i = 0; i < numSamples; i++) {
        uint32_t firstValue = sig_Random_nextU32(first);
        uint32_t secondValue = sig_Random_nextU32(second);
        TEST_ASSERT_EQUAL_HEX32(firstValue, secondValue);
    }
    sig_Random_destroy(&allocator, first);
    sig_Random_destroy(&allocator, second);

    // Re-seeding a generator mid-stream restarts
    // the sequence, confirming that the same seed
    // will reproduce the sequence.
    struct sig_Random* roving = sig_Random_new(&allocator, 7);
    uint32_t reference[16];
    for (size_t i = 0; i < 16; i++) {
        reference[i] = sig_Random_nextU32(roving);
    }
    for (size_t i = 0; i < 100; i++) {
        sig_Random_nextU32(roving);
    }
    sig_Random_seed(roving, 7);
    for (size_t i = 0; i < 16; i++) {
        uint32_t value = sig_Random_nextU32(roving);
        TEST_ASSERT_EQUAL_HEX32(reference[i], value);
    }
    sig_Random_destroy(&allocator, roving);
}

void test_sig_Random_independence(void) {
    size_t numSamples = 256;
    uint32_t referenceA[256];
    uint32_t referenceB[256];

    // Capture each generator's sequence in isolation.
    struct sig_Random* soloA = sig_Random_new(&allocator, 11);
    for (size_t i = 0; i < numSamples; i++) {
        referenceA[i] = sig_Random_nextU32(soloA);
    }
    sig_Random_destroy(&allocator, soloA);

    struct sig_Random* soloB = sig_Random_new(&allocator, 22);
    for (size_t i = 0; i < numSamples; i++) {
        referenceB[i] = sig_Random_nextU32(soloB);
    }
    sig_Random_destroy(&allocator, soloB);

    // sig_Random instances should to be isolated from each other.
    // Confirm this by generating interleaving numbers,
    // and the sequence should be as expected.
    struct sig_Random* a = sig_Random_new(&allocator, 11);
    struct sig_Random* b = sig_Random_new(&allocator, 22);
    for (size_t i = 0; i < numSamples; i++) {
        uint32_t aValue = sig_Random_nextU32(a);
        uint32_t bValue = sig_Random_nextU32(b);
        TEST_ASSERT_EQUAL_HEX32(referenceA[i], aValue);
        TEST_ASSERT_EQUAL_HEX32(referenceB[i], bValue);
    }
    sig_Random_destroy(&allocator, a);
    sig_Random_destroy(&allocator, b);
}

void test_sig_midiToFreq(void) {
    // 69 A 440
    float actual = sig_midiToFreq(69.0f);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(FLOAT_EPSILON, 440.0f, actual,
        "MIDI A4 should be 440 Hz");

    // 60 Middle C 261.63
    actual = sig_midiToFreq(60.0f);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005, sig_FREQ_C4, actual,
        "MIDI C3 should be 261.63 Hz");

    // MIDI 0
    actual = sig_midiToFreq(0.0f);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005, 8.176f, actual,
        "MIDI 0 should be 8.18 Hz");

    // Quarter tone
    actual = sig_midiToFreq(60.5f);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005, 269.29f, actual,
        "A quarter tone above C3 should be 269.29 Hz");

    // Negative MIDI note numbers should return viable frequencies.
    actual = sig_midiToFreq(-60.0f);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005, 0.2555, actual,
        "A negative MIDI number should return a negative frequency.");
}

void test_sig_freqToMidi(void) {
    // 69 A 440
    float actual = sig_freqToMidi(440.0f);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(FLOAT_EPSILON, 69.0f, actual,
        "MIDI A4 should be 440 Hz");

    // 60 Middle C 261.63
    actual = sig_freqToMidi(sig_FREQ_C4);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005f, 60.0f, actual,
        "MIDI C3 should be 261.63 Hz");

    // MIDI 0
    actual = sig_freqToMidi(8.176f);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005f, 0.0f, actual,
        "MIDI 0 should be 8.18 Hz");

    // Quarter tone
    actual = sig_freqToMidi(269.29f);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005f, 60.5f, actual,
        "A quarter tone above C3 should be 269.29 Hz");

    // Negative MIDI note numbers should return viable frequencies.
    actual = sig_freqToMidi(0.2555f);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005f, -60.0f, actual,
        "A negative MIDI number should return a negative frequency.");
}

void test_sig_sig_linearToFreq(void) {
    // 0.0f - 0V - Middle C4 - 261.626
    float actual = sig_linearToFreq(0.0f, sig_FREQ_C4);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(sig_FREQ_C4, actual,
        "0.0f should be middle C 261.626 Hz");

    // 0.75f - 0.75V - A4 440
    actual = sig_linearToFreq(0.75f, sig_FREQ_C4);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 440.0f, actual,
        "3/4 of a volt should be 440 Hz");

    // 2.5V - F#6 - 1479.98
    actual = sig_linearToFreq(2.5f, sig_FREQ_C4);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005, 1479.98f, actual,
        "2.5V should be F#5 739.99 Hz");

    // -1.25 - A2 - 110 Hz
    actual = sig_linearToFreq(-1.25f, sig_FREQ_C4);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005, 110.0f, actual,
        "-1.25V should be A2 110 Hz");

}

void test_sig_sig_freqToLinear(void) {
    // 0.0f - 0V - Middle C4 - 261.626 - 20Vpp
    float actual = sig_freqToLinear(sig_FREQ_C4, sig_FREQ_C4);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.0f, actual,
        "0.0f should be middle C 261.626 Hz");

    // 0.75f - 0.75V - A4 440 - 1Vpp
    actual = sig_freqToLinear(440.0f, sig_FREQ_C4);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 0.75f, actual,
        "3/4 of a volt should be 440 Hz");

    // 2.5V - F#6 - 1479.98 - 10Vpp
    actual = sig_freqToLinear(1479.98f, sig_FREQ_C4);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005, 2.5f, actual,
        "2.5V should be F#5 739.99 Hz");

    // -1.25 - A2 - 110 Hz
    actual = sig_freqToLinear(110.0f, sig_FREQ_C4);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.005, -1.25f, actual,
        "-1.25V should be A2 110 Hz");
}

void test_sig_fillWithValue(void) {
    float actual[BLOCK_SIZE];
    float expected[BLOCK_SIZE];
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        expected[i] = 440.4f;
    };

    sig_fillWithValue(actual, BLOCK_SIZE, 440.4f);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, actual, BLOCK_SIZE);
}

void test_sig_fillWithSilence(void) {
    float buffer[16];
    sig_fillWithSilence(buffer, 16);
    testAssertBufferIsSilent(&allocator, buffer, 16);
}

void test_sig_AudioSettings_new(void) {
    struct sig_AudioSettings* s = sig_AudioSettings_new(&allocator);

    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(sig_DEFAULT_AUDIOSETTINGS.sampleRate,
        s->sampleRate, "The sample rate should be set to the default.");

    TEST_ASSERT_EQUAL_size_t_MESSAGE(sig_DEFAULT_AUDIOSETTINGS.numChannels,
        s->numChannels, "The channels should be set to the default.");

    TEST_ASSERT_EQUAL_size_t_MESSAGE(sig_DEFAULT_AUDIOSETTINGS.blockSize,
        s->blockSize, "The block size should be set to the default.");

    sig_AudioSettings_destroy(&allocator, s);
}

void test_sig_MALLOC_acceptsAddressOfAllocator(void) {
    // The allocator argument can be any expression, such as the address of
    // an allocator struct.
    struct sig_osc_Oscillator* oscillator = sig_MALLOC(&allocator,
        struct sig_osc_Oscillator);
    TEST_ASSERT_NOT_NULL(oscillator);
    allocator.impl->free(&allocator, oscillator);
}

void test_sig_samplesToSeconds(void) {
    struct sig_AudioSettings* s = sig_AudioSettings_new(&allocator);

    size_t expected = 48000;
    size_t actual = sig_secondsToSamples(s, 1.0f);
    TEST_ASSERT_EQUAL_size_t_MESSAGE(expected, actual,
        "One second should be the same as the sample rate.");

    expected = 16000;
    actual = sig_secondsToSamples(s, 1.0f/3.0f);
    TEST_ASSERT_EQUAL_size_t_MESSAGE(expected, actual,
        "Even division of sample rate should as expected.");

    // Fractional samples should be rounded: 1/7" = 6857.142857142857143 samps
    expected = 6857;
    actual = sig_secondsToSamples(s, 1.0f/7.0f);
    TEST_ASSERT_EQUAL_size_t_MESSAGE(expected, actual,
        "Fractional samples < 0.5 should be rounded down.");

    // 1/11" = 4363.636363636363757 samps
    expected = 4364;
    actual = sig_secondsToSamples(s, 1.0f/11.0f);
    TEST_ASSERT_EQUAL_size_t_MESSAGE(expected, actual,
        "Fractional samples >= 0.5 should be rounded up.");
}

void test_sig_Audio_Block_newWithValue_testForValue(
    struct sig_Allocator* localAlloc,
    struct sig_AudioSettings* customSettings,
    float value) {
    float* actual = sig_AudioBlock_newWithValue(localAlloc,
        customSettings, value);
    testAssertBufferContainsValueOnly(&allocator, value, actual,
        customSettings->blockSize);
    sig_AudioBlock_destroy(localAlloc, actual);
}

void test_sig_AudioBlock_newWithValue(void) {
    // Note: This "heap" is actually stored here on the stack,
    // so it needs to stay small, or else has to be moved
    // onto the program's heap. It's here now so that this test
    // is more self-contained.
    char customMemory[32768];
    struct sig_AllocatorHeap customHeap = {
        .length = 32768,
        .memory = customMemory
    };
    struct sig_Allocator localAlloc = {
        .impl = &sig_TLSFAllocatorImpl,
        .heap = &customHeap
    };

    localAlloc.impl->init(&localAlloc);

    struct sig_AudioSettings* customSettings =
        sig_AudioSettings_new(&localAlloc);

    test_sig_Audio_Block_newWithValue_testForValue(&localAlloc,
        customSettings, 440.0);
    test_sig_Audio_Block_newWithValue_testForValue(&localAlloc,
        customSettings, 0.0);
    test_sig_Audio_Block_newWithValue_testForValue(&localAlloc,
        customSettings, 1.0);
    test_sig_Audio_Block_newWithValue_testForValue(&localAlloc,
        customSettings, 0.0);

    sig_AudioSettings_destroy(&localAlloc, customSettings);
}

void test_sig_interpolate_linear_wrapsAtEnd(void) {
    // The table is the first four samples. The rest of the array
    // contains recognizable values that would be returned if the
    // implementation reads past the end.
    float samples[8] = {
        0.0f, 1.0f, 2.0f, 3.0f, 100.0f, 100.0f, 100.0f, 100.0f
    };
    size_t length = 4;

    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.0f,
        sig_interpolate_linear(3.0f, samples, length));

    // Between the last sample and the first.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.5f,
        sig_interpolate_linear(3.5f, samples, length));

    // Exactly at the end of the table, which is the start of the next cycle.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
        sig_interpolate_linear(4.0f, samples, length));
}

void test_sig_osc_Wavetable_negativePhaseStaysWithinTable(void) {
    // The table is the first four samples. The rest of the array
    // contains recognizable values that would be returned if the
    // implementation reads past the end.
    float samples[8] = {
        5.0f, 5.0f, 5.0f, 5.0f, 100.0f, 100.0f, 100.0f, 100.0f
    };
    struct sig_Buffer table = {
        .length = 4,
        .samples = samples
    };
    struct sig_osc_Wavetable osc;
    sig_osc_Wavetable_init(&osc, &table);
    float eoc = 0.0f;

    // A phase just below zero, as produced by through-zero or negative
    // phase modulation, wraps to the very end of the cycle.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 5.0f,
        sig_osc_Wavetable_generate(&osc, 0.0f, -1.0e-9f, 44100.0f, &eoc));
}

void test_sig_FFT_forward(void) {
    size_t length = 64;
    struct sig_FFT* fft = sig_FFT_new(&allocator, length);
    float_array_ptr real = sig_samples_new(&allocator, length);
    float_array_ptr imaginary = sig_samples_new(&allocator, length);
    for (size_t n = 0; n < length; n++) {
        float t = sig_TWOPI * (float) n / (float) length;
        FLOAT_ARRAY(real)[n] = 0.1f + 0.5f * cosf(3.0f * t) +
            0.25f * sinf(5.0f * t);
        FLOAT_ARRAY(imaginary)[n] = 0.0f;
    }

    sig_FFT_forward(fft, real, imaginary);

    // The constant, the cosine, and the sine each appear in their own bins,
    // scaled by the length. The cosine and sine are split between their
    // positive and negative frequency bins.
    float half = (float) length / 2.0f;
    for (size_t k = 0; k < length; k++) {
        float expectedReal = 0.0f;
        float expectedImaginary = 0.0f;
        if (k == 0) {
            expectedReal = 0.1f * (float) length;
        } else if (k == 3 || k == length - 3) {
            expectedReal = 0.5f * half;
        } else if (k == 5) {
            expectedImaginary = -0.25f * half;
        } else if (k == length - 5) {
            expectedImaginary = 0.25f * half;
        }
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, expectedReal,
            FLOAT_ARRAY(real)[k]);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, expectedImaginary,
            FLOAT_ARRAY(imaginary)[k]);
    }

    sig_FFT_destroy(&allocator, fft);
}

void test_sig_FFT_inverse(void) {
    size_t length = 64;
    struct sig_FFT* fft = sig_FFT_new(&allocator, length);
    float_array_ptr real = sig_samples_new(&allocator, length);
    float_array_ptr imaginary = sig_samples_new(&allocator, length);
    sig_fillWithSilence(real, length);
    sig_fillWithSilence(imaginary, length);

    // A single bin becomes a complex exponential at its frequency.
    FLOAT_ARRAY(real)[2] = 1.0f;
    sig_FFT_inverse(fft, real, imaginary);
    for (size_t n = 0; n < length; n++) {
        float t = sig_TWOPI * 2.0f * (float) n / (float) length;
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, cosf(t), FLOAT_ARRAY(real)[n]);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, sinf(t),
            FLOAT_ARRAY(imaginary)[n]);
    }

    // A forward transform followed by an inverse transform multiplies the
    // signal by the length.
    for (size_t n = 0; n < length; n++) {
        FLOAT_ARRAY(real)[n] = sinf((float) n * 0.37f) + 0.2f;
        FLOAT_ARRAY(imaginary)[n] = 0.0f;
    }
    sig_FFT_forward(fft, real, imaginary);
    sig_FFT_inverse(fft, real, imaginary);
    for (size_t n = 0; n < length; n++) {
        TEST_ASSERT_FLOAT_WITHIN(0.001f,
            (sinf((float) n * 0.37f) + 0.2f) * (float) length,
            FLOAT_ARRAY(real)[n]);
        TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, FLOAT_ARRAY(imaginary)[n]);
    }

    sig_FFT_destroy(&allocator, fft);
}

void test_sig_waveform_harmonicGain(void) {
    // A sine contains only its fundamental.
    TEST_ASSERT_EQUAL_FLOAT(1.0f,
        sig_waveform_harmonicGain(sig_StandardWaveform_SINE, 1));
    TEST_ASSERT_EQUAL_FLOAT(0.0f,
        sig_waveform_harmonicGain(sig_StandardWaveform_SINE, 2));
    TEST_ASSERT_EQUAL_FLOAT(0.0f,
        sig_waveform_harmonicGain(sig_StandardWaveform_SINE, 3));

    // Triangles contain only odd harmonics, falling by 1/k^2.
    TEST_ASSERT_EQUAL_FLOAT(8.0f / (sig_PI * sig_PI),
        sig_waveform_harmonicGain(sig_StandardWaveform_TRIANGLE, 1));
    TEST_ASSERT_EQUAL_FLOAT(0.0f,
        sig_waveform_harmonicGain(sig_StandardWaveform_TRIANGLE, 2));
    TEST_ASSERT_EQUAL_FLOAT(8.0f / (9.0f * sig_PI * sig_PI),
        sig_waveform_harmonicGain(sig_StandardWaveform_TRIANGLE, 3));

    // Rising saws contain every harmonic, falling by 1/k,
    // with negative sine gains.
    TEST_ASSERT_EQUAL_FLOAT(-2.0f / sig_PI,
        sig_waveform_harmonicGain(sig_StandardWaveform_SAW, 1));
    TEST_ASSERT_EQUAL_FLOAT(-1.0f / sig_PI,
        sig_waveform_harmonicGain(sig_StandardWaveform_SAW, 2));
    TEST_ASSERT_EQUAL_FLOAT(-2.0f / (3.0f * sig_PI),
        sig_waveform_harmonicGain(sig_StandardWaveform_SAW, 3));

    // Squares contain only odd harmonics, falling by 1/k.
    TEST_ASSERT_EQUAL_FLOAT(4.0f / sig_PI,
        sig_waveform_harmonicGain(sig_StandardWaveform_SQUARE, 1));
    TEST_ASSERT_EQUAL_FLOAT(0.0f,
        sig_waveform_harmonicGain(sig_StandardWaveform_SQUARE, 2));
    TEST_ASSERT_EQUAL_FLOAT(4.0f / (3.0f * sig_PI),
        sig_waveform_harmonicGain(sig_StandardWaveform_SQUARE, 3));

    // Reverse saws are inverted saws.
    for (size_t k = 1; k <= 3; k++) {
        TEST_ASSERT_EQUAL_FLOAT(
            -sig_waveform_harmonicGain(sig_StandardWaveform_SAW, k),
            sig_waveform_harmonicGain(sig_StandardWaveform_REVERSE_SAW, k));
    }
}

void test_sig_waveform_harmonicTaper(void) {
    // Without a taper, every harmonic keeps its full gain.
    for (size_t k = 1; k <= 3; k++) {
        TEST_ASSERT_EQUAL_FLOAT(1.0f,
            sig_waveform_harmonicTaper(sig_HarmonicTaper_NONE, k, 3));
    }

    // Lanczos sigma factors fall smoothly towards zero just beyond the
    // highest harmonic. They are normalized so that the fundamental keeps
    // its full gain, whatever the number of harmonics.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f,
        sig_waveform_harmonicTaper(sig_HarmonicTaper_LANCZOS, 1, 3));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.7071068f,
        sig_waveform_harmonicTaper(sig_HarmonicTaper_LANCZOS, 2, 3));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.3333333f,
        sig_waveform_harmonicTaper(sig_HarmonicTaper_LANCZOS, 3, 3));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f,
        sig_waveform_harmonicTaper(sig_HarmonicTaper_LANCZOS, 1, 1));
}

void test_sig_STANDARD_WAVEFORMS(void) {
    // Each standard waveform pairs its naive generator
    // with the generator for its harmonic series.
    sig_waveform_generator generators[sig_StandardWaveform_COUNT] = {
        sig_waveform_sine,
        sig_waveform_triangle,
        sig_waveform_saw,
        sig_waveform_square,
        sig_waveform_reverseSaw
    };
    sig_waveform_harmonicGenerator harmonics[sig_StandardWaveform_COUNT] = {
        sig_waveform_sineHarmonicGain,
        sig_waveform_triangleHarmonicGain,
        sig_waveform_sawHarmonicGain,
        sig_waveform_squareHarmonicGain,
        sig_waveform_reverseSawHarmonicGain
    };

    for (size_t i = 0; i < sig_StandardWaveform_COUNT; i++) {
        TEST_ASSERT_TRUE(sig_STANDARD_WAVEFORMS[i].generate == generators[i]);
        TEST_ASSERT_TRUE(sig_STANDARD_WAVEFORMS[i].harmonic == harmonics[i]);
    }

    // Triangle harmonics are cosines; the others are sines.
    for (size_t i = 0; i < sig_StandardWaveform_COUNT; i++) {
        float expectedPhase = i == sig_StandardWaveform_TRIANGLE ?
            0.25f : 0.0f;
        TEST_ASSERT_EQUAL_FLOAT(expectedPhase,
            sig_STANDARD_WAVEFORMS[i].harmonicPhase);
    }
}

void test_sig_waveform_sinePhaseTriangle(void) {
    // Like a sine, the triangle starts at zero and rises to its peak
    // a quarter of the way through the cycle.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
        sig_waveform_sinePhaseTriangle(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f,
        sig_waveform_sinePhaseTriangle(sig_PI / 4.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f,
        sig_waveform_sinePhaseTriangle(sig_PI / 2.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
        sig_waveform_sinePhaseTriangle(sig_PI));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, -1.0f,
        sig_waveform_sinePhaseTriangle(sig_PI * 1.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, -0.5f,
        sig_waveform_sinePhaseTriangle(sig_PI * 1.75f));
}

void test_sig_waveform_sinePhaseSaw(void) {
    // Like a sine, the saw starts at zero and rises. It reaches its peak
    // halfway through the cycle, drops to its trough, and rises back to
    // zero.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, sig_waveform_sinePhaseSaw(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f,
        sig_waveform_sinePhaseSaw(sig_PI / 2.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f,
        sig_waveform_sinePhaseSaw(sig_PI - 0.001f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.0f,
        sig_waveform_sinePhaseSaw(sig_PI + 0.001f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, -0.5f,
        sig_waveform_sinePhaseSaw(sig_PI * 1.5f));
}

void test_sig_waveform_sinePhaseHarmonicGain(void) {
    // The triangle contains only odd harmonics, falling by 1/k^2,
    // with alternating signs.
    TEST_ASSERT_EQUAL_FLOAT(8.0f / (sig_PI * sig_PI),
        sig_waveform_sinePhaseTriangleHarmonicGain(1));
    TEST_ASSERT_EQUAL_FLOAT(0.0f,
        sig_waveform_sinePhaseTriangleHarmonicGain(2));
    TEST_ASSERT_EQUAL_FLOAT(-8.0f / (9.0f * sig_PI * sig_PI),
        sig_waveform_sinePhaseTriangleHarmonicGain(3));
    TEST_ASSERT_EQUAL_FLOAT(8.0f / (25.0f * sig_PI * sig_PI),
        sig_waveform_sinePhaseTriangleHarmonicGain(5));

    // The saw contains every harmonic, falling by 1/k,
    // with alternating signs.
    TEST_ASSERT_EQUAL_FLOAT(2.0f / sig_PI,
        sig_waveform_sinePhaseSawHarmonicGain(1));
    TEST_ASSERT_EQUAL_FLOAT(-1.0f / sig_PI,
        sig_waveform_sinePhaseSawHarmonicGain(2));
    TEST_ASSERT_EQUAL_FLOAT(2.0f / (3.0f * sig_PI),
        sig_waveform_sinePhaseSawHarmonicGain(3));
}

void test_sig_SINE_PHASE_WAVEFORMS(void) {
    size_t numSamples = 4096;

    // Every waveform's fundamental is in phase with a sine. Its cosine
    // component is zero and its sine component is positive, both in the
    // waveform's shape and in its harmonic series.
    for (size_t i = 0; i < sig_StandardWaveform_COUNT; i++) {
        const struct sig_WaveformShape* shape = &sig_SINE_PHASE_WAVEFORMS[i];
        double sineComponent = 0.0;
        double cosineComponent = 0.0;
        for (size_t n = 0; n < numSamples; n++) {
            float phase = sig_TWOPI * (float) n / (float) numSamples;
            float sample = shape->generate(phase);
            sineComponent += sample * sinf(phase);
            cosineComponent += sample * cosf(phase);
        }

        TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f,
            (float) (2.0 * cosineComponent / (double) numSamples));
        TEST_ASSERT_TRUE(sineComponent > 0.0);
        TEST_ASSERT_TRUE(shape->harmonic(1) > 0.0f);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, shape->harmonicPhase);
    }
}

struct sig_Buffer* newSineTable(size_t length) {
    struct sig_Buffer* sineTable = sig_Buffer_new(&allocator, length);
    sig_Buffer_fillWithWaveform(sineTable, sig_waveform_sine,
        (float) length, 0.0f, 1.0f);

    return sineTable;
}

bool isNearDiscontinuity(float phase) {
    float distance = 0.05f;
    return phase < distance || phase > 1.0f - distance ||
        fabsf(phase - 0.5f) < distance;
}

// Fills a table with each shape's harmonic series, and asserts that it
// matches the shape's naive waveform.
void testAssertHarmonicsMatchWaveforms(const struct sig_WaveformShape* shapes,
    size_t numShapes) {
    size_t length = 4096;
    struct sig_FFT* fft = sig_FFT_new(&allocator, length);
    float_array_ptr imaginary = sig_samples_new(&allocator, length);
    struct sig_Buffer* table = sig_Buffer_new(&allocator, length);

    for (size_t waveform = 0; waveform < numShapes; waveform++) {
        sig_Buffer_fillWithHarmonics(table, fft, imaginary, &shapes[waveform],
            500, sig_HarmonicTaper_NONE);

        // Away from discontinuities, where band-limited waveforms ring,
        // the table should match the naive waveform.
        for (size_t i = 0; i < length; i++) {
            float phase = (float) i / (float) length;
            if (isNearDiscontinuity(phase)) {
                continue;
            }

            float expected = shapes[waveform].generate(phase * sig_TWOPI);
            TEST_ASSERT_FLOAT_WITHIN(0.01f, expected,
                FLOAT_ARRAY(table->samples)[i]);
        }
    }

    sig_Buffer_destroy(&allocator, table);
    allocator.impl->free(&allocator, imaginary);
    sig_FFT_destroy(&allocator, fft);
}

void test_sig_Buffer_fillWithHarmonics_matchesWaveforms(void) {
    testAssertHarmonicsMatchWaveforms(sig_STANDARD_WAVEFORMS,
        sig_StandardWaveform_COUNT);
}

void test_sig_Buffer_fillWithHarmonics_matchesSinePhaseWaveforms(void) {
    testAssertHarmonicsMatchWaveforms(sig_SINE_PHASE_WAVEFORMS,
        sig_StandardWaveform_COUNT);
}

float maxSample(float* samples, size_t length) {
    float max = samples[0];
    for (size_t i = 1; i < length; i++) {
        if (samples[i] > max) {
            max = samples[i];
        }
    }

    return max;
}

void test_sig_Buffer_fillWithHarmonics_lanczosLimitsOvershoot(void) {
    size_t length = 4096;
    struct sig_FFT* fft = sig_FFT_new(&allocator, length);
    float_array_ptr imaginary = sig_samples_new(&allocator, length);
    struct sig_Buffer* table = sig_Buffer_new(&allocator, length);

    // Without a taper, a square overshoots each jump of 2.0
    // by about 9%, peaking near 1.18.
    sig_Buffer_fillWithHarmonics(table, fft, imaginary,
        &sig_STANDARD_WAVEFORMS[sig_StandardWaveform_SQUARE], 64,
        sig_HarmonicTaper_NONE);
    TEST_ASSERT_TRUE(maxSample(table->samples, length) > 1.15f);

    // Lanczos sigma factors reduce the overshoot to about 1.2%.
    sig_Buffer_fillWithHarmonics(table, fft, imaginary,
        &sig_STANDARD_WAVEFORMS[sig_StandardWaveform_SQUARE], 64,
        sig_HarmonicTaper_LANCZOS);
    TEST_ASSERT_TRUE(maxSample(table->samples, length) < 1.03f);

    sig_Buffer_destroy(&allocator, table);
    allocator.impl->free(&allocator, imaginary);
    sig_FFT_destroy(&allocator, fft);
}

// Measures the amplitude of one harmonic of a single-cycle table
// using a single bin of a discrete Fourier transform.
// Sums are accumulated in double precision so that the measurement
// can resolve components far quieter than the signal.
double harmonicAmplitude(float* samples, size_t length, size_t harmonic) {
    double re = 0.0;
    double im = 0.0;
    for (size_t i = 0; i < length; i++) {
        double angle = 6.283185307179586 *
            (double) ((harmonic * i) % length) / (double) length;
        re += samples[i] * cos(angle);
        im += samples[i] * sin(angle);
    }

    return 2.0 * sqrt(re * re + im * im) / (double) length;
}

void test_sig_Buffer_fillWithHarmonics_isBandlimited(void) {
    size_t length = 4096;
    size_t numHarmonics = 20;
    struct sig_FFT* fft = sig_FFT_new(&allocator, length);
    float_array_ptr imaginary = sig_samples_new(&allocator, length);
    struct sig_Buffer* table = sig_Buffer_new(&allocator, length);
    enum sig_HarmonicTaper tapers[2] = {
        sig_HarmonicTaper_NONE,
        sig_HarmonicTaper_LANCZOS
    };

    for (size_t i = 0; i < 2; i++) {
        sig_Buffer_fillWithHarmonics(table, fft, imaginary,
            &sig_STANDARD_WAVEFORMS[sig_StandardWaveform_SAW], numHarmonics,
            tapers[i]);

        // The highest harmonic is present, weighted by the taper.
        float expected = 2.0f / (sig_PI * numHarmonics) *
            sig_waveform_harmonicTaper(tapers[i], numHarmonics,
                numHarmonics);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, expected,
            harmonicAmplitude(table->samples, length, numHarmonics));

        for (size_t harmonic = numHarmonics + 1; harmonic < length / 2;
            harmonic++) {
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
                harmonicAmplitude(table->samples, length, harmonic));
        }
    }

    sig_Buffer_destroy(&allocator, table);
    allocator.impl->free(&allocator, imaginary);
    sig_FFT_destroy(&allocator, fft);
}

// Creates a bank whose tables each contain a single value,
// ten times their index, so the table being read is easy to identify.
struct sig_WavetableBank* newConstantWavetableBank(size_t numTables) {
    struct sig_WavetableBank* bank = sig_WavetableBank_new(&allocator,
        numTables, 16);
    for (size_t i = 0; i < numTables; i++) {
        sig_Buffer_fillWithValue(bank->waves[i], (float) i * 10.0f);
    }

    return bank;
}

void test_sig_WavetableBank_readLinearAtPhase_wrapsTableIndex(void) {
    struct sig_WavetableBank* bank = newConstantWavetableBank(4);
    float phase = 0.3f;

    // Indices within the bank crossfade between neighbouring tables.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 15.0f,
        sig_WavetableBank_readLinearAtPhase(bank, 1.5f, phase));

    // The last table crossfades back into the first.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 15.0f,
        sig_WavetableBank_readLinearAtPhase(bank, 3.5f, phase));

    // Indices beyond the end wrap around to the start.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
        sig_WavetableBank_readLinearAtPhase(bank, 4.0f, phase));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 12.5f,
        sig_WavetableBank_readLinearAtPhase(bank, 5.25f, phase));

    // Negative indices wrap around to the end.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 30.0f,
        sig_WavetableBank_readLinearAtPhase(bank, -1.0f, phase));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 15.0f,
        sig_WavetableBank_readLinearAtPhase(bank, -0.5f, phase));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 22.5f,
        sig_WavetableBank_readLinearAtPhase(bank, -4.75f, phase));

    // Tiny negative indices, which round to the length of the bank
    // when wrapped, read the first table.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
        sig_WavetableBank_readLinearAtPhase(bank, -1.0e-9f, phase));

    sig_WavetableBank_destroy(&allocator, bank);
}

void test_sig_osc_WavetableBank_tableIndexIsCyclic(void) {
    struct sig_WavetableBank* bank = newConstantWavetableBank(4);
    struct sig_osc_WavetableBank osc;
    sig_osc_WavetableBank_init(&osc, bank);
    float eoc = 0.0f;

    // Each table occupies an equal share of one cycle of the index,
    // and the last table crossfades back into the first.
    float tableIndices[9] = {
        0.0f, 0.25f, 0.5f, 0.75f, 0.875f, 1.0f, 1.25f, -0.25f, -0.125f
    };
    float expected[9] = {
        0.0f, 10.0f, 20.0f, 30.0f, 15.0f, 0.0f, 10.0f, 30.0f, 15.0f
    };

    for (size_t i = 0; i < 9; i++) {
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, expected[i],
            sig_osc_WavetableBank_generate(&osc, 440.0f, 0.0f,
                tableIndices[i], 44100.0f, &eoc));
    }

    sig_WavetableBank_destroy(&allocator, bank);
}

void test_sig_osc_WavetableBank_tableIndex(void) {
    struct sig_WavetableBank* bank = newConstantWavetableBank(4);
    struct sig_osc_WavetableBank osc;
    sig_osc_WavetableBank_init(&osc, bank);
    float eoc = 0.0f;

    // Each table's index reads exactly that table,
    // including the last, which does not wrap back to the first.
    for (size_t table = 0; table < 4; table++) {
        float tableIndex = sig_osc_WavetableBank_tableIndex(table, 4);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, (float) table * 10.0f,
            sig_osc_WavetableBank_generate(&osc, 440.0f, 0.0f, tableIndex,
                44100.0f, &eoc));
    }

    sig_WavetableBank_destroy(&allocator, bank);
}

size_t highestHarmonic(float* samples, size_t length) {
    size_t highest = 0;
    for (size_t harmonic = 1; harmonic < length / 2; harmonic++) {
        if (harmonicAmplitude(samples, length, harmonic) > 0.0001f) {
            highest = harmonic;
        }
    }

    return highest;
}

struct sig_AdditiveWavetableBank* newStandardWavetableBank(size_t numLevels,
    size_t tableLength, float lowestFrequency, float maxHarmonicFrequency,
    enum sig_HarmonicTaper taper) {
    return sig_AdditiveWavetableBank_new(&allocator, sig_STANDARD_WAVEFORMS,
        sig_StandardWaveform_COUNT, numLevels, tableLength, lowestFrequency,
        maxHarmonicFrequency, taper);
}

void test_sig_AdditiveWavetableBank_harmonicsStayBelowMax(void) {
    size_t numLevels = 6;
    size_t tableLength = 2048;
    float lowestFrequency = 20.0f;
    float maxHarmonicFrequency = 2000.0f;
    struct sig_AdditiveWavetableBank* bank =
        newStandardWavetableBank(numLevels, tableLength,
            lowestFrequency, maxHarmonicFrequency, sig_HarmonicTaper_NONE);

    for (size_t level = 0; level < numLevels; level++) {
        float topOfOctave = lowestFrequency * powf(2.0f, (float) level + 1);
        size_t expectedHarmonics = (size_t) (maxHarmonicFrequency /
            topOfOctave);
        if (expectedHarmonics < 1) {
            expectedHarmonics = 1;
        }

        // Each level contains as many harmonics as fit below the maximum
        // at the top of its octave, and always at least the fundamental.
        struct sig_Buffer* saw =
            bank->levels[level]->waves[sig_StandardWaveform_SAW];
        TEST_ASSERT_EQUAL_size_t(expectedHarmonics,
            highestHarmonic(saw->samples, tableLength));

        struct sig_Buffer* sine =
            bank->levels[level]->waves[sig_StandardWaveform_SINE];
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f,
            harmonicAmplitude(sine->samples, tableLength, 1));
        TEST_ASSERT_EQUAL_size_t(1,
            highestHarmonic(sine->samples, tableLength));
    }

    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

// A custom shape that contains only its second harmonic.
float secondHarmonicGenerate(float phase) {
    return sinf(2.0f * phase);
}

float secondHarmonicGain(size_t harmonic) {
    return harmonic == 2 ? 1.0f : 0.0f;
}

void test_sig_AdditiveWavetableBank_customShapes(void) {
    size_t numLevels = 4;
    size_t tableLength = 2048;
    struct sig_WaveformShape shapes[2] = {
        sig_STANDARD_WAVEFORMS[sig_StandardWaveform_SINE],
        {
            .generate = secondHarmonicGenerate,
            .harmonic = secondHarmonicGain,
            .harmonicPhase = 0.0f
        }
    };
    struct sig_AdditiveWavetableBank* bank = sig_AdditiveWavetableBank_new(
        &allocator, shapes, 2, numLevels, tableLength, 20.0f, 2000.0f,
        sig_HarmonicTaper_NONE);

    // Each level contains one table for each shape.
    for (size_t level = 0; level < numLevels; level++) {
        TEST_ASSERT_EQUAL_size_t(2, bank->levels[level]->length);
    }

    // The custom shape's table contains only its second harmonic.
    struct sig_Buffer* custom = bank->levels[0]->waves[1];
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f,
        harmonicAmplitude(custom->samples, tableLength, 2));
    TEST_ASSERT_EQUAL_size_t(2, highestHarmonic(custom->samples, tableLength));

    // The oscillator divides each cycle of its table index between the
    // bank's shapes, so an index of 0.5 reads the custom shape.
    for (size_t i = 0; i < 16; i++) {
        float phase = (float) i / 16.0f;
        struct sig_osc_AdditiveWavetableBank osc;
        sig_osc_AdditiveWavetableBank_init(&osc, bank);
        float eoc = 0.0f;
        float sample = sig_osc_AdditiveWavetableBank_generate(&osc, 100.0f,
            phase, 0.5f, 44100.0f, &eoc);
        TEST_ASSERT_FLOAT_WITHIN(0.001f, secondHarmonicGenerate(
            phase * sig_TWOPI), sample);
    }

    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

void test_sig_AdditiveWavetableBank_tapersEachLevel(void) {
    size_t numLevels = 6;
    size_t tableLength = 2048;
    struct sig_AdditiveWavetableBank* untapered =
        newStandardWavetableBank(numLevels, tableLength,
            20.0f, 2000.0f, sig_HarmonicTaper_NONE);
    struct sig_AdditiveWavetableBank* tapered =
        newStandardWavetableBank(numLevels, tableLength,
            20.0f, 2000.0f, sig_HarmonicTaper_LANCZOS);

    // The lowest level's untapered square shows the full overshoot.
    TEST_ASSERT_TRUE(maxSample(
        untapered->levels[0]->waves[sig_StandardWaveform_SQUARE]->samples,
        tableLength) > 1.15f);

    // Every tapered level limits it. A level with only a few harmonics
    // is mostly its fundamental, which keeps its full gain, so it peaks
    // at no more than the fundamental's amplitude, 4/pi.
    for (size_t level = 0; level < numLevels; level++) {
        float peak = maxSample(
            tapered->levels[level]->waves[sig_StandardWaveform_SQUARE]->samples,
            tableLength);
        if (sig_AdditiveWavetableBank_numHarmonics(tapered, level) >= 20) {
            TEST_ASSERT_TRUE(peak < 1.03f);
        }
        TEST_ASSERT_TRUE(peak <= 4.0f / sig_PI + 0.001f);
    }

    sig_AdditiveWavetableBank_destroy(&allocator, tapered);
    sig_AdditiveWavetableBank_destroy(&allocator, untapered);
}

void test_sig_AdditiveWavetableBank_levelForFrequency(void) {
    struct sig_AdditiveWavetableBank* bank =
        newStandardWavetableBank(6, 256, 20.0f, 2000.0f,
            sig_HarmonicTaper_NONE);

    // Each octave above the lowest frequency is one level higher.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
        sig_AdditiveWavetableBank_levelForFrequency(bank, 20.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f,
        sig_AdditiveWavetableBank_levelForFrequency(bank, 40.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.0f,
        sig_AdditiveWavetableBank_levelForFrequency(bank, 160.0f));

    // Frequencies within an octave fall between levels.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 2.5f,
        sig_AdditiveWavetableBank_levelForFrequency(bank,
            20.0f * powf(2.0f, 2.5f)));

    // Negative frequencies use the level of their magnitude.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f,
        sig_AdditiveWavetableBank_levelForFrequency(bank, -40.0f));

    // Frequencies outside the bank use the nearest level.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
        sig_AdditiveWavetableBank_levelForFrequency(bank, 5.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
        sig_AdditiveWavetableBank_levelForFrequency(bank, 0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 5.0f,
        sig_AdditiveWavetableBank_levelForFrequency(bank, 20000.0f));

    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

void test_sig_AdditiveWavetableBank_readLinearAtPhase(void) {
    struct sig_AdditiveWavetableBank* bank =
        newStandardWavetableBank(6, 2048, 20.0f, 2000.0f,
            sig_HarmonicTaper_NONE);
    float sawIdx = (float) sig_StandardWaveform_SAW;

    for (size_t i = 0; i < 64; i++) {
        float phase = (float) i / 64.0f;
        float levelTwo = sig_WavetableBank_readLinearAtPhase(
            bank->levels[2], sawIdx, phase);
        float levelThree = sig_WavetableBank_readLinearAtPhase(
            bank->levels[3], sawIdx, phase);
        float levelFive = sig_WavetableBank_readLinearAtPhase(
            bank->levels[5], sawIdx, phase);

        // Whole levels read from a single octave's tables.
        TEST_ASSERT_FLOAT_WITHIN(0.00001f, levelTwo,
            sig_AdditiveWavetableBank_readLinearAtPhase(bank, 2.0f,
                sawIdx, phase));

        // Fractional levels crossfade between neighbouring octaves.
        TEST_ASSERT_FLOAT_WITHIN(0.00001f,
            levelTwo * 0.75f + levelThree * 0.25f,
            sig_AdditiveWavetableBank_readLinearAtPhase(bank, 2.25f,
                sawIdx, phase));

        // The highest level has no neighbour above it.
        TEST_ASSERT_FLOAT_WITHIN(0.00001f, levelFive,
            sig_AdditiveWavetableBank_readLinearAtPhase(bank, 5.0f,
                sawIdx, phase));
    }

    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

void test_sig_osc_AdditiveWavetableBank_sine(void) {
    float sampleRate = 44100.0f;
    float frequency = 440.0f;
    struct sig_AdditiveWavetableBank* bank =
        newStandardWavetableBank(10, 4096, 20.0f,
            20000.0f, sig_HarmonicTaper_NONE);
    struct sig_osc_AdditiveWavetableBank osc;
    sig_osc_AdditiveWavetableBank_init(&osc, bank);

    float eoc = 0.0f;
    for (size_t i = 0; i < 256; i++) {
        float expected = sinf(sig_TWOPI * frequency * (float) i / sampleRate);
        float actual = sig_osc_AdditiveWavetableBank_generate(&osc,
            frequency, 0.0f, 0.0f, sampleRate, &eoc);
        TEST_ASSERT_FLOAT_WITHIN(0.001f, expected, actual);
    }

    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

// Measures the power of a periodic signal that is not at its harmonics,
// relative to the power at its harmonics, in dB.
// The signal must contain exactly numCycles cycles of its fundamental.
float aliasedPowerRatio(float* samples, size_t length, size_t numCycles) {
    double totalPower = 0.0;
    for (size_t i = 0; i < length; i++) {
        totalPower += (double) samples[i] * (double) samples[i];
    }
    totalPower /= (double) length;

    double harmonicPower = 0.0;
    for (size_t harmonic = 1; harmonic * numCycles < length / 2;
        harmonic++) {
        double amplitude = harmonicAmplitude(samples, length,
            harmonic * numCycles);
        harmonicPower += amplitude * amplitude / 2.0;
    }

    return (float) (10.0 * log10((totalPower - harmonicPower) /
        harmonicPower));
}

void test_sig_osc_AdditiveWavetableBank_aliasesLess(void) {
    float sampleRate = 44100.0f;
    size_t length = 4410;
    size_t numCycles = 123;
    float frequency = (float) numCycles * sampleRate / (float) length;
    float* bandlimited = (float*) allocator.impl->malloc(&allocator,
        sizeof(float) * length);
    float* naive = (float*) allocator.impl->malloc(&allocator,
        sizeof(float) * length);
    float sawIdx = sig_osc_WavetableBank_tableIndex(sig_StandardWaveform_SAW,
        sig_StandardWaveform_COUNT);
    float eoc = 0.0f;

    struct sig_AdditiveWavetableBank* bank =
        newStandardWavetableBank(10, 2048, 20.0f,
            20000.0f, sig_HarmonicTaper_NONE);
    struct sig_osc_AdditiveWavetableBank bandlimitedOsc;
    sig_osc_AdditiveWavetableBank_init(&bandlimitedOsc, bank);

    struct sig_WavetableBank* naiveBank = sig_WavetableBank_new(&allocator,
        sig_StandardWaveform_COUNT, 2048);
    for (size_t waveform = 0; waveform < sig_StandardWaveform_COUNT; waveform++) {
        sig_Buffer_fillWithWaveform(naiveBank->waves[waveform],
            sig_STANDARD_WAVEFORMS[waveform].generate, 2048.0f, 0.0f, 1.0f);
    }
    struct sig_osc_WavetableBank naiveOsc;
    sig_osc_WavetableBank_init(&naiveOsc, naiveBank);

    for (size_t i = 0; i < length; i++) {
        bandlimited[i] = sig_osc_AdditiveWavetableBank_generate(
            &bandlimitedOsc, frequency, 0.0f, sawIdx, sampleRate, &eoc);
        naive[i] = sig_osc_WavetableBank_generate(&naiveOsc, frequency,
            0.0f, sawIdx, sampleRate, &eoc);
    }

    float bandlimitedAliasing = aliasedPowerRatio(bandlimited, length,
        numCycles);
    float naiveAliasing = aliasedPowerRatio(naive, length, numCycles);
    char message[96];
    snprintf(message, sizeof(message),
        "Band-limited aliasing was %.1f dB; naive aliasing was %.1f dB.",
        bandlimitedAliasing, naiveAliasing);
    TEST_ASSERT_TRUE_MESSAGE(bandlimitedAliasing < -60.0f, message);
    TEST_ASSERT_TRUE_MESSAGE(bandlimitedAliasing < naiveAliasing - 30.0f,
        message);

    sig_WavetableBank_destroy(&allocator, naiveBank);
    sig_AdditiveWavetableBank_destroy(&allocator, bank);
    allocator.impl->free(&allocator, naive);
    allocator.impl->free(&allocator, bandlimited);
}

// Generates the first sample of a new band-limited oscillator,
// read at the specified phase.
float firstBandlimitedSample(struct sig_AdditiveWavetableBank* bank,
    float tableIndex, float phase) {
    struct sig_osc_AdditiveWavetableBank osc;
    sig_osc_AdditiveWavetableBank_init(&osc, bank);
    float eoc = 0.0f;

    return sig_osc_AdditiveWavetableBank_generate(&osc, 100.0f, phase,
        tableIndex, 44100.0f, &eoc);
}

void test_sig_osc_AdditiveWavetableBank_tableIndexIsCyclic(void) {
    struct sig_AdditiveWavetableBank* bank =
        newStandardWavetableBank(6, 2048, 20.0f, 2000.0f,
            sig_HarmonicTaper_NONE);
    float tableIndices[3] = {0.0f, 0.3f, 0.9f};

    for (size_t i = 0; i < 64; i++) {
        float phase = (float) i / 64.0f;

        // Shifting the index by whole cycles reads the same waveform.
        for (size_t j = 0; j < 3; j++) {
            float sample = firstBandlimitedSample(bank, tableIndices[j],
                phase);
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, sample,
                firstBandlimitedSample(bank, tableIndices[j] + 1.0f, phase));
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, sample,
                firstBandlimitedSample(bank, tableIndices[j] - 1.0f, phase));
        }

        // The last waveform crossfades back into the first.
        float reverseSaw = firstBandlimitedSample(bank,
            sig_osc_WavetableBank_tableIndex(sig_StandardWaveform_REVERSE_SAW,
                sig_StandardWaveform_COUNT), phase);
        float sine = firstBandlimitedSample(bank, 0.0f, phase);
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, (reverseSaw + sine) / 2.0f,
            firstBandlimitedSample(bank, 0.9f, phase));
    }

    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

void test_sig_dsp_AdditiveWavetableBankOscillator(void) {
    float sampleRate = audioSettings->sampleRate;
    size_t blockSize = audioSettings->blockSize;
    float frequency = 440.0f;
    struct sig_AdditiveWavetableBank* bank =
        newStandardWavetableBank(10, 4096, 20.0f,
            20000.0f, sig_HarmonicTaper_NONE);

    struct sig_dsp_AdditiveWavetableBankOscillator* osc =
        sig_dsp_AdditiveWavetableBankOscillator_new(&allocator, context);
    osc->wavetables = bank;
    osc->inputs.freq = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, frequency);
    osc->inputs.mul = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.5f);
    osc->inputs.add = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.25f);

    // The output is a scaled and offset sine,
    // whose phase continues across blocks.
    for (size_t block = 0; block < 2; block++) {
        osc->signal.generate(osc);
        for (size_t i = 0; i < blockSize; i++) {
            float t = (float) (block * blockSize + i) / sampleRate;
            float expected = 0.5f * sinf(sig_TWOPI * frequency * t) + 0.25f;
            TEST_ASSERT_FLOAT_WITHIN(0.001f, expected,
                FLOAT_ARRAY(osc->outputs.main)[i]);
        }
    }

    sig_dsp_AdditiveWavetableBankOscillator_destroy(&allocator, osc);
    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

void test_sig_Buffer(void) {
    size_t len = 1024;
    struct sig_Buffer* b = sig_Buffer_new(&allocator, len);
    TEST_ASSERT_EQUAL_size_t_MESSAGE(len, b->length,
        "The buffer should be initialized with the correct length");

    float fillVal = 1.0f;
    sig_Buffer_fillWithValue(b, fillVal);
    testAssertBufferContainsValueOnly(&allocator, fillVal, b->samples,
        b->length);

    sig_Buffer_fillWithSilence(b);
    testAssertBufferContainsValueOnly(&allocator, 0.0f, b->samples, b->length);

    sig_Buffer_destroy(&allocator, b);
}

float fillWithIndices(size_t i, float_array_ptr array) {
    return (float) i;
}

void test_sig_BufferView(void) {
    size_t superLen = 1024;
    size_t viewLen = 256;
    size_t viewStart = 256;

    struct sig_Buffer* b = sig_Buffer_new(&allocator, superLen);
    sig_Buffer_fill(b, fillWithIndices);

    struct sig_Buffer* expectedViewContents =
        sig_Buffer_new(&allocator, viewLen);
    for (size_t i = 0; i < expectedViewContents->length; i++) {
        FLOAT_ARRAY(expectedViewContents->samples)[i] =
            (float) i + viewStart;
    }

    struct sig_Buffer* view = sig_BufferView_new(&allocator,
        b, viewStart, viewLen);

    TEST_ASSERT_EQUAL_size_t_MESSAGE(viewLen, view->length,
        "The BufferView should have the correct length.");

    TEST_ASSERT_EQUAL_FLOAT_ARRAY_MESSAGE(
        expectedViewContents->samples,
        view->samples,
        viewLen,
        "The subarray should contain the correct values.");
}

void test_sig_linearXFade(void) {
    float left = 0.66f;
    float right = 0.45f;
    float mix = -1.0f;
    float expected = left;
    float actual = sig_linearXFade(left, right, mix);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected, actual, "Left signal only.");

    mix = 1.0f;
    expected = right;
    actual = sig_linearXFade(left, right, mix);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected, actual, "Right signal only.");

    mix = 0.0f;
    expected = 0.555f;
    actual = sig_linearXFade(left, right, mix);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected, actual,
        "Both signals mixed evenly.");

    mix = -0.25f;
    expected = 0.58125f;
    actual = sig_linearXFade(left, right, mix);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected, actual,
        "25 percent to the left.");

    left = 0.001f;
    right = 0.01f;
    mix = -0.5f;
    expected = 0.00325f;
    actual = sig_linearXFade(left, right, mix);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected, actual,
        "50 percent to the left.");

    mix = 0.5f;
    expected = 0.00775f;
    actual = sig_linearXFade(left, right, mix);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected, actual,
        "50 percent to the left.");
}

void test_sig_DelayLine_linearComb_fractionalDelay(void) {
    // A delay of 1.5 samples with no feedback should split an impulse
    // evenly between the one- and two-sample delayed outputs.
    struct sig_DelayLine* delayLine = sig_DelayLine_new(&allocator, 8);
    float impulse[5] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float expected[5] = {0.0f, 0.5f, 0.5f, 0.0f, 0.0f};
    float actual[5];

    for (size_t i = 0; i < 5; i++) {
        actual[i] = sig_DelayLine_linearComb(delayLine, impulse[i],
            1.5f, 0.0f);
    }

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, actual, 5);

    sig_DelayLine_destroy(&allocator, delayLine);
}

void test_sig_DelayLine_linearComb_feedback(void) {
    // A two-sample delay with a feedback gain of 0.5 should repeat
    // an impulse every two samples, halving it on each repetition.
    struct sig_DelayLine* delayLine = sig_DelayLine_new(&allocator, 8);
    float impulse[7] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float expected[7] = {0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.0f, 0.25f};
    float actual[7];

    for (size_t i = 0; i < 7; i++) {
        actual[i] = sig_DelayLine_linearComb(delayLine, impulse[i],
            2.0f, 0.5f);
    }

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, actual, 7);

    sig_DelayLine_destroy(&allocator, delayLine);
}

void test_sig_dsp_Value(void) {
    struct sig_dsp_Value* value = sig_dsp_Value_new(&allocator, context);
    value->parameters.value = 123.45f;

    // Output should contain the value parameter.
    value->signal.generate(value);
    testAssertBufferContainsValueOnly(&allocator,
        123.45f, value->outputs.main, audioSettings->blockSize);

    // Output should contain the updated value parameter.
    value->parameters.value = 1.111f;
    value->signal.generate(value);
    testAssertBufferContainsValueOnly(&allocator,
        1.111f, value->outputs.main, audioSettings->blockSize);

    // The lastSample member should have been updated.
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(FLOAT_EPSILON,
        1.111f, value->lastSample,
        "lastSample should have been updated.");

    // After multiple calls to generate(),
    // the output should continue to contain the value parameter.
    value->signal.generate(value);
    value->signal.generate(value);
    testAssertBufferContainsValueOnly(&allocator,
        1.111f, value->outputs.main, audioSettings->blockSize);

    sig_dsp_Value_destroy(&allocator, value);
}

void test_sig_dsp_ConstantValue(void) {
    struct sig_dsp_ConstantValue* constVal = sig_dsp_ConstantValue_new(
        &allocator, context, 42.0f);

    // Output should contain the value, even prior to being evaluated.
    testAssertBufferContainsValueOnly(&allocator, 42.0f,
        constVal->outputs.main, audioSettings->blockSize);

    // The output should not change.
    constVal->signal.generate(constVal);
    testAssertBufferContainsValueOnly(&allocator, 42.0f,
        constVal->outputs.main, audioSettings->blockSize);
}

void test_sig_dsp_TimedTriggerCounter(void) {
    float halfBlockSecs = (audioSettings->blockSize / 2) /
        audioSettings->sampleRate;

    // A trigger right at the beginning of the input buffer.
    float* source = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.0f);
    source[0] = 1.0;

    struct sig_dsp_TimedTriggerCounter* counter = sig_dsp_TimedTriggerCounter_new(&allocator, context);
    counter->inputs.source = source;
    counter->inputs.duration = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, halfBlockSecs);
    counter->inputs.count = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 1.0f);

    // The output should contain a single trigger half block size.
    // (i.e. 24 samples after we received the
    // rising edge of the input trigger)
    counter->signal.generate(counter);
    float* expected = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.0f);
    expected[23] = 1.0f;
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, counter->outputs.main,
        audioSettings->blockSize);

    // The first trigger can happen later after the
    // "duration" has elapsed and it will be recognized
    // because it shouldn't start counting until a trigger is
    // received.
    source[0] = 0.0f;
    source[24] = 1.0f;
    counter->signal.generate(counter);
    sig_fillWithSilence(expected, audioSettings->blockSize);
    expected[47] = 1.0f;
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, counter->outputs.main,
        audioSettings->blockSize);

    // When the input contains two triggers in the
    // specified duration, but we're looking for one,
    // no trigger should be output.
    sig_fillWithSilence(source, audioSettings->blockSize);
    source[1] = 1.0f;
    source[20] = 1.0f;
    counter->signal.generate(counter);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(
        context->silence->outputs.main,
        counter->outputs.main,
        audioSettings->blockSize);

    // When we're looking for two triggers and we get two triggers,
    // one trigger should be fired.
    sig_fillWithValue(counter->inputs.count, audioSettings->blockSize,
        2.0f);
    counter->signal.generate(counter);
    sig_fillWithSilence(expected, audioSettings->blockSize);
    expected[24] = 1.0f;
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, counter->outputs.main,
        audioSettings->blockSize);

    // When we're looking for two triggers and
    // the second one comes too late,
    // No triggers should be fired.
    sig_fillWithSilence(source, audioSettings->blockSize);
    source[0] = 1.0f;
    source[25] = 1.0f;
    counter->signal.generate(counter);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(
        context->silence->outputs.main,
        counter->outputs.main,
        audioSettings->blockSize);
}

void test_sig_dsp_Mul(void) {
    struct sig_dsp_BinaryOp* gain = sig_dsp_Mul_new(&allocator, context);
    gain->inputs.left = sig_AudioBlock_newWithValue(&allocator, audioSettings,
        0.5f);
    gain->inputs.right = sig_AudioBlock_newWithValue(&allocator, audioSettings,
        440.0f);

    gain->signal.generate(gain);
    testAssertBufferContainsValueOnly(&allocator,
        220.0f, gain->outputs.main, audioSettings->blockSize);

    sig_fillWithValue(gain->inputs.left, audioSettings->blockSize, 0.0f);
    gain->signal.generate(gain);
    testAssertBufferContainsValueOnly(&allocator,
        0.0f, gain->outputs.main, audioSettings->blockSize);

    sig_fillWithValue(gain->inputs.left, audioSettings->blockSize, 2.0f);
    gain->signal.generate(gain);
    testAssertBufferContainsValueOnly(&allocator,
        880.0f, gain->outputs.main, audioSettings->blockSize);

    sig_fillWithValue(gain->inputs.left, audioSettings->blockSize, -1.0f);
    gain->signal.generate(gain);
    testAssertBufferContainsValueOnly(&allocator,
        -440.0f, gain->outputs.main, audioSettings->blockSize);

    sig_AudioBlock_destroy(&allocator, gain->inputs.left);
    sig_AudioBlock_destroy(&allocator, gain->inputs.right);
    sig_dsp_Mul_destroy(&allocator, gain);
}

void createOscInputs(struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator* osc,
    float freq, float phaseOffset, float mul, float add) {

    osc->inputs.freq = sig_AudioBlock_newWithValue(allocator,
        osc->signal.audioSettings, freq);
    osc->inputs.phaseOffset = sig_AudioBlock_newWithValue(allocator,
        osc->signal.audioSettings, phaseOffset);
    osc->inputs.mul = sig_AudioBlock_newWithValue(allocator,
        osc->signal.audioSettings, mul);
    osc->inputs.add = sig_AudioBlock_newWithValue(allocator,
        osc->signal.audioSettings, add);
}

void destroyOscInputs(struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator_Inputs* sineInputs) {
    allocator->impl->free(allocator, sineInputs->freq);
    allocator->impl->free(allocator, sineInputs->phaseOffset);
    allocator->impl->free(allocator, sineInputs->mul);
    allocator->impl->free(allocator, sineInputs->add);
}

void test_sig_dsp_SineOscillator(void) {
    // Generated from this program:
    /*
        #include <stddef.h>
        #include <math.h>
        #include <stdio.h>

        const float PI = 3.14159265358979323846f;
        const float TWOPI = 2.0f * PI;

        int main(int argc, char *argv[]) {
            float sr = 44100.0f;
            float freq = 440.0f;
            float phaseStep = freq / sr * TWOPI;
            float phase = 0.0f;
            for (size_t i = 0; i < 64; i++) {
                float sample = sinf(phase);
                phase = phase + phaseStep;
                printf("%.8ff, ", sample);
            }
        }
    */
    float expected[BLOCK_SIZE] = {
        0.00000000f, 0.06264833f, 0.12505053f, 0.18696144f,
        0.24813786f, 0.30833939f, 0.36732957f, 0.42487663f,
        0.48075449f, 0.53474361f, 0.58663195f, 0.63621557f,
        0.68329972f, 0.72769940f, 0.76924020f, 0.80775887f,
        0.84310418f, 0.87513721f, 0.90373212f, 0.92877656f,
        0.95017213f, 0.96783477f, 0.98169512f, 0.99169868f,
        0.99780619f, 0.99999368f, 0.99825245f, 0.99258947f,
        0.98302686f, 0.96960229f, 0.95236850f, 0.93139309f,
        0.90675867f, 0.87856185f, 0.84691346f, 0.81193781f,
        0.77377236f, 0.73256701f, 0.68848366f, 0.64169550f,
        0.59238636f, 0.54074991f, 0.48698899f, 0.43131489f,
        0.37394631f, 0.31510863f, 0.25503296f, 0.19395538f,
        0.13211580f, 0.06975718f, 0.00712451f, -0.05553614f,
        -0.11797862f, -0.17995760f, -0.24122958f, -0.30155385f,
        -0.36069342f, -0.41841593f, -0.47449467f, -0.52870923f,
        -0.58084673f, -0.63070220f, -0.67807990f, -0.72279370f
    };
    struct sig_SignalContext* mono441kContext = sig_SignalContext_new(
        &allocator, &mono441kAudioSettings);

    struct sig_dsp_Oscillator* sine = sig_dsp_SineOscillator_new(&allocator,
        mono441kContext);
    createOscInputs(&allocator, sine, 440.0f, 0.0f, 1.0f, 0.0f);

    sine->signal.generate(sine);
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        TEST_ASSERT_FLOAT_WITHIN(0.00001,
            FLOAT_ARRAY(sine->outputs.main)[i],
            FLOAT_ARRAY(expected)[i]);
    }

    destroyOscInputs(&allocator, &sine->inputs);
    sig_dsp_SineOscillator_destroy(&allocator, sine);
    sig_SignalContext_destroy(&allocator, mono441kContext);
}

void test_test_sig_dsp_SineOscillator_isOffset(void) {
    float expected[BLOCK_SIZE] = {
        1.00000000f, 1.06264830f, 1.12505054f, 1.18696141f,
        1.24813783f, 1.30833936f, 1.36732960f, 1.42487669f,
        1.48075449f, 1.53474355f, 1.58663201f, 1.63621557f,
        1.68329978f, 1.72769940f, 1.76924014f, 1.80775881f,
        1.84310412f, 1.87513721f, 1.90373206f, 1.92877650f,
        1.95017219f, 1.96783471f, 1.98169518f, 1.99169874f,
        1.99780619f, 1.99999368f, 1.99825239f, 1.99258947f,
        1.98302686f, 1.96960235f, 1.95236850f, 1.93139315f,
        1.90675867f, 1.87856185f, 1.84691346f, 1.81193781f,
        1.77377236f, 1.73256707f, 1.68848372f, 1.64169550f,
        1.59238636f, 1.54074991f, 1.48698902f, 1.43131495f,
        1.37394631f, 1.31510866f, 1.25503302f, 1.19395542f,
        1.13211584f, 1.06975722f, 1.00712454f, 0.94446385f,
        0.88202137f, 0.82004237f, 0.75877041f, 0.69844615f,
        0.63930655f, 0.58158410f, 0.52550530f, 0.47129077f,
        0.41915327f, 0.36929780f, 0.32192010f, 0.27720630f
    };

    struct sig_SignalContext* mono441kContext = sig_SignalContext_new(
        &allocator, &mono441kAudioSettings);

    struct sig_dsp_Oscillator* sine = sig_dsp_SineOscillator_new(&allocator,
        mono441kContext);
    createOscInputs(&allocator, sine, 440.0f, 0.0f, 1.0f, 1.0f);

    sine->signal.generate(sine);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(
        expected,
        sine->outputs.main,
        sine->signal.audioSettings->blockSize);

    destroyOscInputs(&allocator, &sine->inputs);
    sig_dsp_SineOscillator_destroy(&allocator, sine);
    sig_SignalContext_destroy(&allocator, mono441kContext);
}

void test_sig_dsp_SineOscillator_accumulatesPhase(void) {
    struct sig_dsp_Oscillator* sine = sig_dsp_SineOscillator_new(&allocator,
        context);
    createOscInputs(&allocator, sine, 440.0f, 0.0f, 1.0f, 0.0f);

    // 440 Hz frequency at 48 KHz sample rate.
    float phaseStep = 0.00916667f;

    sine->signal.generate(sine);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
        0.000001,
        phaseStep * 48.0,
        sine->state.phaseAccumulator,
        "The phase accumulator should have been incremented for each sample in the block."
    );

    sine->signal.generate(sine);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
        0.000001,
        phaseStep * 96.0,
        sine->state.phaseAccumulator,
        "The phase accumulator should have continued to be incremented when generating a second block."
    );

    destroyOscInputs(&allocator, &sine->inputs);
    sig_dsp_SineOscillator_destroy(&allocator, sine);
}

void test_sig_dsp_SineOscillator_phaseWrapsAt2PI(void) {
    struct sig_dsp_Oscillator* sine = sig_dsp_SineOscillator_new(&allocator,
        context);
    createOscInputs(&allocator, sine, 440.0f, 0.0f, 1.0f, 0.0f);

    sine->signal.generate(sine);
    sine->signal.generate(sine);
    sine->signal.generate(sine);

    TEST_ASSERT_TRUE_MESSAGE(
        sine->state.phaseAccumulator <= 1.0 &&
        sine->state.phaseAccumulator >= 0.0,
        "The phase accumulator should wrap around when it is greater than 1.0 (i.e. a normalized equivalent to 2*PI)."
    );

    destroyOscInputs(&allocator, &sine->inputs);
    sig_dsp_SineOscillator_destroy(&allocator, sine);
}

void testDust(struct sig_dsp_Dust* dust,
    float min, float max, int16_t expectedNumDustPerBlock) {
    dust->signal.generate(dust);

    testAssertBufferNotSilent(&allocator, dust->outputs.main,
        audioSettings->blockSize);
    testAssertBufferValuesInRange(dust->outputs.main,
        audioSettings->blockSize, min, max);
    // Signal should generate a number of non-zero samples
    // within 0.5% of the expected number (since it's random).
    testAssertGeneratedSignalContainsApproxNumNonZeroSamples(
        &dust->signal, dust->outputs.main, expectedNumDustPerBlock,
        0.005, 1500);
}

void test_sig_dsp_Dust(void) {
    int32_t expectedNumDustPerBlock = 5;
    float density = (audioSettings->sampleRate /
        audioSettings->blockSize) * expectedNumDustPerBlock;

    struct sig_dsp_Dust* dust = sig_dsp_Dust_new(&allocator, context);
    dust->inputs.density = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, density);

    // Unipolar output.
    testDust(dust, 0.0f, 1.0f, expectedNumDustPerBlock);

    // Bipolar output.
    dust->parameters.bipolar = 1.0f;
    testDust(dust, -1.0f, 1.0f, expectedNumDustPerBlock);

    allocator.impl->free(&allocator, dust->inputs.density);
    sig_dsp_Dust_destroy(&allocator, dust);
}


struct sig_test_BufferPlayer* WaveformPlayer_new(
    sig_waveform_generator waveform,
    float sampleRate, float freq, float duration) {
    struct sig_Buffer* waveformBuffer = sig_Buffer_new(&allocator,
        (size_t) (audioSettings->sampleRate * duration));

    sig_Buffer_fillWithWaveform(waveformBuffer, waveform,
        sampleRate, 0.0f, freq);

    return sig_test_BufferPlayer_new(&allocator, context,
        waveformBuffer);
}

void WaveformPlayer_destroy(struct sig_test_BufferPlayer* player) {
    sig_Buffer_destroy(&allocator, player->buffer);
    sig_test_BufferPlayer_destroy(&allocator, player);
}

void testClockDetector(struct sig_test_BufferPlayer* clockPlayer,
    float duration, float expectedFreq) {
    struct sig_dsp_ClockDetector* det = sig_dsp_ClockDetector_new(
        &allocator, context);
    det->inputs.source = clockPlayer->outputs.main;

    struct sig_dsp_Signal* signals[2] = {&clockPlayer->signal, &det->signal};
    evaluateSignals(audioSettings, signals, 2, duration);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.0002,
        expectedFreq,
        FLOAT_ARRAY(det->outputs.main)[0],
        "The clock's frequency should have been detected correctly.");

    float expectedBPM = expectedFreq * 60.0f;

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.05,
        expectedBPM,
        FLOAT_ARRAY(det->outputs.bpm)[0],
        "The clock's bpm tempo should have been detected correctly.");

    sig_dsp_ClockDetector_destroy(&allocator, det);
}

void testClockDetector_SingleWaveform(sig_waveform_generator waveform,
    float bufferDuration, float clockFreq) {
    struct sig_test_BufferPlayer* clockPlayer =
        WaveformPlayer_new(waveform, audioSettings->sampleRate,
            clockFreq, bufferDuration);

    testClockDetector(clockPlayer, bufferDuration, clockFreq);

    WaveformPlayer_destroy(clockPlayer);
}

void test_sig_dsp_ClockDetector_square(void) {
    // Square wave, 2Hz for two seconds.
    testClockDetector_SingleWaveform(sig_waveform_square, 2.0f, 2.0f);
}

void test_sig_dsp_ClockDetector_sine(void) {
    // Sine wave, 10 Hz.
    testClockDetector_SingleWaveform(sig_waveform_sine, 2.0f, 10.0f);
}

void test_sig_dsp_ClockDetector_slowDown(void) {
    float bufferDuration = 2.0f;
    size_t bufferLen = (size_t) (audioSettings->sampleRate * bufferDuration);
    size_t halfBufferLen = bufferLen / 2;
    float fastSpeed = 10.0f;
    float slowSpeed = 2.0f;

    struct sig_Buffer* waveformBuffer = sig_Buffer_new(&allocator, bufferLen);

    struct sig_Buffer* fastSection = sig_BufferView_new(&allocator,
        waveformBuffer, 0, halfBufferLen);
    sig_Buffer_fillWithWaveform(fastSection, sig_waveform_square,
        audioSettings->sampleRate, 0.0f, fastSpeed);

    struct sig_Buffer* slowSection = sig_BufferView_new(&allocator,
        waveformBuffer, halfBufferLen, halfBufferLen);
    sig_Buffer_fillWithWaveform(slowSection, sig_waveform_square,
        audioSettings->sampleRate, 0.0f, slowSpeed);

    struct sig_test_BufferPlayer* clockPlayer = sig_test_BufferPlayer_new(
        &allocator, context, waveformBuffer);

    testClockDetector(clockPlayer, bufferDuration, slowSpeed);

    sig_BufferView_destroy(&allocator, fastSection);
    sig_BufferView_destroy(&allocator, slowSection);
    sig_Buffer_destroy(&allocator, waveformBuffer);
    sig_test_BufferPlayer_destroy(&allocator, clockPlayer);
}

void test_sig_dsp_ClockDetector_stop(void) {
    float bufferDuration = 122.0f;
    float clockDuration = 1.0f;
    size_t bufferLen = (size_t) (audioSettings->sampleRate * bufferDuration);
    size_t clockSectionLen = (size_t)
        (audioSettings->sampleRate * clockDuration);
    size_t silentSectionLen = (size_t)
        (audioSettings->sampleRate * (bufferDuration - clockDuration));
    float clockFreq = 10.0f;

    struct sig_Buffer* waveformBuffer = sig_Buffer_new(&allocator, bufferLen);

    struct sig_Buffer* clockSection = sig_BufferView_new(&allocator,
        waveformBuffer, 0, clockSectionLen);
    sig_Buffer_fillWithWaveform(clockSection, sig_waveform_square,
        audioSettings->sampleRate, 0.0f, clockFreq);

    struct sig_Buffer* silentSection = sig_BufferView_new(&allocator,
        waveformBuffer, clockSectionLen, silentSectionLen);
    sig_Buffer_fillWithSilence(silentSection);

    struct sig_test_BufferPlayer* clockPlayer = sig_test_BufferPlayer_new(
        &allocator, context, waveformBuffer);

    // When it no longer receives clock pulses, the detector
    // should hold on to whatever tempo it last calculated.
    testClockDetector(clockPlayer, bufferDuration, clockFreq);

    sig_BufferView_destroy(&allocator, silentSection);
    sig_BufferView_destroy(&allocator, clockSection);
    sig_test_BufferPlayer_destroy(&allocator, clockPlayer);
    sig_Buffer_destroy(&allocator, waveformBuffer);
}

void runTimedGate(struct sig_test_BufferPlayer* triggerPlayer,
    struct sig_Buffer* recBuffer, float recDuration, float gateDuration,
    float resetOnTrigger, float bipolar) {
    struct sig_dsp_TimedGate* timedGate = sig_dsp_TimedGate_new(&allocator,
        context);
    timedGate->inputs.duration = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, gateDuration);
    timedGate->inputs.trigger = triggerPlayer->outputs.main;

    timedGate->parameters.resetOnTrigger = resetOnTrigger;
    timedGate->parameters.bipolar = bipolar;

    struct sig_test_BufferRecorder* recorder = sig_test_BufferRecorder_new(
        &allocator, context, recBuffer);
    recorder->inputs.source = timedGate->outputs.main;

    struct sig_dsp_Signal* signals[3] = {
        &triggerPlayer->signal, &timedGate->signal, &recorder->signal};

    evaluateSignals(audioSettings, signals, 3, recDuration);
}


void test_sig_dsp_TimedGate_unipolar(void) {
    float recDuration = 1.0f;
    float recDurationSamples = sig_secondsToSamples(audioSettings,
        recDuration);
    float gateDuration = 0.1f;
    float gateDurationSamples = sig_secondsToSamples(audioSettings,
        gateDuration);

    // One trigger, after 10 samples have elapsed.
    struct sig_Buffer* triggerBuffer = sig_Buffer_new(&allocator,
        recDurationSamples);
    sig_Buffer_fillWithSilence(triggerBuffer);
    FLOAT_ARRAY(triggerBuffer->samples)[10] = 1.0f;
    struct sig_test_BufferPlayer* triggerPlayer = sig_test_BufferPlayer_new(
        &allocator, context, triggerBuffer);

    struct sig_Buffer* recBuffer = sig_Buffer_new(&allocator,
        recDurationSamples);

    runTimedGate(triggerPlayer, recBuffer, recDuration, gateDuration,
        0.0f, 0.0f);

    // First 9 samples, prior to the trigger, should be silent.
    struct sig_Buffer* startSection = sig_BufferView_new(&allocator,
        recBuffer, 0, 10);
    testAssertBufferIsSilent(&allocator,
        startSection->samples, startSection->length);

    // Gate should be high for the specified duration.
    struct sig_Buffer* gateSection = sig_BufferView_new(&allocator,
        recBuffer, 10, gateDurationSamples);
    testAssertBufferContainsValueOnly(&allocator, 1.0,
        gateSection->samples, gateSection->length);

    // All samples after the gate should be silent.
    size_t endSectionIdx = 10 + gateDurationSamples;
    struct sig_Buffer* endSection = sig_BufferView_new(&allocator,
        recBuffer, endSectionIdx, recBuffer->length - endSectionIdx);
    testAssertBufferIsSilent(&allocator, endSection->samples,
        endSection->length);
}

void test_sig_dsp_TimedGate_resetOnTrigger(void) {
    float recDuration = 0.3f;
    size_t recDurationSamples = sig_secondsToSamples(audioSettings,
        recDuration);
    float gateDuration = 0.1f;
    size_t gateDurationSamples = sig_secondsToSamples(audioSettings,
        gateDuration);
    size_t triggerIndices[2] = {0, (size_t) gateDurationSamples / 2};

    struct sig_Buffer* triggerBuffer = sig_Buffer_new(&allocator,
        recDurationSamples);
    sig_Buffer_fillWithSilence(triggerBuffer);
    FLOAT_ARRAY(triggerBuffer->samples)[triggerIndices[0]] = 1.0f;
    FLOAT_ARRAY(triggerBuffer->samples)[triggerIndices[1]] = 1.0f;

    struct sig_test_BufferPlayer* triggerPlayer = sig_test_BufferPlayer_new(
        &allocator, context, triggerBuffer);

    struct sig_Buffer* recBuffer = sig_Buffer_new(&allocator,
        recDurationSamples);

    runTimedGate(triggerPlayer, recBuffer, recDuration, gateDuration,
        1.0f, 0.0f);

    // The gate should open right away.
    // Halfway through the first gate, the signal should drop for one sample
    // and then reopen, staying high for gateDurationSamples.
    struct sig_Buffer* firstGate = sig_BufferView_new(&allocator,
        recBuffer, 0, triggerIndices[1]);

    testAssertBufferContainsValueOnly(&allocator, 1.0,
        firstGate->samples, firstGate->length);

    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.0f,
        FLOAT_ARRAY(recBuffer->samples)[triggerIndices[1]],
        "When the second trigger fires, the gate should drop to zero for one sample.");

    // The second gate should be high for the full gate duration.
    size_t secondGateStartIdx = triggerIndices[1] + 1;
    size_t secondGateLen = gateDurationSamples;
    struct sig_Buffer* secondGate = sig_BufferView_new(&allocator,
        recBuffer, secondGateStartIdx, secondGateLen);
    testAssertBufferContainsValueOnly(&allocator, 1.0,
        secondGate->samples, secondGate->length);

    // Afterwards we should have only silence.
    size_t silenceStartIdx = secondGateStartIdx + secondGateLen;
    size_t silenceLength = recBuffer->length - silenceStartIdx;
    struct sig_Buffer* silence = sig_BufferView_new(&allocator,
        recBuffer, silenceStartIdx, silenceLength);
    testAssertBufferIsSilent(&allocator, silence->samples, silence->length);
}

void test_sig_dsp_TimedGate_bipolar(void) {
    float recDuration = 0.3f;
    size_t recDurationSamples = sig_secondsToSamples(audioSettings,
        recDuration);
    float gateDuration = 0.1f;
    size_t gateDurationSamples = sig_secondsToSamples(audioSettings,
        gateDuration);

    struct sig_Buffer* triggerBuffer = sig_Buffer_new(&allocator,
        recDurationSamples);
    sig_Buffer_fillWithSilence(triggerBuffer);
    FLOAT_ARRAY(triggerBuffer->samples)[0] = 0.5f;
    FLOAT_ARRAY(triggerBuffer->samples)[gateDurationSamples] = -0.5f;
    FLOAT_ARRAY(triggerBuffer->samples)[gateDurationSamples * 2] = 0.75f;

    struct sig_test_BufferPlayer* triggerPlayer = sig_test_BufferPlayer_new(
        &allocator, context, triggerBuffer);

    struct sig_Buffer* recBuffer = sig_Buffer_new(&allocator,
        recDurationSamples);

    runTimedGate(triggerPlayer, recBuffer, recDuration, gateDuration,
        0.0f, 1.0f);

    // The gate should open right away and be positive,
    // and match the amplitude of the trigger.
    struct sig_Buffer* firstGate = sig_BufferView_new(&allocator,
        recBuffer, 0, gateDurationSamples);
    testAssertBufferContainsValueOnly(&allocator, 0.5f,
        firstGate->samples, firstGate->length);

    // The second gate should be negative and also match the amplitude
    // of the trigger.
    size_t secondGateStartIdx = gateDurationSamples;
    size_t secondGateLen = gateDurationSamples;
    struct sig_Buffer* secondGate = sig_BufferView_new(&allocator,
        recBuffer, secondGateStartIdx, secondGateLen);
    testAssertBufferContainsValueOnly(&allocator, -0.5f,
        secondGate->samples, secondGate->length);

    // The third gate should also match the gate value.
    size_t thirdGateStartIdx = gateDurationSamples * 2;
    size_t thirdGateLen = gateDurationSamples;
    struct sig_Buffer* thirdGate = sig_BufferView_new(&allocator,
        recBuffer, thirdGateStartIdx, thirdGateLen);
    testAssertBufferContainsValueOnly(&allocator, 0.75f,
        thirdGate->samples, thirdGate->length);
}

void generateAndTestListIndex(struct sig_dsp_Value* idx, float idxValue,
    struct sig_dsp_List* list, float expectedListValue) {
    idx->parameters.value = idxValue;
    idx->signal.generate(idx);
    list->signal.generate(list);
    testAssertBufferContainsValueOnly(&allocator, expectedListValue,
        list->outputs.main, audioSettings->blockSize);
}

void test_sig_dsp_List_wrapping(void) {
    struct sig_dsp_Value* idx = sig_dsp_Value_new(&allocator,
        context);

    float listItems[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    struct sig_Buffer listBuffer = {
        .length = 5,
        .samples = listItems
    };

    struct sig_dsp_List* list = sig_dsp_List_new(&allocator, context);
    list->list = &listBuffer;
    list->inputs.index = idx->outputs.main;

    // No index rounding required.
    generateAndTestListIndex(idx, 0.0f, list, 1.0f);
    testAssertBufferContainsValueOnly(&allocator, 5.0f,
        list->outputs.length, audioSettings->blockSize);

    // Index is normalized between 0.0->1.0f,
    // so there are values at 0.0, 0.25 0.5 0.75, 1.0
    // Fractional indexes less than halfway should be rounded down.
    generateAndTestListIndex(idx, 0.12f, list, 1.0f);

    // Fractional indexes >= half should be rounded up.
    generateAndTestListIndex(idx, 0.125f, list, 2.0f);

    // Fractional indexes >= half should be rounded up.
    generateAndTestListIndex(idx, 0.25f, list, 2.0f);

    // Another even index.
    generateAndTestListIndex(idx, 0.5f, list, 3.0f);

    // Nearly the last index.
    generateAndTestListIndex(idx, 0.99f, list, 5.0f);

    // Just past the first index.
    generateAndTestListIndex(idx, 0.00001f, list, 1.0f);

    // Exactly the last index.
    generateAndTestListIndex(idx, 1.0f, list, 5.0f);

    // A slightly out of bounds index should round down to the last index.
    generateAndTestListIndex(idx, 1.1f, list, 5.0f);

    // But a larger index should wrap and round up.
    generateAndTestListIndex(idx, 1.25f, list, 2.0f);

    // Larger indices should wrap around past the beginning.
    generateAndTestListIndex(idx, 1.5f, list, 3.0f);

    // Very large indices should wrap around several times.
    generateAndTestListIndex(idx, 3.0f, list, 5.0f);

    // Negative indices should wrap around the end.
    // Note the index "shift" here: -0.25 should point
    // to the last value in the list.
    generateAndTestListIndex(idx, -0.25f, list, 5.0f);

    // But not if they're not negative enough.
    generateAndTestListIndex(idx, -0.12f, list, 1.0f);

    // Very negative indices should keep wrapping around.
    generateAndTestListIndex(idx, -1.5f, list, 5.0f);

    // Small lists should work.
    float small[2] = {1.0f, 2.0f};
    listBuffer.length = 2;
    listBuffer.samples = small;
    generateAndTestListIndex(idx, 0.0f, list, 1.0f);
    generateAndTestListIndex(idx, 1.0f, list, 2.0f);
    generateAndTestListIndex(idx, 0.5f, list, 2.0f);
    generateAndTestListIndex(idx, 0.25f, list, 1.0f);
    generateAndTestListIndex(idx, -1.0f, list, 2.0f);
    generateAndTestListIndex(idx, -0.25f, list, 1.0f);
    generateAndTestListIndex(idx, -0.5f, list, 2.0f);

    // Very small lists should work.
    float verySmall[1] = {1.0f};
    listBuffer.length = 1;
    listBuffer.samples = verySmall;
    generateAndTestListIndex(idx, 1.0f, list, 1.0f);
    generateAndTestListIndex(idx, 0.0f, list, 1.0f);
    generateAndTestListIndex(idx, 0.5f, list, 1.0f);
    generateAndTestListIndex(idx, -0.5f, list, 1.0f);
    generateAndTestListIndex(idx, -1.15f, list, 1.0f);
}

void test_sig_dsp_List_noNormalization(void) {
    struct sig_dsp_Value* idx = sig_dsp_Value_new(&allocator,
        context);

    float listItems[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    struct sig_Buffer listBuffer = {
        .length = 5,
        .samples = listItems
    };

    struct sig_dsp_List* list = sig_dsp_List_new(&allocator, context);
    list->parameters.normalizeIndex = 0.0f;
    list->list = &listBuffer;
    list->inputs.index = idx->outputs.main;

    // No interpolation.

    // Exactly the first index.
    generateAndTestListIndex(idx, 0.0f, list, 1.0f);

    // Exactly the last index.
    generateAndTestListIndex(idx, 4.0f, list, 5.0f);

    // Wrap around.
    generateAndTestListIndex(idx, 5.0f, list, 2.0f);

    // Fractional index, round down.
    generateAndTestListIndex(idx, 5.1f, list, 2.0f);

    // Fractional index, round up.
    generateAndTestListIndex(idx, 1.8f, list, 3.0f);

    // Interpolation
    list->parameters.interpolate = 1.0f;
    generateAndTestListIndex(idx, 5.1f, list, 2.1f);

    // Wrap around, interpolation.
    generateAndTestListIndex(idx, -2.5f, list, 3.5f);
}

void test_sig_dsp_List_clamping(void) {
    struct sig_dsp_Value* idx = sig_dsp_Value_new(&allocator,
        context);

    float listItems[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    struct sig_Buffer listBuffer = {
        .length = 5,
        .samples = listItems
    };

    struct sig_dsp_List* list = sig_dsp_List_new(&allocator, context);
    list->parameters.wrap = 0.0f;
    list->list = &listBuffer;
    list->inputs.index = idx->outputs.main;

    generateAndTestListIndex(idx, 0.0f, list, 1.0f);
    generateAndTestListIndex(idx, 0.00001f, list, 1.0f);
    generateAndTestListIndex(idx, 0.99f, list, 5.0f);
    generateAndTestListIndex(idx, 1.0f, list, 5.0f);
    generateAndTestListIndex(idx, 1.001f, list, 5.0f);
    generateAndTestListIndex(idx, 1.125f, list, 5.0f);
    generateAndTestListIndex(idx, 1.25f, list, 5.0f);
    generateAndTestListIndex(idx, 3.0f, list, 5.0f);
    generateAndTestListIndex(idx, -0.00001f, list, 1.0f);
    generateAndTestListIndex(idx, -0.99f, list, 1.0f);
    generateAndTestListIndex(idx, -1.5f, list, 1.0f);
}

void test_sig_dsp_List_interpolation(void) {
    struct sig_dsp_Value* idx = sig_dsp_Value_new(&allocator,
        context);

    float listItems[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    struct sig_Buffer listBuffer = {
        .length = 5,
        .samples = listItems
    };

    struct sig_dsp_List* list = sig_dsp_List_new(&allocator, context);
    list->parameters.wrap = 1.0f;
    list->parameters.interpolate = 1.0f;
    list->parameters.normalizeIndex = 1.0f;
    list->list = &listBuffer;
    list->inputs.index = idx->outputs.main;

    // Start
    generateAndTestListIndex(idx, 0.0f, list, 1.0f);

    // Interpolated
    generateAndTestListIndex(idx, 0.1f, list, 1.4f);

    // Halfway
    generateAndTestListIndex(idx, 0.5f, list, 3.0f);

    // Interpolated
    generateAndTestListIndex(idx, 0.6f, list, 3.4f);

    // End
    generateAndTestListIndex(idx, 1.0f, list, 5.0f);

    // Almost end.
    generateAndTestListIndex(idx, 0.99999999999f, list, 5.0f);

    // Wrap around and interpolate. 1.1f should be the same as 0.1f
    generateAndTestListIndex(idx, 1.1f, list, 1.4f);
}

void test_sig_dsp_List_noList(void) {
    struct sig_dsp_Value* idx = sig_dsp_Value_new(&allocator,
        context);

    float empty[1] = {42.0f};  // Not so empty, because MSVC.
    struct sig_Buffer listBuffer = {
        .length = 0,
        .samples = empty
    };

    struct sig_dsp_List* list = sig_dsp_List_new(&allocator, context);
    list->inputs.index = idx->outputs.main;

    // A NULL list should return silence.
    list->signal.generate(list);
    testAssertBufferIsSilent(&allocator, list->outputs.main,
        audioSettings->blockSize);
    testAssertBufferIsSilent(&allocator, list->outputs.length,
        audioSettings->blockSize);

    // An empty list should return silence.
    list->list = &listBuffer;
    list->signal.generate(list);
    testAssertBufferIsSilent(&allocator, list->outputs.main,
        audioSettings->blockSize);
    testAssertBufferIsSilent(&allocator, list->outputs.length,
        audioSettings->blockSize);

}

void test_sig_dsp_DCBlock_AC(void) {
    struct sig_dsp_DCBlock* dcBlock = sig_dsp_DCBlock_new(&allocator,
        context);

    // Test a signal at > 5Hz. It should pass through unattenuated.
    struct sig_dsp_Oscillator* sine = sig_dsp_SineOscillator_new(
        &allocator, context);
    sine->inputs.freq = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 10.0f);
    dcBlock->inputs.source = sine->outputs.main;

    // Generate some sample blocks,
    // giving time for the filter to settle.
    for (size_t i = 0; i < 4000; i++) {
        sine->signal.generate(sine);
        dcBlock->signal.generate(dcBlock);
    }

    // The filter is probably not phase-preserving, so we can't
    // test for exact values, but we can at least check the basics.
    // TODO: Implement an FFT analysis of the output
    // to verify that the frequency content is as expected.
    testAssertBufferNotSilent(&allocator,dcBlock->outputs.main,
        context->audioSettings->blockSize);
    testAssertBufferValuesInRange(dcBlock->outputs.main,
        context->audioSettings->blockSize, -1.0f, 1.0f);
}

void test_sig_dsp_DCBlock_DC(void) {
    struct sig_dsp_DCBlock* dcBlock = sig_dsp_DCBlock_new(&allocator,
        context);

    // A steady DC signal—output should be nearly silent.
    dcBlock = sig_dsp_DCBlock_new(&allocator, context);
    struct sig_dsp_ConstantValue* constant = sig_dsp_ConstantValue_new(
        &allocator, context, 1.0f);
    dcBlock->inputs.source = constant->outputs.main;

    for (size_t i = 0; i < 4000; i++) {
        dcBlock->signal.generate(dcBlock);
    }

    testAssertSamplesBelowDB(dcBlock->outputs.main,
        -180.0f, 1.0f, context->audioSettings->blockSize);
}

void test_sig_softClip(void) {
    // Below the threshold the sample passes through unchanged.
    TEST_ASSERT_EQUAL_FLOAT(0.5f, sig_softClip(0.5f, 0.8f));
    TEST_ASSERT_EQUAL_FLOAT(-0.5f, sig_softClip(-0.5f, 0.8f));
    TEST_ASSERT_EQUAL_FLOAT(0.8f, sig_softClip(0.8f, 0.8f));

    // The response is odd symmetric.
    TEST_ASSERT_EQUAL_FLOAT(sig_softClip(1.2f, 0.8f),
        -sig_softClip(-1.2f, 0.8f));

    // Above the threshold the sample is compressed but keeps increasing.
    TEST_ASSERT_TRUE(sig_softClip(1.0f, 0.8f) > 0.8f);
    TEST_ASSERT_TRUE(sig_softClip(1.0f, 0.8f) < sig_softClip(1.2f, 0.8f));

    // Large samples approach full scale without exceeding it.
    TEST_ASSERT_TRUE(sig_softClip(100.0f, 0.8f) > 0.99f);
    TEST_ASSERT_TRUE(sig_softClip(100.0f, 0.8f) <= 1.0f);

    // The threshold is clamped to a usable range.
    TEST_ASSERT_EQUAL_FLOAT(0.5f, sig_softClip(0.5f, 1.5f));
}

void test_sig_dsp_SoftClip(void) {
    struct sig_dsp_SoftClip* softClip = sig_dsp_SoftClip_new(&allocator,
        context);
    softClip->parameters.threshold = 0.8f;

    struct sig_dsp_ConstantValue* dc = sig_dsp_ConstantValue_new(
        &allocator, context, 2.0f);
    softClip->inputs.source = dc->outputs.main;

    for (size_t i = 0; i < 4; i++) {
        dc->signal.generate(dc);
        softClip->signal.generate(softClip);
    }

    // A sample above the threshold is limited to just below full scale.
    testAssertSamplesBelowDB(softClip->outputs.main, 0.0f, 1.0f,
        audioSettings->blockSize);
    testAssertSamplesAboveDB(softClip->outputs.main, -0.1f, 1.0f,
        audioSettings->blockSize);
}

static float_array_ptr alternatingBlock(void) {
    float_array_ptr samples = sig_AudioBlock_new(&allocator, audioSettings);
    for (size_t i = 0; i < audioSettings->blockSize; i++) {
        FLOAT_ARRAY(samples)[i] = (i % 2 == 0) ? 1.0f : -1.0f;
    }

    return samples;
}

static struct sig_test_BufferPlayer* newAlternatingSource(void) {
    struct sig_Buffer* buffer = (struct sig_Buffer*) allocator.impl->malloc(
        &allocator, sizeof(struct sig_Buffer));
    buffer->length = audioSettings->blockSize;
    buffer->samples = alternatingBlock();

    return sig_test_BufferPlayer_new(&allocator, context, buffer);
}

static void generateSVF(struct sig_dsp_Signal* source,
    struct sig_dsp_SVF* svf, size_t numBlocks) {
    for (size_t i = 0; i < numBlocks; i++) {
        source->generate(source);
        svf->signal.generate(svf);
    }
}

static struct sig_dsp_SVF* newSVF(float frequency, float resonance) {
    struct sig_dsp_SVF* svf = sig_dsp_SVF_new(&allocator, context);
    svf->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, frequency);
    svf->inputs.resonance = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, resonance);

    return svf;
}

void test_sig_filter_svf_shapes(void) {
    // The shapes are weighted sums of the input (v0), band pass (v1),
    // and low pass (v2) taps, scaled by the damping k.
    float input = 1.0f;
    float band = 0.5f;
    float low = 0.25f;
    float k = 1.5f;

    TEST_ASSERT_FLOAT_WITHIN(0.0001f, input - k * band - low,
        sig_filter_svf_highPass(input, band, low, k));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, input - k * band,
        sig_filter_svf_notch(input, band, low, k));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, input - k * band - 2.0f * low,
        sig_filter_svf_peak(input, band, low, k));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, input - 2.0f * k * band,
        sig_filter_svf_allPass(input, band, low, k));
}

void test_sig_filter_SVF_generate(void) {
    struct sig_filter_SVF state;
    sig_filter_SVF_init(&state);
    sig_filter_SVF_calculateCoefficients(&state, 1000.0f, 0.5f, 44100.0f);

    // A low pass filter settles to unity gain at DC, and its band pass
    // output settles to zero.
    struct sig_filter_SVF_Taps taps;
    for (size_t i = 0; i < 4410; i++) {
        sig_filter_SVF_generate(&state, 1.0f, &taps);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, taps.low);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, taps.band);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, taps.input);
}

void test_sig_filter_SVF_calculateCoefficients_atHalfSampleRate(void) {
    struct sig_filter_SVF state;
    sig_filter_SVF_init(&state);

    // A cutoff at the Nyquist frequency is clamped just below it, so the
    // tangent stays positive and finite instead of crossing its pole.
    sig_filter_SVF_calculateCoefficients(&state, 22050.0f, 0.5f, 44100.0f);

    TEST_ASSERT_TRUE(state.a1 > 0.0f);
    TEST_ASSERT_TRUE(state.a2 > 0.0f);
    TEST_ASSERT_TRUE(state.a3 > 0.0f);
    // The clamp gives a tangent of tan(pi * 0.49), about 31.82.
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 31.82f, state.a2 / state.a1);
}

void test_sig_dsp_SVF_lowPass(void) {
    struct sig_dsp_SVF* svf = newSVF(1000.0f, 0.5f);
    struct sig_dsp_ConstantValue* dc = sig_dsp_ConstantValue_new(
        &allocator, context, 1.0f);
    svf->inputs.source = dc->outputs.main;

    generateSVF(&dc->signal, svf, 4000);

    // A low pass filter passes DC at unity gain.
    testAssertSamplesAboveDB(svf->outputs.low, -0.1f, 1.0f,
        audioSettings->blockSize);
}

void test_sig_dsp_SVF_lowPass_blocksHighFrequency(void) {
    struct sig_test_BufferPlayer* source = newAlternatingSource();
    struct sig_dsp_SVF* svf = newSVF(1000.0f, 0.5f);
    svf->inputs.source = source->outputs.main;

    generateSVF(&source->signal, svf, 4000);

    // A low pass filter strongly attenuates a signal at the Nyquist
    // frequency.
    testAssertSamplesBelowDB(svf->outputs.low, -30.0f, 1.0f,
        audioSettings->blockSize);
}

void test_sig_dsp_SVF_highPass(void) {
    struct sig_dsp_SVF* svf = newSVF(1000.0f, 0.5f);
    struct sig_dsp_ConstantValue* dc = sig_dsp_ConstantValue_new(
        &allocator, context, 1.0f);
    svf->inputs.source = dc->outputs.main;

    generateSVF(&dc->signal, svf, 4000);

    // A high pass filter blocks DC.
    testAssertSamplesBelowDB(svf->outputs.high, -60.0f, 1.0f,
        audioSettings->blockSize);
}

void test_sig_dsp_SVF_highPass_passesHighFrequency(void) {
    struct sig_test_BufferPlayer* source = newAlternatingSource();
    struct sig_dsp_SVF* svf = newSVF(1000.0f, 0.5f);
    svf->inputs.source = source->outputs.main;

    generateSVF(&source->signal, svf, 4000);

    // A high pass filter passes a signal at the Nyquist frequency.
    testAssertSamplesAboveDB(svf->outputs.high, -3.0f, 1.0f,
        audioSettings->blockSize);
}

void test_sig_dsp_SVF_bandPass(void) {
    struct sig_dsp_SVF* svf = newSVF(1000.0f, 0.5f);
    struct sig_dsp_ConstantValue* dc = sig_dsp_ConstantValue_new(
        &allocator, context, 1.0f);
    svf->inputs.source = dc->outputs.main;

    generateSVF(&dc->signal, svf, 4000);

    // A band pass filter blocks DC.
    testAssertSamplesBelowDB(svf->outputs.band, -60.0f, 1.0f,
        audioSettings->blockSize);
}

void test_sig_dsp_SVF_notch(void) {
    struct sig_dsp_SVF* svf = newSVF(1000.0f, 0.5f);
    struct sig_dsp_ConstantValue* dc = sig_dsp_ConstantValue_new(
        &allocator, context, 1.0f);
    svf->inputs.source = dc->outputs.main;

    generateSVF(&dc->signal, svf, 4000);

    // A notch filter passes DC at unity gain.
    testAssertSamplesAboveDB(svf->outputs.notch, -0.1f, 1.0f,
        audioSettings->blockSize);
}

void test_sig_dsp_SVF_peak(void) {
    struct sig_dsp_SVF* svf = newSVF(1000.0f, 0.5f);
    struct sig_dsp_ConstantValue* dc = sig_dsp_ConstantValue_new(
        &allocator, context, 1.0f);
    svf->inputs.source = dc->outputs.main;

    generateSVF(&dc->signal, svf, 4000);

    // The peak response has unity magnitude at DC.
    testAssertSamplesAboveDB(svf->outputs.peak, -0.1f, 1.0f,
        audioSettings->blockSize);
}

void test_sig_dsp_SVF_allPass(void) {
    struct sig_dsp_SVF* svf = newSVF(1000.0f, 0.5f);
    struct sig_dsp_ConstantValue* dc = sig_dsp_ConstantValue_new(
        &allocator, context, 1.0f);
    svf->inputs.source = dc->outputs.main;

    generateSVF(&dc->signal, svf, 4000);

    // An all pass filter passes DC at unity gain.
    testAssertSamplesAboveDB(svf->outputs.all, -0.1f, 1.0f,
        audioSettings->blockSize);
}

void test_sig_dsp_SVF_resonance(void) {
    // Drive a low pass filter at its cutoff frequency with a sine.
    // A higher resonance produces a louder output.
    struct sig_dsp_Oscillator* sine = sig_dsp_SineOscillator_new(
        &allocator, context);
    sine->inputs.freq = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 1000.0f);

    struct sig_dsp_SVF* damped = newSVF(1000.0f, 0.0f);
    damped->inputs.source = sine->outputs.main;

    struct sig_dsp_SVF* resonant = newSVF(1000.0f, 0.9f);
    resonant->inputs.source = sine->outputs.main;

    for (size_t i = 0; i < 4000; i++) {
        sine->signal.generate(sine);
        damped->signal.generate(damped);
        resonant->signal.generate(resonant);
    }

    float dampedPeak = 0.0f;
    float resonantPeak = 0.0f;
    for (size_t i = 0; i < audioSettings->blockSize; i++) {
        float dampedSample = fabsf(FLOAT_ARRAY(damped->outputs.low)[i]);
        float resonantSample = fabsf(FLOAT_ARRAY(resonant->outputs.low)[i]);
        if (dampedSample > dampedPeak) {
            dampedPeak = dampedSample;
        }
        if (resonantSample > resonantPeak) {
            resonantPeak = resonantSample;
        }
    }

    // With a resonance of 0.0 the low pass gain at the cutoff is 0.5 (-6 dB).
    TEST_ASSERT_TRUE(dampedPeak > 0.4f && dampedPeak < 0.6f);
    // With a resonance of 0.9 the low pass gain at the cutoff is 5 (+14 dB).
    TEST_ASSERT_TRUE(resonantPeak > 4.0f && resonantPeak < 6.0f);
}

void test_sig_dsp_SVF_highResonanceIsStable(void) {
    struct sig_dsp_Oscillator* sine = sig_dsp_SineOscillator_new(
        &allocator, context);
    sine->inputs.freq = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 1000.0f);

    struct sig_dsp_SVF* svf = newSVF(1000.0f, 0.99f);
    svf->inputs.source = sine->outputs.main;

    generateSVF(&sine->signal, svf, 4000);

    // Near maximum resonance the filter's outputs remain bounded.
    testAssertBufferValuesInRange(svf->outputs.low,
        audioSettings->blockSize, -100.0f, 100.0f);
    testAssertBufferValuesInRange(svf->outputs.band,
        audioSettings->blockSize, -100.0f, 100.0f);
    testAssertBufferValuesInRange(svf->outputs.high,
        audioSettings->blockSize, -100.0f, 100.0f);
    testAssertBufferValuesInRange(svf->outputs.notch,
        audioSettings->blockSize, -100.0f, 100.0f);
    testAssertBufferValuesInRange(svf->outputs.peak,
        audioSettings->blockSize, -100.0f, 100.0f);
    testAssertBufferValuesInRange(svf->outputs.all,
        audioSettings->blockSize, -100.0f, 100.0f);
}

void test_sig_dsp_SVF_maxResonanceIsStable(void) {
    struct sig_dsp_Oscillator* sine = sig_dsp_SineOscillator_new(
        &allocator, context);
    sine->inputs.freq = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 1000.0f);

    struct sig_dsp_SVF* svf = newSVF(1000.0f, 1.0f);
    svf->inputs.source = sine->outputs.main;

    // Four seconds of a sine at the cutoff frequency.
    size_t numBlocks = (size_t) (4.0f * audioSettings->sampleRate /
        (float) audioSettings->blockSize);
    generateSVF(&sine->signal, svf, numBlocks);

    // The minimum damping keeps the resonance bounded instead of growing
    // without limit.
    float peak = 0.0f;
    for (size_t i = 0; i < audioSettings->blockSize; i++) {
        float sample = fabsf(FLOAT_ARRAY(svf->outputs.low)[i]);
        if (sample > peak) {
            peak = sample;
        }
    }
    TEST_ASSERT_TRUE(peak < 500.0f);
}

void test_sig_dsp_SVFLowPass(void) {
    struct sig_dsp_SVFLowPass* svf = sig_dsp_SVFLowPass_new(&allocator,
        context);
    svf->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 1000.0f);
    svf->inputs.resonance = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.5f);

    struct sig_dsp_ConstantValue* dc = sig_dsp_ConstantValue_new(
        &allocator, context, 1.0f);
    svf->inputs.source = dc->outputs.main;

    for (size_t i = 0; i < 4000; i++) {
        dc->signal.generate(dc);
        svf->signal.generate(svf);
    }

    // A low pass filter passes DC at unity gain.
    testAssertSamplesAboveDB(svf->outputs.main, -0.1f, 1.0f,
        audioSettings->blockSize);
}

void test_sig_filter_Ladder_generate(void) {
    struct sig_filter_Ladder state;
    sig_filter_Ladder_init(&state, 4);
    sig_filter_Ladder_calculateCoefficients(&state, 1000.0f, 44100.0f);

    // Each pole passes DC, and the input saturator limits the signal first.
    struct sig_filter_Ladder_Taps taps;
    for (size_t i = 0; i < 4410; i++) {
        sig_filter_Ladder_generate(&state, 1.0f, 0.0f, 0.5f, &taps);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.001f, sig_fastTanhf(1.0f), taps.pole4);
}

void test_sig_filter_Ladder_calcStageInput(void) {
    struct sig_filter_Ladder state;
    sig_filter_Ladder_init(&state, 4);

    // The interpolation starts at the previous input sample and moves toward
    // the current one over the sub-steps. With no resonance or passband gain,
    // the interpolated value passes through unchanged.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
        sig_filter_Ladder_calcStageInput(&state, 1.0f, 0.0f, 0.0f, 0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f,
        sig_filter_Ladder_calcStageInput(&state, 1.0f, 1.0f, 0.0f, 0.0f));
}

void test_sig_filter_Ladder_calcStages(void) {
    struct sig_filter_Ladder state;
    sig_filter_Ladder_init(&state, 4);
    sig_filter_Ladder_calculateCoefficients(&state, 1000.0f, 44100.0f);

    // Each pole passes DC, so a constant input settles at unity at every pole.
    struct sig_filter_Ladder_Taps taps;
    for (size_t i = 0; i < 4410; i++) {
        sig_filter_Ladder_calcStages(&state, 1.0f, &taps);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, taps.pole1);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, taps.pole4);
}

void test_sig_dsp_Ladder_outputs(void) {
    struct sig_dsp_Ladder* ladder = sig_dsp_Ladder_new(&allocator, context);
    ladder->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 1000.0f);
    ladder->inputs.resonance = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.0f);

    struct sig_dsp_Oscillator* sine = sig_dsp_SineOscillator_new(
        &allocator, context);
    sine->inputs.freq = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 1000.0f);
    ladder->inputs.source = sine->outputs.main;

    for (size_t i = 0; i < 100; i++) {
        sine->signal.generate(sine);
        ladder->signal.generate(ladder);
    }

    // The ladder writes a signal to each of its three outputs.
    testAssertBufferNotSilent(&allocator, ladder->outputs.main,
        audioSettings->blockSize);
    testAssertBufferNotSilent(&allocator, ladder->outputs.twoPole,
        audioSettings->blockSize);
    testAssertBufferNotSilent(&allocator, ladder->outputs.fourPole,
        audioSettings->blockSize);
}

// The distance between two phases on the 0.0-1.0 phase circle.
float phaseDistance(float a, float b) {
    float difference = fabsf(a - b);
    return difference > 0.5f ? 1.0f - difference : difference;
}

// Drives a ratio phasor with a reference phase ramp, and asserts that its
// phase stays at numerator / denominator times the reference's accumulated
// cycles, transposed by octaves.
void testAssertRatioPhasorLocks(float numerator, float denominator,
    float octaves, float frequency, size_t numSamples) {
    float sampleRate = 48000.0f;
    float reference = 0.0f;
    double referenceCycles = 0.0;
    double ratio = (double) numerator / (double) denominator *
        pow(2.0, (double) octaves);
    struct sig_osc_RatioPhasor phasor;
    sig_osc_RatioPhasor_init(&phasor);

    for (size_t i = 0; i < numSamples; i++) {
        float phase = sig_osc_RatioPhasor_generate(&phasor, reference,
            numerator, denominator, octaves);
        double cycles = ratio * (referenceCycles + reference);
        float expected = (float) (cycles - floor(cycles));
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
            phaseDistance(expected, phase));

        float previous = reference;
        sig_osc_Oscillator_accumulatePhase(&reference, frequency, sampleRate);
        if (reference < previous) {
            referenceCycles += 1.0;
        }
    }
}

void test_sig_osc_RatioPhasor_locksToWholeRatio(void) {
    // With a whole number ratio, the phasor runs exactly three times
    // as fast as the reference, starting together.
    testAssertRatioPhasorLocks(3.0f, 1.0f, 0.0f, 100.0f, 48000);
}

void test_sig_osc_RatioPhasor_locksAcrossReferenceCycles(void) {
    // With a fractional ratio, the phasor completes numerator cycles for
    // every denominator cycles of the reference, aligning each time.
    testAssertRatioPhasorLocks(2.0f, 3.0f, 0.0f, 100.0f, 48000);
    testAssertRatioPhasorLocks(7.0f, 2.0f, 0.0f, 100.0f, 48000);
    testAssertRatioPhasorLocks(1.0f, 11.0f, 0.0f, 100.0f, 48000);
}

void test_sig_osc_RatioPhasor_locksWhenTransposedByOctaves(void) {
    // Transposing by whole octaves keeps the ratio a fraction of whole
    // numbers, so the phasor stays locked to the reference: two thirds
    // up an octave, three down two octaves, and one eleventh down an octave.
    testAssertRatioPhasorLocks(2.0f, 3.0f, 1.0f, 100.0f, 48000);
    testAssertRatioPhasorLocks(3.0f, 1.0f, -2.0f, 100.0f, 48000);
    testAssertRatioPhasorLocks(1.0f, 11.0f, -1.0f, 100.0f, 48000);
}

void test_sig_osc_RatioPhasor_glidesAfterRatioChange(void) {
    float sampleRate = 48000.0f;
    float frequency = 100.0f;
    float reference = 0.0f;
    double referenceCycles = 0.0;
    struct sig_osc_RatioPhasor phasor;
    sig_osc_RatioPhasor_init(&phasor);
    sig_osc_RatioPhasor_calculateCoefficient(&phasor, 0.05f, sampleRate);

    // Run at 1:1, then switch to 2:1 a quarter of the way through
    // a reference cycle.
    float previousPhase = 0.0f;
    size_t switchSample = 120;
    size_t settledSample = switchSample + 24000;
    for (size_t i = 0; i <= settledSample; i++) {
        float numerator = i < switchSample ? 1.0f : 2.0f;
        float phase = sig_osc_RatioPhasor_generate(&phasor, reference,
            numerator, 1.0f, 0.0f);
        double cycles = numerator * (referenceCycles + reference);
        float locked = (float) (cycles - floor(cycles));

        if (i == switchSample) {
            // The phase stays continuous instead of jumping to the newly
            // locked phase, which is a quarter cycle away.
            TEST_ASSERT_TRUE(phaseDistance(previousPhase, phase) < 0.01f);
            TEST_ASSERT_TRUE(phaseDistance(locked, phase) > 0.2f);
        } else if (i == settledSample) {
            // After ten glide times, the phase has settled at the newly
            // locked phase.
            TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f,
                phaseDistance(locked, phase));
        }

        previousPhase = phase;
        float previous = reference;
        sig_osc_Oscillator_accumulatePhase(&reference, frequency, sampleRate);
        if (reference < previous) {
            referenceCycles += 1.0;
        }
    }
}

void test_sig_osc_RatioPhasor_followsWithoutLocking(void) {
    float sampleRate = 48000.0f;
    float frequency = 100.0f;
    float reference = 0.0f;
    float previousReference = 0.0f;
    float previousPhase = 0.0f;
    struct sig_osc_RatioPhasor phasor;
    sig_osc_RatioPhasor_init(&phasor);

    // A ratio that is not a fraction of whole numbers advances the phase by
    // the ratio times the reference's increment on every sample, including
    // when the reference wraps.
    for (size_t i = 0; i < 4800; i++) {
        float phase = sig_osc_RatioPhasor_generate(&phasor, reference,
            1.5f, 1.0f, 0.0f);
        if (i > 0) {
            float referenceStep = reference - previousReference;
            referenceStep -= roundf(referenceStep);
            float expected = sig_osc_Oscillator_wrapPhase(
                previousPhase + 1.5f * referenceStep);
            TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.0f,
                phaseDistance(expected, phase));
        }

        previousPhase = phase;
        previousReference = reference;
        sig_osc_Oscillator_accumulatePhase(&reference, frequency, sampleRate);
    }
}

void test_sig_osc_RatioPhasor_followsFractionalTransposition(void) {
    float sampleRate = 48000.0f;
    float frequency = 100.0f;
    float octaves = 0.25f;
    float transposedRatio = 3.0f / 2.0f * powf(2.0f, octaves);
    float reference = 0.0f;
    float previousReference = 0.0f;
    float previousPhase = 0.0f;
    struct sig_osc_RatioPhasor phasor;
    sig_osc_RatioPhasor_init(&phasor);

    // Transposing a whole ratio by part of an octave advances the phase by
    // the transposed ratio times the reference's increment on every sample.
    for (size_t i = 0; i < 4800; i++) {
        float phase = sig_osc_RatioPhasor_generate(&phasor, reference,
            3.0f, 2.0f, octaves);
        if (i > 0) {
            float referenceStep = reference - previousReference;
            referenceStep -= roundf(referenceStep);
            float expected = sig_osc_Oscillator_wrapPhase(
                previousPhase + transposedRatio * referenceStep);
            TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.0f,
                phaseDistance(expected, phase));
        }

        previousPhase = phase;
        previousReference = reference;
        sig_osc_Oscillator_accumulatePhase(&reference, frequency, sampleRate);
    }
}

void test_sig_osc_RatioPhasor_followsDenominatorBelowOne(void) {
    float sampleRate = 48000.0f;
    float reference = 0.0f;
    float previousReference = 0.0f;
    float previousPhase = 0.0f;
    struct sig_osc_RatioPhasor phasor;
    sig_osc_RatioPhasor_init(&phasor);

    // A denominator between 0.0 and 1.0 is not a whole number, so the
    // phasor follows the reference at the ratio, across many reference
    // cycles, without locking.
    for (size_t i = 0; i < 48000; i++) {
        float phase = sig_osc_RatioPhasor_generate(&phasor, reference,
            1.0f, 0.5f, 0.0f);
        if (i > 0) {
            float referenceStep = reference - previousReference;
            referenceStep -= roundf(referenceStep);
            float expected = sig_osc_Oscillator_wrapPhase(
                previousPhase + 2.0f * referenceStep);
            TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.0f,
                phaseDistance(expected, phase));
        }

        previousPhase = phase;
        previousReference = reference;
        sig_osc_Oscillator_accumulatePhase(&reference, 100.0f, sampleRate);
    }
}

void test_sig_osc_RatioPhasor_limitsFrequency(void) {
    float sampleRate = 48000.0f;
    float maxFrequency = 20000.0f;
    float reference = 0.0f;
    float previousPhase = 0.0f;
    struct sig_osc_RatioPhasor phasor;
    sig_osc_RatioPhasor_init(&phasor);
    sig_osc_RatioPhasor_setMaxFrequency(&phasor, maxFrequency, sampleRate);

    // A 100 Hz reference at a ratio of 523:1, transposed up five octaves,
    // would run at about 1.7 MHz. Instead, the phasor runs at its maximum
    // frequency.
    for (size_t i = 0; i < 4800; i++) {
        float phase = sig_osc_RatioPhasor_generate(&phasor, reference,
            523.0f, 1.0f, 5.0f);
        if (i > 0) {
            float step = phase - previousPhase;
            if (step < 0.0f) {
                step += 1.0f;
            }
            TEST_ASSERT_FLOAT_WITHIN(0.00001f, maxFrequency / sampleRate,
                step);
        }

        previousPhase = phase;
        sig_osc_Oscillator_accumulatePhase(&reference, 100.0f, sampleRate);
    }
}

void test_sig_osc_RatioPhasor_limitsToNyquistByDefault(void) {
    float sampleRate = 48000.0f;
    float reference = 0.0f;
    float previousPhase = 0.0f;
    struct sig_osc_RatioPhasor phasor;
    sig_osc_RatioPhasor_init(&phasor);

    // Without a maximum frequency, the phasor advances by at most half a
    // cycle per sample.
    for (size_t i = 0; i < 480; i++) {
        float phase = sig_osc_RatioPhasor_generate(&phasor, reference,
            523.0f, 1.0f, 5.0f);
        if (i > 0) {
            TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.5f,
                phaseDistance(previousPhase, phase));
        }

        previousPhase = phase;
        sig_osc_Oscillator_accumulatePhase(&reference, 100.0f, sampleRate);
    }
}

void test_sig_osc_PeakFrequency_measuresPhaseRate(void) {
    float sampleRate = 48000.0f;
    float phase = 0.0f;
    struct sig_osc_PeakFrequency peak;
    sig_osc_PeakFrequency_init(&peak);
    sig_osc_PeakFrequency_calculateCoefficient(&peak, 0.05f, sampleRate);

    // A phase that ramps at a steady 1 kHz, wrapping on every cycle, has a
    // peak frequency of 1 kHz.
    for (size_t i = 0; i < 4800; i++) {
        float frequency = sig_osc_PeakFrequency_generate(&peak, phase,
            sampleRate);
        if (i > 0) {
            TEST_ASSERT_FLOAT_WITHIN(0.5f, 1000.0f, frequency);
        }
        sig_osc_Oscillator_accumulatePhase(&phase, 1000.0f, sampleRate);
    }
}

void test_sig_osc_PeakFrequency_holdsAndReleasesPeak(void) {
    float sampleRate = 48000.0f;
    float release = 0.05f;
    size_t releaseSamples = 2400;
    float phase = 0.0f;
    struct sig_osc_PeakFrequency peak;
    sig_osc_PeakFrequency_init(&peak);
    sig_osc_PeakFrequency_calculateCoefficient(&peak, release, sampleRate);

    // When a 1 kHz phase slows to 100 Hz, the peak falls from 1 kHz with
    // the release time constant until it reaches 100 Hz.
    for (size_t i = 0; i < releaseSamples + 24000; i++) {
        float frequency = i < releaseSamples ? 1000.0f : 100.0f;
        float result = sig_osc_PeakFrequency_generate(&peak, phase,
            sampleRate);
        if (i == releaseSamples * 2) {
            TEST_ASSERT_FLOAT_WITHIN(2.0f, 1000.0f * expf(-1.0f), result);
        } else if (i == releaseSamples + 24000 - 1) {
            TEST_ASSERT_FLOAT_WITHIN(0.5f, 100.0f, result);
        }
        sig_osc_Oscillator_accumulatePhase(&phase, frequency, sampleRate);
    }
}

void test_sig_snapToInteger(void) {
    // Values within the tolerance of a whole number snap to it exactly,
    // on either side and for negative numbers.
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sig_snapToInteger(0.004f, 0.005f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sig_snapToInteger(-0.004f, 0.005f));
    TEST_ASSERT_EQUAL_FLOAT(2.0f, sig_snapToInteger(1.996f, 0.005f));
    TEST_ASSERT_EQUAL_FLOAT(-3.0f, sig_snapToInteger(-3.003f, 0.005f));

    // Values farther from a whole number pass through unchanged.
    TEST_ASSERT_EQUAL_FLOAT(0.006f, sig_snapToInteger(0.006f, 0.005f));
    TEST_ASSERT_EQUAL_FLOAT(-1.5f, sig_snapToInteger(-1.5f, 0.005f));
    TEST_ASSERT_EQUAL_FLOAT(2.25f, sig_snapToInteger(2.25f, 0.005f));
}

void test_sig_dsp_SnapToInteger(void) {
    struct sig_dsp_SnapToInteger* snap = sig_dsp_SnapToInteger_new(
        &allocator, context);
    float_array_ptr source = sig_AudioBlock_new(&allocator, audioSettings);
    snap->inputs.source = source;
    snap->parameters.tolerance = 0.01f;

    // Each sample within the tolerance of a whole number snaps to it,
    // and the others pass through unchanged.
    float values[4] = {0.995f, 1.5f, -2.008f, 0.02f};
    float expected[4] = {1.0f, 1.5f, -2.0f, 0.02f};
    for (size_t i = 0; i < audioSettings->blockSize; i++) {
        FLOAT_ARRAY(source)[i] = values[i % 4];
    }

    snap->signal.generate(snap);

    for (size_t i = 0; i < audioSettings->blockSize; i++) {
        TEST_ASSERT_EQUAL_FLOAT(expected[i % 4],
            FLOAT_ARRAY(snap->outputs.main)[i]);
    }

    sig_dsp_SnapToInteger_destroy(&allocator, snap);
}

void test_sig_dsp_Phasor(void) {
    float frequency = 440.0f;
    struct sig_dsp_Phasor* phasor = sig_dsp_Phasor_new(&allocator, context);
    phasor->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, frequency);

    // The phase ramps from 0.0 at the frequency, wrapping at 1.0,
    // continuously across blocks.
    for (size_t block = 0; block < 4; block++) {
        phasor->signal.generate(phasor);
        for (size_t i = 0; i < audioSettings->blockSize; i++) {
            double cycles = (double) (block * audioSettings->blockSize + i) *
                frequency / audioSettings->sampleRate;
            float expected = (float) (cycles - floor(cycles));
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, phaseDistance(expected,
                FLOAT_ARRAY(phasor->outputs.main)[i]));
        }
    }

    sig_dsp_Phasor_destroy(&allocator, phasor);
}

void test_sig_dsp_RatioPhasor(void) {
    float frequency = 100.0f;
    struct sig_dsp_Phasor* reference = sig_dsp_Phasor_new(&allocator,
        context);
    reference->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, frequency);

    struct sig_dsp_Value* numerator = sig_dsp_Value_new(&allocator, context);
    numerator->parameters.value = 2.0f;
    struct sig_dsp_RatioPhasor* phasor = sig_dsp_RatioPhasor_new(&allocator,
        context);
    phasor->inputs.reference = reference->outputs.main;
    phasor->inputs.numerator = numerator->outputs.main;
    phasor->inputs.denominator = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 3.0f);
    phasor->parameters.glide = 0.05f;

    // The phasor stays locked at two thirds of the reference's accumulated
    // cycles across blocks.
    size_t numBlocks = 100;
    double referenceCycles = 0.0;
    float previousReference = 0.0f;
    float previousPhase = 0.0f;
    for (size_t block = 0; block < numBlocks * 2; block++) {
        if (block == numBlocks) {
            numerator->parameters.value = 5.0f;
        }

        reference->signal.generate(reference);
        numerator->signal.generate(numerator);
        phasor->signal.generate(phasor);

        for (size_t i = 0; i < audioSettings->blockSize; i++) {
            float referencePhase = FLOAT_ARRAY(reference->outputs.main)[i];
            if (referencePhase < previousReference) {
                referenceCycles += 1.0;
            }
            previousReference = referencePhase;

            float phase = FLOAT_ARRAY(phasor->outputs.main)[i];
            if (block < numBlocks) {
                double cycles = 2.0 / 3.0 * (referenceCycles + referencePhase);
                float expected = (float) (cycles - floor(cycles));
                TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
                    phaseDistance(expected, phase));
            } else if (block == numBlocks && i == 0) {
                // When the numerator changes, the glide keeps the phase
                // continuous.
                TEST_ASSERT_TRUE(phaseDistance(previousPhase, phase) < 0.01f);
            }
            previousPhase = phase;
        }
    }

    sig_dsp_RatioPhasor_destroy(&allocator, phasor);
    sig_dsp_Phasor_destroy(&allocator, reference);
}

void test_sig_dsp_RatioPhasor_transposesByOctaves(void) {
    float frequency = 100.0f;
    struct sig_dsp_Phasor* reference = sig_dsp_Phasor_new(&allocator,
        context);
    reference->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, frequency);

    struct sig_dsp_RatioPhasor* phasor = sig_dsp_RatioPhasor_new(&allocator,
        context);
    phasor->inputs.reference = reference->outputs.main;
    phasor->inputs.numerator = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 2.0f);
    phasor->inputs.denominator = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 3.0f);
    phasor->inputs.octaves = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 1.0f);

    // Two thirds transposed up an octave stays locked at four thirds of
    // the reference's accumulated cycles.
    double referenceCycles = 0.0;
    float previousReference = 0.0f;
    for (size_t block = 0; block < 100; block++) {
        reference->signal.generate(reference);
        phasor->signal.generate(phasor);

        for (size_t i = 0; i < audioSettings->blockSize; i++) {
            float referencePhase = FLOAT_ARRAY(reference->outputs.main)[i];
            if (referencePhase < previousReference) {
                referenceCycles += 1.0;
            }
            previousReference = referencePhase;

            double cycles = 4.0 / 3.0 * (referenceCycles + referencePhase);
            float expected = (float) (cycles - floor(cycles));
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, phaseDistance(expected,
                FLOAT_ARRAY(phasor->outputs.main)[i]));
        }
    }

    sig_dsp_RatioPhasor_destroy(&allocator, phasor);
    sig_dsp_Phasor_destroy(&allocator, reference);
}

void test_sig_dsp_RatioPhasor_limitsFrequency(void) {
    float maxFrequency = 10000.0f;
    struct sig_dsp_Phasor* reference = sig_dsp_Phasor_new(&allocator,
        context);
    reference->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 100.0f);

    struct sig_dsp_RatioPhasor* phasor = sig_dsp_RatioPhasor_new(&allocator,
        context);
    phasor->inputs.reference = reference->outputs.main;
    phasor->inputs.numerator = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 523.0f);
    phasor->inputs.octaves = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 5.0f);
    phasor->parameters.maxFrequency = maxFrequency;

    // Instead of running at about 1.7 MHz, the phasor runs at its maximum
    // frequency, across blocks.
    float expectedStep = maxFrequency / audioSettings->sampleRate;
    float previousPhase = 0.0f;
    for (size_t block = 0; block < 4; block++) {
        reference->signal.generate(reference);
        phasor->signal.generate(phasor);

        for (size_t i = 0; i < audioSettings->blockSize; i++) {
            float phase = FLOAT_ARRAY(phasor->outputs.main)[i];
            if (block > 0 || i > 0) {
                float step = phase - previousPhase;
                if (step < 0.0f) {
                    step += 1.0f;
                }
                TEST_ASSERT_FLOAT_WITHIN(0.00001f, expectedStep, step);
            }
            previousPhase = phase;
        }
    }

    sig_dsp_RatioPhasor_destroy(&allocator, phasor);
    sig_dsp_Phasor_destroy(&allocator, reference);
}

void test_sig_dsp_WavetableReader(void) {
    struct sig_Buffer* sineTable = newSineTable(4096);
    struct sig_dsp_Phasor* phasor = sig_dsp_Phasor_new(&allocator, context);
    phasor->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 440.0f);

    struct sig_dsp_WavetableReader* reader = sig_dsp_WavetableReader_new(
        &allocator, context);
    reader->wavetable = sineTable;
    reader->inputs.phase = phasor->outputs.main;
    reader->inputs.phaseOffset = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.25f);
    reader->inputs.mul = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.5f);
    reader->inputs.add = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.1f);

    // The reader reads the table at the phase plus the phase offset,
    // then applies the gain and offset.
    for (size_t block = 0; block < 4; block++) {
        phasor->signal.generate(phasor);
        reader->signal.generate(reader);
        for (size_t i = 0; i < audioSettings->blockSize; i++) {
            float phase = FLOAT_ARRAY(phasor->outputs.main)[i];
            float expected = 0.5f * sinf(sig_TWOPI * (phase + 0.25f)) + 0.1f;
            TEST_ASSERT_FLOAT_WITHIN(0.001f, expected,
                FLOAT_ARRAY(reader->outputs.main)[i]);
        }
    }

    sig_dsp_WavetableReader_destroy(&allocator, reader);
    sig_dsp_Phasor_destroy(&allocator, phasor);
    sig_Buffer_destroy(&allocator, sineTable);
}

void test_sig_dsp_AdditiveWavetableBankReader(void) {
    struct sig_AdditiveWavetableBank* bank = newStandardWavetableBank(10,
        2048, 20.0f, 20000.0f, sig_HarmonicTaper_NONE);
    float sawIndex = sig_osc_WavetableBank_tableIndex(
        sig_StandardWaveform_SAW, sig_StandardWaveform_COUNT);
    struct sig_dsp_Phasor* phasor = sig_dsp_Phasor_new(&allocator, context);
    phasor->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 100.0f);

    struct sig_dsp_Value* frequency = sig_dsp_Value_new(&allocator, context);
    struct sig_dsp_AdditiveWavetableBankReader* reader =
        sig_dsp_AdditiveWavetableBankReader_new(&allocator, context);
    reader->wavetables = bank;
    reader->inputs.phase = phasor->outputs.main;
    reader->inputs.frequency = frequency->outputs.main;
    reader->inputs.tableIndex = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, sawIndex);

    // The reader reads the waveform at the phase input, from the octave
    // that its frequency input selects, independently of the phase's rate.
    float frequencies[2] = {100.0f, 5000.0f};
    for (size_t f = 0; f < 2; f++) {
        frequency->parameters.value = frequencies[f];
        float level = sig_AdditiveWavetableBank_levelForFrequency(bank,
            frequencies[f]);
        for (size_t block = 0; block < 4; block++) {
            phasor->signal.generate(phasor);
            frequency->signal.generate(frequency);
            reader->signal.generate(reader);
            for (size_t i = 0; i < audioSettings->blockSize; i++) {
                float phase = FLOAT_ARRAY(phasor->outputs.main)[i];
                float expected = sig_AdditiveWavetableBank_readLinearAtPhase(
                    bank, level, (float) sig_StandardWaveform_SAW, phase);
                TEST_ASSERT_FLOAT_WITHIN(0.0001f, expected,
                    FLOAT_ARRAY(reader->outputs.main)[i]);
            }
        }
    }

    sig_dsp_AdditiveWavetableBankReader_destroy(&allocator, reader);
    sig_dsp_Phasor_destroy(&allocator, phasor);
    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

// Measures the mean output of a carrier whose phase is modulated by a sine
// modulator, both driven by one phasor. The ratio starts at 2:1 and
// switches to 1:1 a quarter of the way through a carrier cycle, when the
// two phases are misaligned, and the mean is measured once the glide has
// settled.
float meanOfPhaseLockedModulation(struct sig_AdditiveWavetableBank* bank,
    struct sig_Buffer* sineTable, float tableIndex, float index) {
    float frequency = 50.0f;
    struct sig_dsp_Phasor* phasor = sig_dsp_Phasor_new(&allocator, context);
    phasor->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, frequency);

    struct sig_dsp_Value* numerator = sig_dsp_Value_new(&allocator, context);
    numerator->parameters.value = 2.0f;
    struct sig_dsp_RatioPhasor* ratio = sig_dsp_RatioPhasor_new(&allocator,
        context);
    ratio->inputs.reference = phasor->outputs.main;
    ratio->inputs.numerator = numerator->outputs.main;
    ratio->parameters.glide = 0.05f;

    struct sig_dsp_WavetableReader* modulator = sig_dsp_WavetableReader_new(
        &allocator, context);
    modulator->wavetable = sineTable;
    modulator->inputs.phase = ratio->outputs.main;
    modulator->inputs.mul = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, index);

    struct sig_dsp_AdditiveWavetableBankReader* carrier =
        sig_dsp_AdditiveWavetableBankReader_new(&allocator, context);
    carrier->wavetables = bank;
    carrier->inputs.phase = phasor->outputs.main;
    carrier->inputs.phaseOffset = modulator->outputs.main;
    carrier->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, frequency);
    carrier->inputs.tableIndex = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, tableIndex);

    size_t blocksPerSecond = (size_t) (audioSettings->sampleRate /
        (float) audioSettings->blockSize);
    size_t blocksPerQuarterCycle = (size_t) (audioSettings->sampleRate /
        (4.0f * frequency * (float) audioSettings->blockSize));
    size_t switchBlock = blocksPerSecond / 2 + blocksPerQuarterCycle;
    double sum = 0.0;
    size_t numSamples = 0;
    for (size_t block = 0; block < blocksPerSecond * 2; block++) {
        if (block == switchBlock) {
            numerator->parameters.value = 1.0f;
        }

        phasor->signal.generate(phasor);
        numerator->signal.generate(numerator);
        ratio->signal.generate(ratio);
        modulator->signal.generate(modulator);
        carrier->signal.generate(carrier);

        if (block >= blocksPerSecond) {
            for (size_t i = 0; i < audioSettings->blockSize; i++) {
                sum += FLOAT_ARRAY(carrier->outputs.main)[i];
                numSamples++;
            }
        }
    }

    sig_dsp_AdditiveWavetableBankReader_destroy(&allocator, carrier);
    sig_dsp_WavetableReader_destroy(&allocator, modulator);
    sig_dsp_RatioPhasor_destroy(&allocator, ratio);
    sig_dsp_Phasor_destroy(&allocator, phasor);

    return (float) (sum / (double) numSamples);
}

void test_sig_dsp_AdditiveWavetableBankReader_selectsLevelFromPeak(void) {
    struct sig_AdditiveWavetableBank* bank = newStandardWavetableBank(10,
        2048, 20.0f, 20000.0f, sig_HarmonicTaper_NONE);
    float sawIndex = sig_osc_WavetableBank_tableIndex(
        sig_StandardWaveform_SAW, sig_StandardWaveform_COUNT);
    float phaseFrequency = 5000.0f;
    struct sig_dsp_Phasor* phasor = sig_dsp_Phasor_new(&allocator, context);
    phasor->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, phaseFrequency);

    struct sig_dsp_AdditiveWavetableBankReader* reader =
        sig_dsp_AdditiveWavetableBankReader_new(&allocator, context);
    reader->wavetables = bank;
    reader->inputs.phase = phasor->outputs.main;
    reader->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 100.0f);
    reader->inputs.tableIndex = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, sawIndex);
    reader->parameters.peakRelease = 0.05f;

    // With a peak release time, the reader ignores its frequency input, and
    // selects the level for the rate at which its phase actually advances.
    float level = sig_AdditiveWavetableBank_levelForFrequency(bank,
        phaseFrequency);
    for (size_t block = 0; block < 4; block++) {
        phasor->signal.generate(phasor);
        reader->signal.generate(reader);
        // The first sample has no previous phase to measure from.
        for (size_t i = block == 0 ? 1 : 0; i < audioSettings->blockSize;
            i++) {
            float phase = FLOAT_ARRAY(phasor->outputs.main)[i];
            float expected = sig_AdditiveWavetableBank_readLinearAtPhase(
                bank, level, (float) sig_StandardWaveform_SAW, phase);
            TEST_ASSERT_FLOAT_WITHIN(0.001f, expected,
                FLOAT_ARRAY(reader->outputs.main)[i]);
        }
    }

    sig_dsp_AdditiveWavetableBankReader_destroy(&allocator, reader);
    sig_dsp_Phasor_destroy(&allocator, phasor);
    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

void test_sig_dsp_RatioPhasor_phaseModulationHasNoDC(void) {
    struct sig_AdditiveWavetableBank* bank = newStandardWavetableBank(10,
        2048, 20.0f, 2000.0f, sig_HarmonicTaper_LANCZOS);
    struct sig_Buffer* sineTable = newSineTable(2048);
    float shapes[2] = {
        sig_osc_WavetableBank_tableIndex(sig_StandardWaveform_SINE,
            sig_StandardWaveform_COUNT),
        sig_osc_WavetableBank_tableIndex(sig_StandardWaveform_SQUARE,
            sig_StandardWaveform_COUNT)
    };
    float indices[2] = {0.3f, 0.45f};

    // At a 1:1 ratio, phase modulation with independently running
    // oscillators produces DC of up to about 0.6 for a sine carrier and 0.7
    // for a square, depending on their relative phase. With the modulator
    // locked to the carrier's phase, the output has no DC.
    for (size_t shape = 0; shape < 2; shape++) {
        for (size_t i = 0; i < 2; i++) {
            float mean = meanOfPhaseLockedModulation(bank, sineTable,
                shapes[shape], indices[i]);
            TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, mean);
        }
    }

    sig_Buffer_destroy(&allocator, sineTable);
    sig_AdditiveWavetableBank_destroy(&allocator, bank);
}

void test_sig_dsp_WavetableBankReader(void) {
    size_t tableLength = 2048;
    struct sig_WavetableBank* bank = sig_WavetableBank_new(&allocator, 2,
        tableLength);
    sig_Buffer_fillWithWaveform(bank->waves[0], sig_waveform_sine,
        tableLength, 0.0f, 1.0f);
    sig_Buffer_fillWithWaveform(bank->waves[1], sig_waveform_triangle,
        tableLength, 0.0f, 1.0f);

    struct sig_dsp_Phasor* phasor = sig_dsp_Phasor_new(&allocator, context);
    phasor->inputs.frequency = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 440.0f);

    struct sig_dsp_WavetableBankReader* reader =
        sig_dsp_WavetableBankReader_new(&allocator, context);
    reader->wavetables = bank;
    reader->inputs.phase = phasor->outputs.main;
    reader->inputs.phaseOffset = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.25f);
    reader->inputs.tableIndex = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.25f);
    reader->inputs.mul = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.5f);
    reader->inputs.add = sig_AudioBlock_newWithValue(&allocator,
        audioSettings, 0.1f);

    // The reader reads the bank at the phase plus the phase offset. A table
    // index of 0.25 cycles of a two-table bank crossfades halfway between
    // the sine and the triangle.
    for (size_t block = 0; block < 4; block++) {
        phasor->signal.generate(phasor);
        reader->signal.generate(reader);
        for (size_t i = 0; i < audioSettings->blockSize; i++) {
            float phase = FLOAT_ARRAY(phasor->outputs.main)[i] + 0.25f;
            phase -= floorf(phase);
            float radians = sig_TWOPI * phase;
            float mixed = 0.5f * sig_waveform_sine(radians) +
                0.5f * sig_waveform_triangle(radians);
            TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f * mixed + 0.1f,
                FLOAT_ARRAY(reader->outputs.main)[i]);
        }
    }

    sig_dsp_WavetableBankReader_destroy(&allocator, reader);
    sig_dsp_Phasor_destroy(&allocator, phasor);
    sig_WavetableBank_destroy(&allocator, bank);
}

#define CLOCK_PULSE_WIDTH 5

// Returns the value of a pulse train at a sample, with a pulse that
// starts at each sample listed in pulseStarts.
float clockPulse(size_t sample, const size_t* pulseStarts,
    size_t numPulses) {
    for (size_t i = 0; i < numPulses; i++) {
        if (sample >= pulseStarts[i] &&
            sample < pulseStarts[i] + CLOCK_PULSE_WIDTH) {
            return 1.0f;
        }
    }

    return 0.0f;
}

void test_sig_osc_ClockPhasor_followsPulses(void) {
    struct sig_osc_ClockPhasor phasor;
    sig_osc_ClockPhasor_init(&phasor);
    size_t period = 100;

    // The phasor waits for a second pulse to measure the clock's period.
    // From then on, it completes one cycle per period, wrapping at each
    // pulse.
    for (size_t n = 0; n < period * 10; n++) {
        float pulse = n % period < CLOCK_PULSE_WIDTH ? 1.0f : 0.0f;
        float phase = sig_osc_ClockPhasor_generate(&phasor, pulse, 0.5f);
        float expected = n < period ? 0.0f :
            (float) ((n - period) % period) / (float) period;
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
            phaseDistance(expected, phase));
    }
}

void test_sig_osc_ClockPhasor_spreadsCorrection(void) {
    struct sig_osc_ClockPhasor phasor;
    sig_osc_ClockPhasor_init(&phasor);

    // The clock speeds up from a period of 100 samples to 80.
    size_t pulseStarts[9] = {0, 100, 200, 300, 400, 480, 560, 640, 720};
    float previousPhase = 0.0f;

    for (size_t n = 0; n < 800; n++) {
        float phase = sig_osc_ClockPhasor_generate(&phasor,
            clockPulse(n, pulseStarts, 9), 0.5f);

        if (n == 480) {
            // The early pulse arrives 0.2 cycles ahead of the phasor.
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
                phaseDistance(0.8f, phase));
        }

        // Rather than jumping, the phasor spreads its correction over the
        // next period, advancing 1.2 cycles in 80 samples.
        TEST_ASSERT_TRUE(phaseDistance(previousPhase, phase) <=
            1.2f / 80.0f + 0.0001f);

        // From the next pulse, the phasor is aligned with the clock again.
        if (n >= 560) {
            float expected = (float) ((n - 560) % 80) / 80.0f;
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
                phaseDistance(expected, phase));
        }

        previousPhase = phase;
    }
}

void test_sig_osc_ClockPhasor_slowsWithoutJumping(void) {
    struct sig_osc_ClockPhasor phasor;
    sig_osc_ClockPhasor_init(&phasor);

    // The clock slows down from a period of 100 samples to 120.
    size_t pulseStarts[8] = {0, 100, 200, 300, 400, 520, 640, 760};
    float previousPhase = 0.0f;

    for (size_t n = 0; n < 880; n++) {
        float phase = sig_osc_ClockPhasor_generate(&phasor,
            clockPulse(n, pulseStarts, 8), 0.5f);

        if (n == 520) {
            // The late pulse arrives 0.2 cycles after the phasor wrapped.
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
                phaseDistance(0.2f, phase));
        }

        // The phasor never moves backwards. It slows down instead,
        // advancing 0.8 cycles in the next 120 samples.
        float step = phase - previousPhase;
        if (step < 0.0f) {
            step += 1.0f;
        }
        TEST_ASSERT_TRUE(step <= 0.01f + 0.0001f);

        if (n >= 640) {
            float expected = (float) ((n - 640) % 120) / 120.0f;
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
                phaseDistance(expected, phase));
        }

        previousPhase = phase;
    }
}

void test_sig_osc_ClockPhasor_keepsFrequencyWhenPulsesStop(void) {
    struct sig_osc_ClockPhasor phasor;
    sig_osc_ClockPhasor_init(&phasor);

    // The pulses stop after the sixth.
    size_t pulseStarts[6] = {0, 100, 200, 300, 400, 500};

    // The phasor continues at the clock's last frequency.
    for (size_t n = 0; n < 2000; n++) {
        float phase = sig_osc_ClockPhasor_generate(&phasor,
            clockPulse(n, pulseStarts, 6), 0.5f);
        if (n >= 100) {
            float expected = (float) ((n - 100) % 100) / 100.0f;
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f,
                phaseDistance(expected, phase));
        }
    }
}

// The window and hop lengths that sig_dsp_TempoDetector uses at 48 kHz.
#define TEST_TEMPO_WINDOW_LENGTH 2048
#define TEST_TEMPO_HOP_LENGTH 557
#define TEST_TEMPO_THRESHOLD -70.0f

// Generates one sample of a tone whose partials have amplitudes
// 0.5 / h. Every partial follows the fundamental's vibrato, as the
// partials of an acoustic instrument do. phases contains one phase per
// partial.
static float testTempo_harmonicTone(double time, float sampleRate,
    float fundamental, size_t numHarmonics, float vibratoDepth,
    float vibratoRate, double* phases) {
    float modulation = 1.0f + vibratoDepth *
        sinf(sig_TWOPI * vibratoRate * (float) time);
    float sample = 0.0f;
    for (size_t h = 0; h < numHarmonics; h++) {
        phases[h] += 2.0 * 3.141592653589793 * (double) fundamental *
            (double) (h + 1) * (double) modulation / (double) sampleRate;
        if (phases[h] > 2.0 * 3.141592653589793) {
            phases[h] -= 2.0 * 3.141592653589793;
        }
        sample += (0.5f / (float) (h + 1)) * sinf((float) phases[h]);
    }

    return sample;
}

// Returns the amplitude envelope of a plucked note at the given time
// after its onset: a 2 ms attack and an exponential decay.
static float testTempo_pluckEnvelope(double time, float decay) {
    float attack = (float) (time / 0.002);
    if (attack > 1.0f) {
        attack = 1.0f;
    }

    return attack * expf((float) -time / decay);
}

static void testTempo_clearPhases(double* phases, size_t numPhases) {
    for (size_t i = 0; i < numPhases; i++) {
        phases[i] = 0.0;
    }
}

// Runs a plucked note at 220 Hz over a sustained tone at 330 Hz through
// a detector, and reports the envelope's peak after the onset and the
// time from the onset to the first sample that reports the peak.
static void testTempo_measurePluckOverTone(float sampleRate,
    size_t windowLength, size_t hopLength, float* peak, float* latency) {
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, sampleRate, windowLength,
            hopLength, 2, TEST_TEMPO_THRESHOLD);
    double tonePhases[6];
    double pluckPhases[6];
    testTempo_clearPhases(tonePhases, 6);
    testTempo_clearPhases(pluckPhases, 6);

    size_t onset = (size_t) (sampleRate * 1.0f);
    size_t numSamples = onset + (size_t) (sampleRate * 0.15f);
    *peak = 0.0f;
    size_t peakTime = onset;
    for (size_t i = 0; i < numSamples; i++) {
        float sample = 0.6f * testTempo_harmonicTone((double) i / sampleRate,
            sampleRate, 330.0f, 6, 0.0f, 0.0f, tonePhases);
        if (i >= onset) {
            double time = (double) (i - onset) / sampleRate;
            sample += testTempo_pluckEnvelope(time, 0.3f) *
                testTempo_harmonicTone(time, sampleRate, 220.0f, 6, 0.0f,
                    0.0f, pluckPhases);
        }

        float envelope = sig_tempo_OnsetDetector_generate(detector, sample);
        if (i >= onset && envelope > *peak) {
            *peak = envelope;
            peakTime = i;
        }
    }

    *latency = (float) (peakTime - onset) / sampleRate;
    sig_tempo_OnsetDetector_destroy(&allocator, detector);
}

// Runs a tone with vibrato of plus or minus 3 percent at 5.5 Hz through
// a detector, and returns the envelope's peak after the tone's own
// onset has passed.
static float testTempo_measureVibrato(float fundamental,
    size_t numHarmonics) {
    float sampleRate = audioSettings->sampleRate;
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, sampleRate,
            TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, 2,
            TEST_TEMPO_THRESHOLD);
    double phases[12];
    testTempo_clearPhases(phases, 12);

    size_t settle = (size_t) (sampleRate * 0.5f);
    size_t numSamples = (size_t) (sampleRate * 2.0f);
    float peak = 0.0f;
    for (size_t i = 0; i < numSamples; i++) {
        float sample = 0.5f * testTempo_harmonicTone((double) i / sampleRate,
            sampleRate, fundamental, numHarmonics, 0.03f, 5.5f, phases);
        float envelope = sig_tempo_OnsetDetector_generate(detector, sample);
        if (i >= settle && envelope > peak) {
            peak = envelope;
        }
    }

    sig_tempo_OnsetDetector_destroy(&allocator, detector);

    return peak;
}

// Runs white noise with the given peak amplitude through a detector,
// with plucked notes at -40 dBFS every half second from plucksStart
// seconds onward. Reports the envelope's peak before and after the
// first pluck.
static void testTempo_measurePlucksInNoise(float noiseAmplitude,
    float plucksStart, float threshold, float* noisePeak,
    float* pluckPeak) {
    float sampleRate = audioSettings->sampleRate;
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, sampleRate,
            TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, 2, threshold);
    struct sig_Random* random = sig_Random_new(&allocator, 5);
    double phases[6];
    testTempo_clearPhases(phases, 6);

    size_t firstPluck = (size_t) (sampleRate * plucksStart);
    size_t pluckInterval = (size_t) (sampleRate * 0.5f);
    size_t numSamples = firstPluck + 3 * pluckInterval;
    *noisePeak = 0.0f;
    *pluckPeak = 0.0f;
    for (size_t i = 0; i < numSamples; i++) {
        float sample = noiseAmplitude *
            (2.0f * sig_Random_nextFloat(random) - 1.0f);
        if (i >= firstPluck) {
            size_t position = (i - firstPluck) % pluckInterval;
            if (position == 0) {
                testTempo_clearPhases(phases, 6);
            }
            double time = (double) position / sampleRate;
            sample += 0.01f * testTempo_pluckEnvelope(time, 0.15f) *
                testTempo_harmonicTone(time, sampleRate, 330.0f, 6, 0.0f,
                    0.0f, phases);
        }

        float envelope = sig_tempo_OnsetDetector_generate(detector, sample);
        if (i < firstPluck && envelope > *noisePeak) {
            *noisePeak = envelope;
        }
        if (i >= firstPluck && envelope > *pluckPeak) {
            *pluckPeak = envelope;
        }
    }

    sig_Random_destroy(&allocator, random);
    sig_tempo_OnsetDetector_destroy(&allocator, detector);
}

// Runs a train of single-sample clicks at 120 BPM through a detector,
// and returns the smallest of the envelope's peaks after each click.
static float testTempo_measureClickPeaks(float sampleRate,
    size_t windowLength, size_t hopLength) {
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, sampleRate, windowLength,
            hopLength, 2, TEST_TEMPO_THRESHOLD);
    size_t clickInterval = (size_t) (sampleRate * 0.5f);
    size_t numClicks = 4;
    float smallestPeak = 1.0f;
    float peak = 0.0f;
    for (size_t i = 0; i < numClicks * clickInterval; i++) {
        if (i % clickInterval == 0) {
            if (i > 0 && peak < smallestPeak) {
                smallestPeak = peak;
            }
            peak = 0.0f;
        }
        float sample = i % clickInterval == clickInterval / 2 ? 1.0f : 0.0f;
        float envelope = sig_tempo_OnsetDetector_generate(detector, sample);
        if (envelope > peak) {
            peak = envelope;
        }
    }
    if (peak < smallestPeak) {
        smallestPeak = peak;
    }

    sig_tempo_OnsetDetector_destroy(&allocator, detector);

    return smallestPeak;
}

void test_sig_tempo_OnsetDetector_silence(void) {
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, audioSettings->sampleRate,
            TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, 2,
            TEST_TEMPO_THRESHOLD);

    // One second of silence produces no onset strength.
    size_t numSamples = (size_t) audioSettings->sampleRate;
    for (size_t i = 0; i < numSamples; i++) {
        float envelope = sig_tempo_OnsetDetector_generate(detector, 0.0f);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, envelope);
    }

    sig_tempo_OnsetDetector_destroy(&allocator, detector);
}

void test_sig_tempo_OnsetDetector_detectsOnsets(void) {
    float sampleRate = audioSettings->sampleRate;
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, sampleRate,
            TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, 2,
            TEST_TEMPO_THRESHOLD);

    size_t numSamples = (size_t) (sampleRate * 2.0f);
    float_array_ptr envelope = sig_samples_new(&allocator, numSamples);

    // Clicks every half second, a 120 BPM beat.
    size_t firstClick = (size_t) (sampleRate * 0.25f);
    size_t clickInterval = (size_t) (sampleRate * 0.5f);
    for (size_t i = 0; i < numSamples; i++) {
        float sample = (i >= firstClick &&
            (i - firstClick) % clickInterval == 0) ? 1.0f : 0.0f;
        FLOAT_ARRAY(envelope)[i] =
            sig_tempo_OnsetDetector_generate(detector, sample);
    }

    for (size_t i = 0; i < firstClick; i++) {
        TEST_ASSERT_EQUAL_FLOAT(0.0f, FLOAT_ARRAY(envelope)[i]);
    }

    // The click leaves the frame after one window, and the frame it
    // is compared with two hops later.
    size_t settleTime = TEST_TEMPO_WINDOW_LENGTH +
        3 * TEST_TEMPO_HOP_LENGTH;
    for (size_t click = firstClick; click < numSamples;
        click += clickInterval) {
        // Each click raises the envelope close to its largest value,
        // ln 2...
        float peak = 0.0f;
        for (size_t i = click; i < click + settleTime; i++) {
            float value = FLOAT_ARRAY(envelope)[i];
            TEST_ASSERT_TRUE(value <= sig_LOG2);
            if (value > peak) {
                peak = value;
            }
        }
        TEST_ASSERT_TRUE(peak > 0.5f);

        // ...and the envelope settles back to zero before the next
        // click.
        size_t settleEnd = click + clickInterval;
        if (settleEnd > numSamples) {
            settleEnd = numSamples;
        }
        for (size_t i = click + settleTime; i < settleEnd; i++) {
            TEST_ASSERT_EQUAL_FLOAT(0.0f, FLOAT_ARRAY(envelope)[i]);
        }
    }

    allocator.impl->free(&allocator, envelope);
    sig_tempo_OnsetDetector_destroy(&allocator, detector);
}

void test_sig_tempo_OnsetDetector_ignoresVibrato(void) {
    float pluckPeak;
    float latency;
    testTempo_measurePluckOverTone(audioSettings->sampleRate,
        TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, &pluckPeak,
        &latency);
    float vibratoPeak = testTempo_measureVibrato(220.0f, 4);

    TEST_ASSERT_TRUE(pluckPeak > 0.05f);
    TEST_ASSERT_TRUE(vibratoPeak <= pluckPeak / 10.0f);
}

void test_sig_tempo_OnsetDetector_ignoresVibratoOnRichTone(void) {
    float pluckPeak;
    float latency;
    testTempo_measurePluckOverTone(audioSettings->sampleRate,
        TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, &pluckPeak,
        &latency);
    float vibratoPeak = testTempo_measureVibrato(440.0f, 12);

    TEST_ASSERT_TRUE(pluckPeak > 0.05f);
    TEST_ASSERT_TRUE(vibratoPeak <= pluckPeak / 10.0f);
}

void test_sig_tempo_OnsetDetector_whitensDynamics(void) {
    float sampleRate = audioSettings->sampleRate;
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, sampleRate,
            TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, 2,
            TEST_TEMPO_THRESHOLD);
    double phases[6];

    // Loud plucks, then plucks 26 dB quieter, both at 120 BPM.
    float loudAmplitude = 1.0f;
    float quietAmplitude = 0.05f;
    size_t pluckInterval = (size_t) (sampleRate * 0.5f);
    size_t numLoud = 3 * pluckInterval;
    size_t numQuiet = 5 * pluckInterval;
    size_t numSamples = numLoud + numQuiet;

    float loudPeak = 0.0f;
    float quietPeak = 0.0f;
    size_t quietMeasureStart = numSamples - 2 * pluckInterval;

    for (size_t i = 0; i < numSamples; i++) {
        size_t position = i % pluckInterval;
        if (position == 0) {
            testTempo_clearPhases(phases, 6);
        }
        float amplitude = i < numLoud ? loudAmplitude : quietAmplitude;
        double time = (double) position / sampleRate;
        float sample = amplitude * testTempo_pluckEnvelope(time, 0.15f) *
            testTempo_harmonicTone(time, sampleRate, 330.0f, 6, 0.0f, 0.0f,
                phases);
        float envelope = sig_tempo_OnsetDetector_generate(detector, sample);
        if (i < numLoud && envelope > loudPeak) {
            loudPeak = envelope;
        }
        if (i >= quietMeasureStart && envelope > quietPeak) {
            quietPeak = envelope;
        }
    }
    TEST_ASSERT_TRUE(loudPeak > 0.1f);
    TEST_ASSERT_TRUE(quietPeak > loudPeak / 2.0f);

    sig_tempo_OnsetDetector_destroy(&allocator, detector);
}

void test_sig_tempo_OnsetDetector_ignoresQuietNoise(void) {
    // White noise at -60 dBFS gives a zero envelope at the default
    // threshold, once the noise's own onset has passed.
    float sampleRate = audioSettings->sampleRate;
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, sampleRate,
            TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, 2,
            TEST_TEMPO_THRESHOLD);
    struct sig_Random* random = sig_Random_new(&allocator, 5);

    size_t settle = (size_t) (sampleRate * 0.1f);
    size_t numSamples = (size_t) (sampleRate * 2.0f);
    for (size_t i = 0; i < numSamples; i++) {
        float sample = 0.001f * (2.0f * sig_Random_nextFloat(random) - 1.0f);
        float envelope = sig_tempo_OnsetDetector_generate(detector, sample);
        if (i >= settle) {
            TEST_ASSERT_EQUAL_FLOAT(0.0f, envelope);
        }
    }

    sig_Random_destroy(&allocator, random);
    sig_tempo_OnsetDetector_destroy(&allocator, detector);
}

void test_sig_tempo_OnsetDetector_detectsPlucksInNoise(void) {
    // Plucks at -40 dBFS in white noise at -60 dBFS give clear peaks.
    float noisePeak;
    float pluckPeak;
    testTempo_measurePlucksInNoise(0.001f, 1.0f, TEST_TEMPO_THRESHOLD,
        &noisePeak, &pluckPeak);
    TEST_ASSERT_TRUE(pluckPeak > 0.1f);
}

void test_sig_tempo_OnsetDetector_appliesThreshold(void) {
    // When the threshold rises to -30 dB, the detector ignores plucks
    // at -40 dBFS.
    float noisePeak;
    float pluckPeak;
    testTempo_measurePlucksInNoise(0.0f, 0.2f, -30.0f, &noisePeak,
        &pluckPeak);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pluckPeak);

    // The threshold can also change while the detector runs.
    float sampleRate = audioSettings->sampleRate;
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, sampleRate,
            TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, 2, -30.0f);
    double phases[6];
    size_t pluckInterval = (size_t) (sampleRate * 0.5f);
    float quietPeak = 0.0f;
    float loudPeak = 0.0f;
    for (size_t i = 0; i < 4 * pluckInterval; i++) {
        size_t position = i % pluckInterval;
        if (position == 0) {
            testTempo_clearPhases(phases, 6);
        }
        if (i == 2 * pluckInterval) {
            sig_tempo_OnsetDetector_setThreshold(detector, -70.0f);
        }
        double time = (double) position / sampleRate;
        float sample = 0.01f * testTempo_pluckEnvelope(time, 0.15f) *
            testTempo_harmonicTone(time, sampleRate, 330.0f, 6, 0.0f, 0.0f,
                phases);
        float envelope = sig_tempo_OnsetDetector_generate(detector, sample);
        if (i < 2 * pluckInterval && envelope > quietPeak) {
            quietPeak = envelope;
        }
        if (i >= 2 * pluckInterval && envelope > loudPeak) {
            loudPeak = envelope;
        }
    }

    TEST_ASSERT_EQUAL_FLOAT(0.0f, quietPeak);
    TEST_ASSERT_TRUE(loudPeak > 0.1f);

    sig_tempo_OnsetDetector_destroy(&allocator, detector);
}

void test_sig_tempo_OnsetDetector_detectsPluckOverToneQuickly(void) {
    float peak;
    float latency;
    testTempo_measurePluckOverTone(audioSettings->sampleRate,
        TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, &peak, &latency);

    TEST_ASSERT_TRUE(peak > 0.05f);
    TEST_ASSERT_TRUE(latency <= 0.03f);
}

void test_sig_tempo_OnsetDetector_matchesAcrossSampleRates(void) {
    // The node's window and hop lengths at each sample rate.
    float peak48 = testTempo_measureClickPeaks(48000.0f, 2048, 557);
    float peak441 = testTempo_measureClickPeaks(44100.0f, 2048, 512);
    float peak96 = testTempo_measureClickPeaks(96000.0f, 4096, 1115);

    TEST_ASSERT_TRUE(peak48 > 0.5f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f * peak48, peak48, peak441);
    TEST_ASSERT_FLOAT_WITHIN(0.1f * peak48, peak48, peak96);

    float pluckPeak441;
    float pluckPeak96;
    float latency;
    testTempo_measurePluckOverTone(44100.0f, 2048, 512, &pluckPeak441,
        &latency);
    testTempo_measurePluckOverTone(96000.0f, 4096, 1115, &pluckPeak96,
        &latency);
    TEST_ASSERT_FLOAT_WITHIN(0.25f * pluckPeak441, pluckPeak441,
        pluckPeak96);
}

#define TEST_TEMPO_NUM_HAT_PARTIALS 32

// Synthesizes kick-like thumps and hi-hat-like noise bursts. A thump
// is a sine that falls from 150 to 50 Hz, with a 5 ms attack, so its
// energy lies below about 200 Hz. A burst is a sum of sines at random
// frequencies from 7 to 15 kHz, with a 2 ms attack.
struct testTempo_Drums {
    float sampleRate;
    float hatFrequencies[TEST_TEMPO_NUM_HAT_PARTIALS];
    float hatPhases[TEST_TEMPO_NUM_HAT_PARTIALS];
    double kickPhase;
    float kickScale;
    float hatScale;
};

static float testTempo_Drums_kick(struct testTempo_Drums* self,
    double time) {
    if (time == 0.0) {
        self->kickPhase = 0.0;
    }
    double frequency = 50.0 + 100.0 * exp(-time / 0.03);
    self->kickPhase += 2.0 * 3.141592653589793 * frequency /
        (double) self->sampleRate;
    float attack = (float) (time / 0.005);
    if (attack > 1.0f) {
        attack = 1.0f;
    }

    return self->kickScale * attack * expf((float) -time / 0.15f) *
        sinf((float) self->kickPhase);
}

static float testTempo_Drums_hat(struct testTempo_Drums* self, double time,
    size_t sample) {
    float attack = (float) (time / 0.002);
    if (attack > 1.0f) {
        attack = 1.0f;
    }
    float sum = 0.0f;
    for (size_t k = 0; k < TEST_TEMPO_NUM_HAT_PARTIALS; k++) {
        double cycles = (double) self->hatFrequencies[k] * (double) sample /
            (double) self->sampleRate;
        sum += sinf(self->hatPhases[k] +
            sig_TWOPI * (float) (cycles - floor(cycles)));
    }

    return self->hatScale * attack * expf((float) -time / 0.03f) * sum;
}

// Chooses the bursts' partials, and scales the thumps and bursts so
// that each has a peak level of 1.
static void testTempo_Drums_init(struct testTempo_Drums* self,
    float sampleRate) {
    struct sig_Random* random = sig_Random_new(&allocator, 3);
    self->sampleRate = sampleRate;
    for (size_t k = 0; k < TEST_TEMPO_NUM_HAT_PARTIALS; k++) {
        self->hatFrequencies[k] = 7000.0f +
            8000.0f * sig_Random_nextFloat(random);
        self->hatPhases[k] = sig_TWOPI * sig_Random_nextFloat(random);
    }
    sig_Random_destroy(&allocator, random);

    self->kickScale = 1.0f;
    self->hatScale = 1.0f;
    float kickPeak = 0.0f;
    float hatPeak = 0.0f;
    for (size_t i = 0; i < (size_t) (0.25f * sampleRate); i++) {
        double time = (double) i / sampleRate;
        float kick = fabsf(testTempo_Drums_kick(self, time));
        float hat = fabsf(testTempo_Drums_hat(self, time, i));
        kickPeak = kick > kickPeak ? kick : kickPeak;
        hatPeak = hat > hatPeak ? hat : hatPeak;
    }
    self->kickScale = 1.0f / kickPeak;
    self->hatScale = 1.0f / hatPeak;
}

void test_sig_tempo_OnsetDetector_favoursLowFrequencies(void) {
    // Noise bursts above 6 kHz give much smaller peaks than kick-like
    // thumps of the same peak level, each at 120 BPM.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Drums drums;
    testTempo_Drums_init(&drums, sampleRate);
    size_t interval = (size_t) (0.5f * sampleRate);
    float peaks[2] = {0.0f, 0.0f};

    for (size_t kind = 0; kind < 2; kind++) {
        struct sig_tempo_OnsetDetector* detector =
            sig_tempo_OnsetDetector_new(&allocator, sampleRate,
                TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, 2,
                TEST_TEMPO_THRESHOLD);
        for (size_t i = 0; i < 4 * interval; i++) {
            double time = (double) (i % interval) / sampleRate;
            float sample = 0.5f * (kind == 0 ?
                testTempo_Drums_kick(&drums, time) :
                testTempo_Drums_hat(&drums, time, i));
            float envelope = sig_tempo_OnsetDetector_generate(detector,
                sample);
            if (i >= interval && envelope > peaks[kind]) {
                peaks[kind] = envelope;
            }
        }
        sig_tempo_OnsetDetector_destroy(&allocator, detector);
    }

    TEST_ASSERT_TRUE(peaks[0] > 0.05f);
    TEST_ASSERT_TRUE(peaks[1] < peaks[0] / 4.0f);
}

void test_sig_tempo_OnsetDetector_reportsNewValues(void) {
    // The detector completes one analysis per hop.
    struct sig_tempo_OnsetDetector* detector =
        sig_tempo_OnsetDetector_new(&allocator, audioSettings->sampleRate,
            TEST_TEMPO_WINDOW_LENGTH, TEST_TEMPO_HOP_LENGTH, 2,
            TEST_TEMPO_THRESHOLD);

    for (size_t i = 0; i < 4 * TEST_TEMPO_HOP_LENGTH; i++) {
        sig_tempo_OnsetDetector_generate(detector, 0.0f);
        bool isHopEnd = (i + 1) % TEST_TEMPO_HOP_LENGTH == 0;
        TEST_ASSERT_EQUAL(isHopEnd,
            sig_tempo_OnsetDetector_hasNewValue(detector));
    }

    sig_tempo_OnsetDetector_destroy(&allocator, detector);
}

void test_sig_tempo_OnsetDetector_countsBands(void) {
    // 24 bands per octave from 30 Hz to 6 kHz give 101 bands with a
    // 2048-sample window at 48 kHz, and 104 at 44.1 kHz, because the
    // lowest filters merge where their centres share an FFT bin.
    TEST_ASSERT_EQUAL(101,
        sig_tempo_OnsetDetector_calcNumBands(48000.0f, 2048));
    TEST_ASSERT_EQUAL(104,
        sig_tempo_OnsetDetector_calcNumBands(44100.0f, 2048));

    // At 11.025 kHz, with the same bin width as at 44.1 kHz, the bands
    // stop at 0.45 times the sample rate.
    TEST_ASSERT_TRUE(sig_tempo_OnsetDetector_calcNumBands(11025.0f, 512) <
        sig_tempo_OnsetDetector_calcNumBands(44100.0f, 2048));
}

void test_sig_tempo_TempoEstimator_tracksTempo(void) {
    // One impulse per beat of a 120 BPM beat, at an envelope rate of
    // 86 values per second, gives one impulse every 43 values.
    float envelopeRate = 86.0f;
    size_t beatInterval = 43;
    size_t numValues = 258;

    struct sig_tempo_TempoEstimator* estimator =
        sig_tempo_TempoEstimator_new(&allocator, envelopeRate, 30.0f,
            200.0f);

    float tempo = 0.0f;
    for (size_t i = 0; i < numValues; i++) {
        float value = (i % beatInterval == 0) ? 1.0f : 0.0f;
        tempo = sig_tempo_TempoEstimator_generate(estimator, value);
    }

    TEST_ASSERT_FLOAT_WITHIN(3.0f, 120.0f, tempo);

    sig_tempo_TempoEstimator_destroy(&allocator, estimator);
}

void test_sig_tempo_TempoEstimator_tracksSlowTempo(void) {
    // One impulse every 86 values is 60 BPM at 86 values per second.
    float envelopeRate = 86.0f;
    size_t beatInterval = 86;
    size_t numValues = 344;

    struct sig_tempo_TempoEstimator* estimator =
        sig_tempo_TempoEstimator_new(&allocator, envelopeRate, 30.0f,
            200.0f);

    float tempo = 0.0f;
    for (size_t i = 0; i < numValues; i++) {
        float value = (i % beatInterval == 0) ? 1.0f : 0.0f;
        tempo = sig_tempo_TempoEstimator_generate(estimator, value);
    }

    TEST_ASSERT_FLOAT_WITHIN(3.0f, 60.0f, tempo);

    sig_tempo_TempoEstimator_destroy(&allocator, estimator);
}

void test_sig_tempo_TempoEstimator_followsTempoChange(void) {
    // 120 BPM for 3 seconds, then 90 BPM for 8 seconds, which fills
    // the window, at an envelope rate of 86 values per second.
    float envelopeRate = 86.0f;
    size_t fastInterval = 43;
    size_t slowInterval = 57;
    size_t numFast = 258;
    size_t numSlow = 688;
    size_t numValues = numFast + numSlow;

    struct sig_tempo_TempoEstimator* estimator =
        sig_tempo_TempoEstimator_new(&allocator, envelopeRate, 30.0f,
            200.0f);

    float tempo = 0.0f;
    float tempoBeforeChange = 0.0f;
    for (size_t i = 0; i < numValues; i++) {
        bool isFast = i < numFast;
        size_t interval = isFast ? fastInterval : slowInterval;
        size_t position = isFast ? i : i - numFast;
        float value = (position % interval == 0) ? 1.0f : 0.0f;
        tempo = sig_tempo_TempoEstimator_generate(estimator, value);
        if (i == numFast - 43) {
            tempoBeforeChange = tempo;
        }
    }

    // The estimate tracks the fast tempo before the change...
    TEST_ASSERT_FLOAT_WITHIN(3.0f, 120.0f, tempoBeforeChange);
    // ...and the slow tempo once the window contains only the slow
    // tempo.
    TEST_ASSERT_FLOAT_WITHIN(3.0f, 90.0f, tempo);

    sig_tempo_TempoEstimator_destroy(&allocator, estimator);
}

void test_sig_tempo_TempoEstimator_holdsTempoInSilence(void) {
    // 120 BPM for 3 seconds, then silence for 2 seconds, at an
    // envelope rate of 86 values per second.
    float envelopeRate = 86.0f;
    size_t beatInterval = 43;
    size_t numBeats = 258;
    size_t numValues = 430;

    struct sig_tempo_TempoEstimator* estimator =
        sig_tempo_TempoEstimator_new(&allocator, envelopeRate, 30.0f,
            200.0f);

    float tempo = 0.0f;
    for (size_t i = 0; i < numValues; i++) {
        float value = (i < numBeats && i % beatInterval == 0) ? 1.0f : 0.0f;
        tempo = sig_tempo_TempoEstimator_generate(estimator, value);
    }

    // The estimate holds through the silence.
    TEST_ASSERT_FLOAT_WITHIN(3.0f, 120.0f, tempo);

    sig_tempo_TempoEstimator_destroy(&allocator, estimator);
}

void test_sig_tempo_TempoEstimator_holdsEstablishedLevel(void) {
    // Clicks at 120 BPM for 10 seconds, then 10 seconds in which every
    // other click is 26 dB quieter, at an envelope rate of 86 values
    // per second. The accented clicks raise the 60 BPM level's score
    // above 1.3 times, but not above 2 times, the 120 BPM level's
    // score.
    float envelopeRate = 86.0f;
    size_t beatInterval = 43;
    size_t numSteady = 860;
    size_t numValues = 1720;

    struct sig_tempo_TempoEstimator* estimator =
        sig_tempo_TempoEstimator_new(&allocator, envelopeRate, 30.0f,
            200.0f);

    float tempo = 0.0f;
    float tempoBeforeAccents = 0.0f;
    for (size_t i = 0; i < numValues; i++) {
        float value = 0.0f;
        if (i % beatInterval == 0) {
            bool isQuiet = i >= numSteady && (i / beatInterval) % 2 == 1;
            value = isQuiet ? 0.05f : 1.0f;
        }
        tempo = sig_tempo_TempoEstimator_generate(estimator, value);
        if (i == numSteady - 1) {
            tempoBeforeAccents = tempo;
        }

        // The established estimate stays at 120 BPM throughout.
        if (i >= numSteady) {
            TEST_ASSERT_FLOAT_WITHIN(3.0f, 120.0f, tempo);
        }
    }

    TEST_ASSERT_FLOAT_WITHIN(3.0f, 120.0f, tempoBeforeAccents);

    sig_tempo_TempoEstimator_destroy(&allocator, estimator);
}

void test_sig_tempo_TempoEstimator_followsTempoAfterLongRun(void) {
    // Clicks of random strength at 120 BPM over low random noise for 10
    // minutes, then clicks at 120 BPM without noise for 9 seconds, then
    // 10 seconds of silence, then clicks at 90 BPM for 9 seconds, at an
    // envelope rate of 86 values per second. The clicks without noise
    // replace the noise in the window before the silence, because the
    // estimator follows noise when its window contains no pairs of
    // clicks.
    float envelopeRate = 86.0f;
    size_t fastInterval = 43;
    size_t slowInterval = 57;
    size_t numNoisy = 51600;
    size_t numFast = numNoisy + 774;
    size_t numSilent = 860;
    size_t numSlow = 774;
    struct sig_Random* random = sig_Random_new(&allocator, 7);

    struct sig_tempo_TempoEstimator* estimator =
        sig_tempo_TempoEstimator_new(&allocator, envelopeRate, 30.0f,
            200.0f);

    float tempo = 0.0f;
    for (size_t i = 0; i < numFast; i++) {
        bool isNoisy = i < numNoisy;
        float value = 0.0f;
        if (i % fastInterval == 0) {
            value = isNoisy ? 0.3f + 0.7f * sig_Random_nextFloat(random) :
                1.0f;
        } else if (isNoisy) {
            value = 0.02f * sig_Random_nextFloat(random);
        }
        tempo = sig_tempo_TempoEstimator_generate(estimator, value);
    }
    TEST_ASSERT_FLOAT_WITHIN(3.0f, 120.0f, tempo);

    // The estimate holds through the silence.
    for (size_t i = 0; i < numSilent; i++) {
        tempo = sig_tempo_TempoEstimator_generate(estimator, 0.0f);
    }
    TEST_ASSERT_FLOAT_WITHIN(3.0f, 120.0f, tempo);

    // The estimator finds the new tempo once the window contains it.
    for (size_t i = 0; i < numSlow; i++) {
        float value = i % slowInterval == 0 ? 1.0f : 0.0f;
        tempo = sig_tempo_TempoEstimator_generate(estimator, value);
    }
    TEST_ASSERT_FLOAT_WITHIN(3.0f, 90.0f, tempo);

    sig_tempo_TempoEstimator_destroy(&allocator, estimator);
    sig_Random_destroy(&allocator, random);
}

void test_sig_tempo_TempoEstimator_restrictsTempoRange(void) {
    // One impulse every 86 values is 60 BPM at 86 values per second.
    float envelopeRate = 86.0f;
    size_t beatInterval = 86;
    size_t numValues = 344;

    struct sig_tempo_TempoEstimator* estimator =
        sig_tempo_TempoEstimator_new(&allocator, envelopeRate, 30.0f,
            200.0f);

    float tempo = 0.0f;
    for (size_t i = 0; i < numValues; i++) {
        float value = (i % beatInterval == 0) ? 1.0f : 0.0f;
        tempo = sig_tempo_TempoEstimator_generate(estimator, value);
    }
    TEST_ASSERT_FLOAT_WITHIN(3.0f, 60.0f, tempo);

    // When the range excludes the estimate, the estimator chooses the
    // best tempo in the new range at its next estimation.
    sig_tempo_TempoEstimator_setTempoRange(estimator, 80.0f, 160.0f);
    for (size_t i = numValues; i < numValues + 32; i++) {
        float value = (i % beatInterval == 0) ? 1.0f : 0.0f;
        tempo = sig_tempo_TempoEstimator_generate(estimator, value);
    }
    TEST_ASSERT_FLOAT_WITHIN(3.0f, 120.0f, tempo);

    sig_tempo_TempoEstimator_destroy(&allocator, estimator);
}

#define TEST_TEMPO_MAX_BEATS 64
#define TEST_TEMPO_MAX_WRAPS 128
#define TEST_TEMPO_MAX_TAPS 8

// A synthetic performance for a beat tracker: an envelope value and a
// tempo estimate for each hop, and the beats' sample times.
struct testTempo_Performance {
    float sampleRate;
    size_t hopLength;
    size_t numHops;
    float_array_ptr envelope;
    float_array_ptr tempo;
    double beatTimes[TEST_TEMPO_MAX_BEATS];
    size_t numBeats;
    double tapTimes[TEST_TEMPO_MAX_TAPS];
    size_t numTaps;
};

// The sample times at which a beat tracker's phase wrapped, and the
// number of samples on which the phase fell without wrapping.
struct testTempo_TrackerRun {
    double wrapTimes[TEST_TEMPO_MAX_WRAPS];
    size_t numWraps;
    size_t numBackwardSteps;
};

typedef float (*testTempo_TempoSchedule)(double time);

static float testTempo_steadyTempo(double time) {
    (void) time;
    return 120.0f;
}

// 120 BPM slowing to 90 BPM over 8 seconds, from the first beat.
static float testTempo_ritardandoTempo(double time) {
    if (time < 0.25) {
        return 120.0f;
    }
    if (time < 8.25) {
        return 120.0f - 30.0f * (float) (time - 0.25) / 8.0f;
    }

    return 90.0f;
}

// 120 BPM until the beat at 8.25 seconds, and 100 BPM after it.
static float testTempo_stepTempo(double time) {
    return time < 8.2 ? 120.0f : 100.0f;
}

static void testTempo_Performance_init(struct testTempo_Performance* self,
    float sampleRate, size_t hopLength, float duration,
    testTempo_TempoSchedule tempoAt) {
    self->sampleRate = sampleRate;
    self->hopLength = hopLength;
    self->numHops = (size_t) (duration * sampleRate / (float) hopLength);
    self->envelope = sig_samples_new(&allocator, self->numHops);
    self->tempo = sig_samples_new(&allocator, self->numHops);
    self->numBeats = 0;
    self->numTaps = 0;
    sig_fillWithSilence(self->envelope, self->numHops);
    for (size_t hop = 0; hop < self->numHops; hop++) {
        double time = (double) ((hop + 1) * hopLength) / sampleRate;
        FLOAT_ARRAY(self->tempo)[hop] = tempoAt(time);
    }
}

static void testTempo_Performance_destroy(
    struct testTempo_Performance* self) {
    allocator.impl->free(&allocator, self->envelope);
    allocator.impl->free(&allocator, self->tempo);
}

// Adds an onset at a time in seconds. As in the onset detector's
// envelope, the onset raises the two envelope values whose frames
// first contain it to the same value. Value i describes the hop that
// ends at sample (i + 1) * hopLength, so the centre of the two values
// lies one hop after the onset, on average.
static void testTempo_Performance_addOnset(
    struct testTempo_Performance* self, double time, float strength) {
    size_t index = (size_t) ceil(time * self->sampleRate /
        (double) self->hopLength) - 1;
    if (index + 1 >= self->numHops) {
        return;
    }

    FLOAT_ARRAY(self->envelope)[index] += strength;
    FLOAT_ARRAY(self->envelope)[index + 1] += strength;
}

// Adds beats of the given strength from start to end seconds, spaced by
// the schedule's tempo at each beat, with off-beats of the given
// strength halfway between them when offBeatStrength is above zero.
static void testTempo_Performance_addBeats(
    struct testTempo_Performance* self, double start, double end,
    testTempo_TempoSchedule tempoAt, float beatStrength,
    float offBeatStrength) {
    double time = start;
    while (time < end && self->numBeats < TEST_TEMPO_MAX_BEATS) {
        double interval = 60.0 / (double) tempoAt(time);
        testTempo_Performance_addOnset(self, time, beatStrength);
        self->beatTimes[self->numBeats] = time * self->sampleRate;
        self->numBeats++;
        if (offBeatStrength > 0.0f) {
            testTempo_Performance_addOnset(self, time + 0.5 * interval,
                offBeatStrength);
        }
        time += interval;
    }
}

// Adds a tap at a time in seconds. Taps must be added in time order.
static void testTempo_Performance_addTap(
    struct testTempo_Performance* self, double time) {
    if (self->numTaps < TEST_TEMPO_MAX_TAPS) {
        self->tapTimes[self->numTaps] = time;
        self->numTaps++;
    }
}

// Runs a beat tracker over a performance, advancing the follower once
// per sample, tapping it at the performance's taps, and passing it each
// envelope value at the end of its hop. When states, confidences, or
// tempos are not NULL, they receive the tracker's state, confidence, and
// tempo after each hop.
static void testTempo_runTracker(struct sig_tempo_BeatTracker* tracker,
    struct testTempo_Performance* performance,
    struct testTempo_TrackerRun* run, float* states, float* confidences,
    float* tempos) {
    size_t nextTap = 0;
    run->numWraps = 0;
    run->numBackwardSteps = 0;
    float previousPhase = 0.0f;
    size_t sampleTime = 0;
    for (size_t hop = 0; hop < performance->numHops; hop++) {
        for (size_t i = 0; i < performance->hopLength; i++) {
            sampleTime++;
            float phase = sig_tempo_BeatTracker_follow(tracker);
            if (nextTap < performance->numTaps && sampleTime ==
                (size_t) round(performance->tapTimes[nextTap] *
                    performance->sampleRate)) {
                sig_tempo_BeatTracker_tap(tracker);
                nextTap++;
            }
            if (previousPhase - phase > 0.5f) {
                if (run->numWraps < TEST_TEMPO_MAX_WRAPS) {
                    run->wrapTimes[run->numWraps] = (double) sampleTime;
                    run->numWraps++;
                }
            } else if (phase < previousPhase) {
                run->numBackwardSteps++;
            }
            previousPhase = phase;
        }

        sig_tempo_BeatTracker_analyse(tracker,
            FLOAT_ARRAY(performance->envelope)[hop],
            FLOAT_ARRAY(performance->tempo)[hop]);
        if (states != NULL) {
            states[hop] = (float) sig_tempo_BeatTracker_state(tracker);
        }
        if (confidences != NULL) {
            confidences[hop] = sig_tempo_BeatTracker_confidence(tracker);
        }
        if (tempos != NULL) {
            tempos[hop] = sig_tempo_BeatTracker_tempo(tracker);
        }
    }
}

// Returns the signed distance in samples from a time to the nearest
// phase wrap.
static double testTempo_nearestWrapError(struct testTempo_TrackerRun* run,
    double time) {
    double nearest = 1.0e12;
    for (size_t w = 0; w < run->numWraps; w++) {
        double error = run->wrapTimes[w] - time;
        if (fabs(error) < fabs(nearest)) {
            nearest = error;
        }
    }

    return nearest;
}

// Asserts that the phase never fell without wrapping, and that no two
// wraps fell closer together than 0.6 of the shortest beat interval.
static void testTempo_assertOneWrapPerBeat(struct testTempo_TrackerRun* run,
    double shortestInterval) {
    TEST_ASSERT_EQUAL(0, run->numBackwardSteps);
    for (size_t w = 1; w < run->numWraps; w++) {
        TEST_ASSERT_TRUE(run->wrapTimes[w] - run->wrapTimes[w - 1] >
            0.6 * shortestInterval);
    }
}

// Asserts that each beat from firstBeat onward has a wrap within
// maxError seconds, and returns the RMS error in seconds.
static double testTempo_assertBeatsFollowed(struct testTempo_TrackerRun* run,
    struct testTempo_Performance* performance, size_t firstBeat,
    double maxError) {
    double sumOfSquares = 0.0;
    size_t count = 0;
    for (size_t b = firstBeat; b < performance->numBeats; b++) {
        double error = testTempo_nearestWrapError(run,
            performance->beatTimes[b]) / performance->sampleRate;
        TEST_ASSERT_TRUE(fabs(error) <= maxError);
        sumOfSquares += error * error;
        count++;
    }
    TEST_ASSERT_TRUE(count > 0);

    return sqrt(sumOfSquares / (double) count);
}

// Returns the index of the first beat at or after a time in seconds.
static size_t testTempo_firstBeatAfter(
    struct testTempo_Performance* performance, double time) {
    size_t b = 0;
    while (b < performance->numBeats &&
        performance->beatTimes[b] < time * performance->sampleRate) {
        b++;
    }

    return b;
}

static size_t testTempo_hopAt(struct testTempo_Performance* performance,
    double time) {
    return (size_t) (time * performance->sampleRate /
        (double) performance->hopLength);
}

static struct sig_tempo_BeatTracker* testTempo_newTracker(float sampleRate) {
    // The synthetic envelopes place each onset one hop early, on
    // average.
    return sig_tempo_BeatTracker_new(&allocator, sampleRate,
        TEST_TEMPO_HOP_LENGTH, (float) TEST_TEMPO_HOP_LENGTH, 30.0f, 200.0f);
}

void test_sig_tempo_BeatTracker_runsAtEstimatedTempo(void) {
    // With no onsets, the phase runs at the estimated tempo.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 3.0f, testTempo_steadyTempo);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_WAITING,
        sig_tempo_BeatTracker_state(tracker));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sig_tempo_BeatTracker_tempo(tracker));

    testTempo_runTracker(tracker, &performance, &run, NULL, NULL, NULL);

    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_SEARCHING,
        sig_tempo_BeatTracker_state(tracker));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 120.0f,
        sig_tempo_BeatTracker_tempo(tracker));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sig_tempo_BeatTracker_confidence(tracker));
    TEST_ASSERT_TRUE(run.numWraps >= 5);
    for (size_t w = 1; w < run.numWraps; w++) {
        TEST_ASSERT_FLOAT_WITHIN(2.0, 0.5 * sampleRate,
            run.wrapTimes[w] - run.wrapTimes[w - 1]);
    }
    testTempo_assertOneWrapPerBeat(&run, 0.5 * sampleRate);

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_locksToSteadyBeats(void) {
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 12.0f, testTempo_steadyTempo);
    testTempo_Performance_addBeats(&performance, 0.25, 12.0,
        testTempo_steadyTempo, 1.0f, 0.0f);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, NULL, NULL, NULL);

    // Once locked, each beat has a wrap within 30 ms.
    testTempo_assertBeatsFollowed(&run, &performance,
        testTempo_firstBeatAfter(&performance, 3.0), 0.03);
    testTempo_assertOneWrapPerBeat(&run, 0.5 * sampleRate);
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_TRACKING,
        sig_tempo_BeatTracker_state(tracker));
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 120.0f,
        sig_tempo_BeatTracker_tempo(tracker));

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_followsRitardando(void) {
    // The tempo estimate follows the ritardando, as the estimator
    // would after converging.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 11.0f, testTempo_ritardandoTempo);
    testTempo_Performance_addBeats(&performance, 0.25, 11.0,
        testTempo_ritardandoTempo, 1.0f, 0.0f);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, NULL, NULL, NULL);

    // Each beat after the first three has a wrap within 100 ms, and
    // the RMS error stays below 60 ms.
    double rmsError = testTempo_assertBeatsFollowed(&run, &performance, 3,
        0.1);
    TEST_ASSERT_TRUE(rmsError < 0.06);
    testTempo_assertOneWrapPerBeat(&run, 0.5 * sampleRate);

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_followsTempoStep(void) {
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 14.0f, testTempo_stepTempo);
    testTempo_Performance_addBeats(&performance, 0.25, 14.0,
        testTempo_stepTempo, 1.0f, 0.0f);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, NULL, NULL, NULL);

    // From the fifth beat after the step, each beat has a wrap within
    // 40 ms.
    size_t firstAfterStep = testTempo_firstBeatAfter(&performance, 8.3);
    testTempo_assertBeatsFollowed(&run, &performance, firstAfterStep + 4,
        0.04);
    testTempo_assertOneWrapPerBeat(&run, 0.5 * sampleRate);
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 100.0f,
        sig_tempo_BeatTracker_tempo(tracker));

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_keepsTempoThroughGap(void) {
    // Beats at 120 BPM stop at 6 seconds, and resume at 10 seconds, 0.3
    // beats late.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 14.0f, testTempo_steadyTempo);
    testTempo_Performance_addBeats(&performance, 0.25, 6.0,
        testTempo_steadyTempo, 1.0f, 0.0f);
    size_t firstResumed = performance.numBeats;
    testTempo_Performance_addBeats(&performance, 10.15, 14.0,
        testTempo_steadyTempo, 1.0f, 0.0f);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, NULL, NULL, NULL);

    // Through the gap, the phase keeps its tempo.
    size_t numGapWraps = 0;
    for (size_t w = 1; w < run.numWraps; w++) {
        if (run.wrapTimes[w - 1] > 6.5 * sampleRate &&
            run.wrapTimes[w] < 10.0 * sampleRate) {
            TEST_ASSERT_FLOAT_WITHIN(0.005 * sampleRate, 0.5 * sampleRate,
                run.wrapTimes[w] - run.wrapTimes[w - 1]);
            numGapWraps++;
        }
    }
    TEST_ASSERT_TRUE(numGapWraps >= 5);

    // From the third beat after the resumption, each beat has a wrap
    // within 40 ms.
    testTempo_assertBeatsFollowed(&run, &performance, firstResumed + 2,
        0.04);
    testTempo_assertOneWrapPerBeat(&run, 0.5 * sampleRate);

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_ignoresOffBeats(void) {
    // Off-beat onsets at 0.6 of the beats' strength do not move the
    // phase.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 12.0f, testTempo_steadyTempo);
    testTempo_Performance_addBeats(&performance, 0.25, 12.0,
        testTempo_steadyTempo, 1.0f, 0.6f);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, NULL, NULL, NULL);

    testTempo_assertBeatsFollowed(&run, &performance,
        testTempo_firstBeatAfter(&performance, 3.0), 0.03);
    testTempo_assertOneWrapPerBeat(&run, 0.5 * sampleRate);

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_reportsSilence(void) {
    // Beats at 120 BPM until 5 seconds, then silence until 9 seconds,
    // then beats again.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 11.0f, testTempo_steadyTempo);
    testTempo_Performance_addBeats(&performance, 0.25, 5.0,
        testTempo_steadyTempo, 1.0f, 0.0f);
    testTempo_Performance_addBeats(&performance, 9.0, 11.0,
        testTempo_steadyTempo, 1.0f, 0.0f);
    float_array_ptr states = sig_samples_new(&allocator,
        performance.numHops);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, FLOAT_ARRAY(states),
        NULL, NULL);

    // The last beat sounds at 4.75 seconds. With the default timeout of
    // four beats, the follower is still tracking three and a half beats
    // later, and silent five beats later.
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_TRACKING,
        FLOAT_ARRAY(states)[testTempo_hopAt(&performance, 4.9)]);
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_TRACKING,
        FLOAT_ARRAY(states)[testTempo_hopAt(&performance, 6.5)]);
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_SILENT,
        FLOAT_ARRAY(states)[testTempo_hopAt(&performance, 7.25)]);
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_SILENT,
        FLOAT_ARRAY(states)[testTempo_hopAt(&performance, 8.9)]);
    // The first strong beat after the silence returns it to tracking.
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_TRACKING,
        sig_tempo_BeatTracker_state(tracker));
    sig_tempo_BeatTracker_destroy(&allocator, tracker);

    // With a timeout of eight beats, the follower is still tracking
    // five beats after the last beat.
    tracker = testTempo_newTracker(sampleRate);
    sig_tempo_BeatTracker_setSilenceTimeout(tracker, 8.0f);
    testTempo_runTracker(tracker, &performance, &run, FLOAT_ARRAY(states),
        NULL, NULL);
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_TRACKING,
        FLOAT_ARRAY(states)[testTempo_hopAt(&performance, 7.25)]);
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_SILENT,
        FLOAT_ARRAY(states)[testTempo_hopAt(&performance, 9.2)]);

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    allocator.impl->free(&allocator, states);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_reportsConfidence(void) {
    // Beats at 120 BPM until 8 seconds, then silence.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 12.0f, testTempo_steadyTempo);
    testTempo_Performance_addBeats(&performance, 0.25, 8.0,
        testTempo_steadyTempo, 1.0f, 0.0f);
    float_array_ptr confidences = sig_samples_new(&allocator,
        performance.numHops);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, NULL,
        FLOAT_ARRAY(confidences), NULL);

    // The confidence starts at zero, rises toward 1 on steady beats...
    TEST_ASSERT_EQUAL_FLOAT(0.0f, FLOAT_ARRAY(confidences)[0]);
    float early = FLOAT_ARRAY(confidences)[testTempo_hopAt(&performance,
        2.0)];
    float late = FLOAT_ARRAY(confidences)[testTempo_hopAt(&performance,
        8.0)];
    TEST_ASSERT_TRUE(early > 0.0f);
    TEST_ASSERT_TRUE(late > early);
    TEST_ASSERT_TRUE(late > 0.9f);
    for (size_t hop = 0; hop < testTempo_hopAt(&performance, 8.0); hop++) {
        TEST_ASSERT_TRUE(FLOAT_ARRAY(confidences)[hop] <= 1.0f);
    }

    // ...and falls to zero in silence.
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sig_tempo_BeatTracker_confidence(tracker));

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    allocator.impl->free(&allocator, confidences);
    testTempo_Performance_destroy(&performance);
}

static float testTempo_slowTempo(double time) {
    (void) time;
    return 100.0f;
}

static float testTempo_noTempo(double time) {
    (void) time;
    return 0.0f;
}

void test_sig_tempo_BeatTracker_tapMovesPhaseOntoBeats(void) {
    // Off-beats are stronger than the beats, so the follower locks to
    // them. A tap on the beat at 6.25 seconds moves it to the beats.
    // With the beats at 0.85 of the off-beats' strength or below, the
    // follower locks to the off-beats, and with the beats at 0.7 or
    // above, the tap holds.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 16.0f, testTempo_steadyTempo);
    testTempo_Performance_addBeats(&performance, 0.25, 16.0,
        testTempo_steadyTempo, 0.75f, 1.0f);
    testTempo_Performance_addTap(&performance, 6.25);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, NULL, NULL, NULL);

    // Before the tap, the wraps fall on the off-beats...
    size_t beforeTap = testTempo_firstBeatAfter(&performance, 5.0);
    TEST_ASSERT_TRUE(fabs(testTempo_nearestWrapError(&run,
        performance.beatTimes[beforeTap])) > 0.2 * sampleRate);

    // ...and from the second beat after the tap, once the output has
    // closed its gap to the grid, for at least 16 beats, they fall
    // within 30 ms of the beats.
    size_t afterTap = testTempo_firstBeatAfter(&performance, 7.0);
    TEST_ASSERT_TRUE(performance.numBeats - afterTap >= 16);
    testTempo_assertBeatsFollowed(&run, &performance, afterTap, 0.03);
    testTempo_assertOneWrapPerBeat(&run, 0.5 * sampleRate);

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_tapsSetTempo(void) {
    // Beats and taps at 100 BPM, while the estimate is 120 BPM.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 10.0f, testTempo_steadyTempo);
    testTempo_Performance_addBeats(&performance, 0.5, 10.0,
        testTempo_slowTempo, 1.0f, 0.0f);
    for (size_t t = 0; t < 4; t++) {
        testTempo_Performance_addTap(&performance, 0.5 + 0.6 * (double) t);
    }
    float_array_ptr tempos = sig_samples_new(&allocator,
        performance.numHops);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, NULL, NULL,
        FLOAT_ARRAY(tempos));

    // The taps set the tempo, and the follower stays at it.
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 100.0f,
        FLOAT_ARRAY(tempos)[testTempo_hopAt(&performance, 1.15)]);
    for (size_t hop = testTempo_hopAt(&performance, 1.15);
        hop < performance.numHops; hop++) {
        TEST_ASSERT_FLOAT_WITHIN(1.0f, 100.0f, FLOAT_ARRAY(tempos)[hop]);
    }
    testTempo_assertBeatsFollowed(&run, &performance, 2, 0.03);

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    allocator.impl->free(&allocator, tempos);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_twoTapsStartFollower(void) {
    // With no tempo estimate and no onsets, taps at 0.5 and 1 seconds
    // start the follower at 120 BPM.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 4.0f, testTempo_noTempo);
    testTempo_Performance_addTap(&performance, 0.5);
    testTempo_Performance_addTap(&performance, 1.0);
    float_array_ptr states = sig_samples_new(&allocator,
        performance.numHops);
    float_array_ptr tempos = sig_samples_new(&allocator,
        performance.numHops);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, FLOAT_ARRAY(states),
        NULL, FLOAT_ARRAY(tempos));

    // The first tap only starts an interval...
    size_t betweenTaps = testTempo_hopAt(&performance, 0.75);
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_WAITING,
        FLOAT_ARRAY(states)[betweenTaps]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, FLOAT_ARRAY(tempos)[betweenTaps]);

    // ...and the second sets the tempo and starts the follower, which
    // wraps on the tapped beats.
    size_t afterTaps = testTempo_hopAt(&performance, 1.25);
    TEST_ASSERT_EQUAL(sig_tempo_BeatTracker_State_TRACKING,
        FLOAT_ARRAY(states)[afterTaps]);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 120.0f, FLOAT_ARRAY(tempos)[afterTaps]);
    TEST_ASSERT_TRUE(run.numWraps >= 5);
    for (size_t w = 0; w < run.numWraps; w++) {
        TEST_ASSERT_TRUE(run.wrapTimes[w] > 1.0 * sampleRate);
        double offset = fmod(run.wrapTimes[w] - 1.0 * sampleRate,
            0.5 * sampleRate);
        TEST_ASSERT_TRUE(offset < 2.0 || offset > 0.5 * sampleRate - 2.0);
    }

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    allocator.impl->free(&allocator, states);
    allocator.impl->free(&allocator, tempos);
    testTempo_Performance_destroy(&performance);
}

void test_sig_tempo_BeatTracker_reanchorsAfterSilenceFollowingTaps(void) {
    // A tap at 3.25 seconds, then beats stop at 6 seconds and resume at
    // 10 seconds, 0.3 beats late.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Performance performance;
    testTempo_Performance_init(&performance, sampleRate,
        TEST_TEMPO_HOP_LENGTH, 14.0f, testTempo_steadyTempo);
    testTempo_Performance_addBeats(&performance, 0.25, 6.0,
        testTempo_steadyTempo, 1.0f, 0.0f);
    size_t firstResumed = performance.numBeats;
    testTempo_Performance_addBeats(&performance, 10.15, 14.0,
        testTempo_steadyTempo, 1.0f, 0.0f);
    testTempo_Performance_addTap(&performance, 3.25);
    struct sig_tempo_BeatTracker* tracker = testTempo_newTracker(sampleRate);
    struct testTempo_TrackerRun run;

    testTempo_runTracker(tracker, &performance, &run, NULL, NULL, NULL);

    // From the third beat after the resumption, each beat has a wrap
    // within 40 ms.
    testTempo_assertBeatsFollowed(&run, &performance, firstResumed + 2,
        0.04);
    testTempo_assertOneWrapPerBeat(&run, 0.5 * sampleRate);

    sig_tempo_BeatTracker_destroy(&allocator, tracker);
    testTempo_Performance_destroy(&performance);
}

#define TEST_TEMPO_MAX_TRIGGERS 32

// The rising edges and lengths of the clock triggers that a clock
// generator produced.
struct testTempo_ClockRun {
    size_t triggerTimes[TEST_TEMPO_MAX_TRIGGERS];
    size_t triggerLengths[TEST_TEMPO_MAX_TRIGGERS];
    size_t numTriggers;
    float maxPhase;
};

// Runs a clock generator over a phase ramp at 120 BPM, with a constant
// follower state and confidence. From switchTime onward, the tempo
// input changes to switchTempo.
static void testTempo_runClock(struct sig_tempo_ClockGenerator* generator,
    enum sig_tempo_BeatTracker_State state, size_t numSamples,
    size_t switchTime, float switchTempo, struct testTempo_ClockRun* run) {
    struct sig_tempo_ClockGenerator_Outputs outputs;
    float period = 0.5f * audioSettings->sampleRate;
    float previousClock = 0.0f;
    run->numTriggers = 0;
    run->maxPhase = 0.0f;
    for (size_t i = 0; i < numSamples; i++) {
        float phase = (float) i / period;
        phase -= floorf(phase);
        float tempo = i < switchTime ? 120.0f : switchTempo;
        sig_tempo_ClockGenerator_generate(generator, phase, tempo, 0.75f,
            state, &outputs);

        TEST_ASSERT_EQUAL_FLOAT(tempo, outputs.bpm);
        TEST_ASSERT_EQUAL_FLOAT(0.75f, outputs.confidence);
        if (outputs.phase > run->maxPhase) {
            run->maxPhase = outputs.phase;
        }
        if (outputs.clock > 0.5f && previousClock <= 0.5f &&
            run->numTriggers < TEST_TEMPO_MAX_TRIGGERS) {
            run->triggerTimes[run->numTriggers] = i;
            run->triggerLengths[run->numTriggers] = 0;
            run->numTriggers++;
        }
        if (outputs.clock > 0.5f && run->numTriggers > 0) {
            run->triggerLengths[run->numTriggers - 1]++;
        }
        previousClock = outputs.clock;
    }
}

void test_sig_tempo_ClockGenerator_firesEarlyByLead(void) {
    float sampleRate = audioSettings->sampleRate;
    size_t numSamples = (size_t) (2.2f * sampleRate);
    struct sig_tempo_ClockGenerator generator;
    struct testTempo_ClockRun onBeat;
    struct testTempo_ClockRun early;

    sig_tempo_ClockGenerator_init(&generator, sampleRate);
    testTempo_runClock(&generator, sig_tempo_BeatTracker_State_TRACKING,
        numSamples, numSamples, 120.0f, &onBeat);

    sig_tempo_ClockGenerator_init(&generator, sampleRate);
    sig_tempo_ClockGenerator_setLead(&generator, 0.1f);
    testTempo_runClock(&generator, sig_tempo_BeatTracker_State_TRACKING,
        numSamples, numSamples, 120.0f, &early);

    // Without a lead, the clock fires when the phase wraps, every half
    // second.
    TEST_ASSERT_EQUAL(4, onBeat.numTriggers);
    for (size_t t = 0; t < onBeat.numTriggers; t++) {
        TEST_ASSERT_INT_WITHIN(1, (t + 1) * 24000, onBeat.triggerTimes[t]);
    }

    // With a lead of 100 ms, it fires 100 ms earlier.
    TEST_ASSERT_EQUAL(4, early.numTriggers);
    for (size_t t = 0; t < onBeat.numTriggers; t++) {
        TEST_ASSERT_INT_WITHIN(1, onBeat.triggerTimes[t] - 4800,
            early.triggerTimes[t]);
    }
}

void test_sig_tempo_ClockGenerator_firesOncePerBeat(void) {
    // With a lead of 200 ms, a fall in tempo from 120 to 100 BPM just
    // after a trigger steps the shifted phase back across the wrap.
    float sampleRate = audioSettings->sampleRate;
    size_t numSamples = (size_t) (2.2f * sampleRate);
    struct sig_tempo_ClockGenerator generator;
    struct testTempo_ClockRun run;

    sig_tempo_ClockGenerator_init(&generator, sampleRate);
    sig_tempo_ClockGenerator_setLead(&generator, 0.2f);
    // The first trigger fires 200 ms before the phase wraps at 0.5 s.
    size_t switchTime = 24000 - 9600 + 100;
    testTempo_runClock(&generator, sig_tempo_BeatTracker_State_TRACKING,
        numSamples, switchTime, 100.0f, &run);

    TEST_ASSERT_TRUE(run.numTriggers >= 4);
    for (size_t t = 1; t < run.numTriggers; t++) {
        TEST_ASSERT_TRUE(run.triggerTimes[t] - run.triggerTimes[t - 1] >
            12000);
    }
}

void test_sig_tempo_ClockGenerator_holdsTriggerDuration(void) {
    float sampleRate = audioSettings->sampleRate;
    size_t numSamples = (size_t) (1.2f * sampleRate);
    struct sig_tempo_ClockGenerator generator;
    struct testTempo_ClockRun run;

    // The default trigger lasts 10 ms.
    sig_tempo_ClockGenerator_init(&generator, sampleRate);
    testTempo_runClock(&generator, sig_tempo_BeatTracker_State_TRACKING,
        numSamples, numSamples, 120.0f, &run);
    TEST_ASSERT_EQUAL(2, run.numTriggers);
    for (size_t t = 0; t < run.numTriggers; t++) {
        TEST_ASSERT_EQUAL(480, run.triggerLengths[t]);
    }

    sig_tempo_ClockGenerator_init(&generator, sampleRate);
    sig_tempo_ClockGenerator_setTriggerDuration(&generator, 0.002f);
    testTempo_runClock(&generator, sig_tempo_BeatTracker_State_TRACKING,
        numSamples, numSamples, 120.0f, &run);
    TEST_ASSERT_EQUAL(2, run.numTriggers);
    for (size_t t = 0; t < run.numTriggers; t++) {
        TEST_ASSERT_EQUAL(96, run.triggerLengths[t]);
    }
}

void test_sig_tempo_ClockGenerator_followsSilenceBehaviour(void) {
    float sampleRate = audioSettings->sampleRate;
    size_t numSamples = (size_t) (1.2f * sampleRate);
    struct sig_tempo_ClockGenerator generator;
    struct testTempo_ClockRun run;

    // By default, the clock mutes while the follower is silent...
    sig_tempo_ClockGenerator_init(&generator, sampleRate);
    testTempo_runClock(&generator, sig_tempo_BeatTracker_State_SILENT,
        numSamples, numSamples, 120.0f, &run);
    TEST_ASSERT_EQUAL(0, run.numTriggers);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, run.maxPhase);

    // ...and free-runs when the silence behaviour is 1.
    sig_tempo_ClockGenerator_setSilenceBehaviour(&generator, 1.0f);
    testTempo_runClock(&generator, sig_tempo_BeatTracker_State_SILENT,
        numSamples, numSamples, 120.0f, &run);
    TEST_ASSERT_EQUAL(2, run.numTriggers);
    TEST_ASSERT_TRUE(run.maxPhase > 0.99f);

    // The clock always mutes while the follower is searching.
    testTempo_runClock(&generator, sig_tempo_BeatTracker_State_SEARCHING,
        numSamples, numSamples, 120.0f, &run);
    TEST_ASSERT_EQUAL(0, run.numTriggers);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, run.maxPhase);
}

#define TEST_TEMPO_MAX_NOTES 64
#define TEST_TEMPO_MAX_NODE_TRIGGERS 64

// A synthesized performance for a tempo detector: clicks or plucked
// notes at given times and amplitudes, optionally over a drone with
// vibrato.
struct testTempo_Score {
    float sampleRate;
    double noteTimes[TEST_TEMPO_MAX_NOTES];
    float noteAmplitudes[TEST_TEMPO_MAX_NOTES];
    size_t numNotes;
    bool isPlucked;
    float droneAmplitude;
    size_t nextNote;
    size_t noteStart;
    float noteAmplitude;
    double notePhases[6];
    double dronePhases[6];
};

// What a tempo detector's outputs did during a run.
struct testTempo_NodeRun {
    double triggerTimes[TEST_TEMPO_MAX_NODE_TRIGGERS];
    size_t numTriggers;
    float maxPhaseInWindow;
    float bpm;
    float confidence;
};

static void testTempo_Score_init(struct testTempo_Score* self,
    float sampleRate, bool isPlucked, float droneAmplitude) {
    self->sampleRate = sampleRate;
    self->numNotes = 0;
    self->isPlucked = isPlucked;
    self->droneAmplitude = droneAmplitude;
    self->nextNote = 0;
    self->noteStart = 0;
    self->noteAmplitude = 0.0f;
    testTempo_clearPhases(self->notePhases, 6);
    testTempo_clearPhases(self->dronePhases, 6);
}

// Adds notes from start to end seconds, spaced by interval seconds.
static void testTempo_Score_addNotes(struct testTempo_Score* self,
    double start, double end, double interval, float amplitude) {
    for (double time = start; time < end &&
        self->numNotes < TEST_TEMPO_MAX_NOTES; time += interval) {
        self->noteTimes[self->numNotes] = time;
        self->noteAmplitudes[self->numNotes] = amplitude;
        self->numNotes++;
    }
}

static float testTempo_Score_generate(struct testTempo_Score* self,
    size_t sample) {
    float sampleRate = self->sampleRate;
    float output = 0.0f;
    bool isNoteStart = false;
    if (self->nextNote < self->numNotes && sample ==
        (size_t) round(self->noteTimes[self->nextNote] * sampleRate)) {
        self->noteStart = sample;
        self->noteAmplitude = self->noteAmplitudes[self->nextNote];
        testTempo_clearPhases(self->notePhases, 6);
        self->nextNote++;
        isNoteStart = true;
    }

    if (!self->isPlucked) {
        output = isNoteStart ? self->noteAmplitude : 0.0f;
    } else if (self->nextNote > 0) {
        double time = (double) (sample - self->noteStart) / sampleRate;
        output = self->noteAmplitude * testTempo_pluckEnvelope(time, 0.3f) *
            testTempo_harmonicTone(time, sampleRate, 220.0f, 6, 0.0f, 0.0f,
                self->notePhases);
    }

    if (self->droneAmplitude > 0.0f) {
        output += self->droneAmplitude * testTempo_harmonicTone(
            (double) sample / sampleRate, sampleRate, 110.0f, 6, 0.03f, 5.5f,
            self->dronePhases);
    }

    return output;
}

// Runs a tempo detector over a score, and records the clock's rising
// edges, the largest phase from windowStart to windowEnd seconds, and
// the final tempo and confidence.
static void testTempo_runDetector(struct sig_dsp_TempoDetector* detector,
    struct testTempo_Score* score, double duration, double windowStart,
    double windowEnd, struct testTempo_NodeRun* run) {
    struct sig_AudioSettings* settings = detector->signal.audioSettings;
    float_array_ptr source = sig_AudioBlock_newSilent(&allocator, settings);
    detector->inputs.source = source;
    size_t numBlocks = (size_t) (duration * settings->sampleRate /
        (double) settings->blockSize);
    size_t sample = 0;
    float previousClock = 0.0f;
    run->numTriggers = 0;
    run->maxPhaseInWindow = 0.0f;

    for (size_t block = 0; block < numBlocks; block++) {
        for (size_t i = 0; i < settings->blockSize; i++) {
            FLOAT_ARRAY(source)[i] = testTempo_Score_generate(score,
                sample + i);
        }

        detector->signal.generate(detector);

        for (size_t i = 0; i < settings->blockSize; i++) {
            double time = (double) sample / settings->sampleRate;
            float clock = FLOAT_ARRAY(detector->outputs.clock)[i];
            float phase = FLOAT_ARRAY(detector->outputs.phase)[i];
            if (clock > 0.5f && previousClock <= 0.5f &&
                run->numTriggers < TEST_TEMPO_MAX_NODE_TRIGGERS) {
                run->triggerTimes[run->numTriggers] = time;
                run->numTriggers++;
            }
            if (time >= windowStart && time < windowEnd &&
                phase > run->maxPhaseInWindow) {
                run->maxPhaseInWindow = phase;
            }
            previousClock = clock;
            sample++;
        }
    }

    run->bpm = FLOAT_ARRAY(detector->outputs.bpm)[settings->blockSize - 1];
    run->confidence =
        FLOAT_ARRAY(detector->outputs.confidence)[settings->blockSize - 1];
    sig_AudioBlock_destroy(&allocator, source);
}

static double testTempo_nearestTime(const double* times, size_t numTimes,
    double time) {
    double nearest = 1.0e9;
    for (size_t i = 0; i < numTimes; i++) {
        if (fabs(times[i] - time) < fabs(nearest - time)) {
            nearest = times[i];
        }
    }

    return nearest;
}

// Asserts that, from a time in seconds onward, each note has a trigger
// within maxError seconds, and each trigger has a note within maxError
// seconds.
static void testTempo_assertTriggersOnNotes(struct testTempo_NodeRun* run,
    struct testTempo_Score* score, double from, double maxError) {
    size_t numChecked = 0;
    for (size_t n = 0; n < score->numNotes; n++) {
        double note = score->noteTimes[n];
        if (note < from) {
            continue;
        }
        double trigger = testTempo_nearestTime(run->triggerTimes,
            run->numTriggers, note);
        TEST_ASSERT_TRUE(fabs(trigger - note) <= maxError);
        numChecked++;
    }
    TEST_ASSERT_TRUE(numChecked > 0);

    for (size_t t = 0; t < run->numTriggers; t++) {
        double trigger = run->triggerTimes[t];
        if (trigger < from) {
            continue;
        }
        double note = testTempo_nearestTime(score->noteTimes,
            score->numNotes, trigger);
        TEST_ASSERT_TRUE(fabs(trigger - note) <= maxError);
    }
}

// Checks that a tempo detector follows a click track at 120 BPM at a
// sample rate.
static void testTempo_checkClickTrack(float sampleRate) {
    struct sig_AudioSettings* localSettings =
        sig_AudioSettings_new(&allocator);
    localSettings->sampleRate = sampleRate;
    struct sig_SignalContext* localContext =
        sig_SignalContext_new(&allocator, localSettings);
    struct sig_dsp_TempoDetector* detector =
        sig_dsp_TempoDetector_new(&allocator, localContext);
    struct testTempo_Score score;
    struct testTempo_NodeRun run;

    testTempo_Score_init(&score, sampleRate, false, 0.0f);
    testTempo_Score_addNotes(&score, 0.25, 9.0, 0.5, 1.0f);
    testTempo_runDetector(detector, &score, 9.0, 0.0, 0.0, &run);

    // Once locked, each click has a trigger within 30 ms.
    testTempo_assertTriggersOnNotes(&run, &score, 4.0, 0.03);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 120.0f, run.bpm);
    TEST_ASSERT_TRUE(run.confidence > 0.5f);

    sig_dsp_TempoDetector_destroy(&allocator, detector);
    sig_SignalContext_destroy(&allocator, localContext);
    sig_AudioSettings_destroy(&allocator, localSettings);
}

void test_sig_dsp_TempoDetector_tracksClickTrack(void) {
    testTempo_checkClickTrack(48000.0f);
}

void test_sig_dsp_TempoDetector_tracksPluckedTones(void) {
    // Plucked notes at 100 BPM over a drone with vibrato.
    float sampleRate = audioSettings->sampleRate;
    struct sig_dsp_TempoDetector* detector =
        sig_dsp_TempoDetector_new(&allocator, context);
    struct testTempo_Score score;
    struct testTempo_NodeRun run;

    testTempo_Score_init(&score, sampleRate, true, 0.3f);
    testTempo_Score_addNotes(&score, 0.3, 12.0, 0.6, 0.6f);
    testTempo_runDetector(detector, &score, 12.0, 0.0, 0.0, &run);

    TEST_ASSERT_FLOAT_WITHIN(2.0f, 100.0f, run.bpm);
    testTempo_assertTriggersOnNotes(&run, &score, 6.0, 0.03);
    for (size_t t = 1; t < run.numTriggers; t++) {
        if (run.triggerTimes[t - 1] >= 6.0) {
            TEST_ASSERT_FLOAT_WITHIN(0.03, 0.6,
                run.triggerTimes[t] - run.triggerTimes[t - 1]);
        }
    }

    sig_dsp_TempoDetector_destroy(&allocator, detector);
}

void test_sig_dsp_TempoDetector_followsSilenceBehaviour(void) {
    // Clicks at 120 BPM stop at 6 seconds. The follower becomes silent
    // about four and a half beats after the last click.
    float sampleRate = audioSettings->sampleRate;
    struct testTempo_Score score;
    struct testTempo_NodeRun run;

    // By default, the clock mutes in silence.
    struct sig_dsp_TempoDetector* detector =
        sig_dsp_TempoDetector_new(&allocator, context);
    testTempo_Score_init(&score, sampleRate, false, 0.0f);
    testTempo_Score_addNotes(&score, 0.25, 6.0, 0.5, 1.0f);
    testTempo_runDetector(detector, &score, 10.0, 8.5, 10.0, &run);

    TEST_ASSERT_TRUE(run.numTriggers > 6);
    TEST_ASSERT_TRUE(run.triggerTimes[run.numTriggers - 1] < 8.5);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, run.maxPhaseInWindow);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, run.confidence);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 120.0f, run.bpm);
    sig_dsp_TempoDetector_destroy(&allocator, detector);

    // When silenceBehaviour is 1, the clock keeps running at the tempo.
    detector = sig_dsp_TempoDetector_new(&allocator, context);
    detector->parameters.silenceBehaviour = 1.0f;
    testTempo_Score_init(&score, sampleRate, false, 0.0f);
    testTempo_Score_addNotes(&score, 0.25, 6.0, 0.5, 1.0f);
    testTempo_runDetector(detector, &score, 10.0, 8.5, 10.0, &run);

    size_t numSilentTriggers = 0;
    for (size_t t = 1; t < run.numTriggers; t++) {
        if (run.triggerTimes[t - 1] >= 8.5) {
            TEST_ASSERT_FLOAT_WITHIN(0.01, 0.5,
                run.triggerTimes[t] - run.triggerTimes[t - 1]);
            numSilentTriggers++;
        }
    }
    TEST_ASSERT_TRUE(numSilentTriggers >= 2);
    TEST_ASSERT_TRUE(run.maxPhaseInWindow > 0.9f);
    sig_dsp_TempoDetector_destroy(&allocator, detector);
}

void test_sig_dsp_TempoDetector_tracksQuietPassage(void) {
    // Plucked notes at 120 BPM drop from -6 dBFS to -40 dBFS at 5
    // seconds.
    float sampleRate = audioSettings->sampleRate;
    struct sig_dsp_TempoDetector* detector =
        sig_dsp_TempoDetector_new(&allocator, context);
    struct testTempo_Score score;
    struct testTempo_NodeRun run;

    testTempo_Score_init(&score, sampleRate, true, 0.0f);
    testTempo_Score_addNotes(&score, 0.25, 5.0, 0.5, 1.0f);
    testTempo_Score_addNotes(&score, 5.25, 12.0, 0.5, 0.02f);
    testTempo_runDetector(detector, &score, 12.0, 0.0, 0.0, &run);

    testTempo_assertTriggersOnNotes(&run, &score, 8.0, 0.03);

    sig_dsp_TempoDetector_destroy(&allocator, detector);
}

void test_sig_dsp_TempoDetector_followsKicksUnderHiHats(void) {
    // Hi-hat-like bursts on the off-beats at 120 BPM start at 0.25
    // seconds, and kick-like thumps on the beats, at half the bursts'
    // peak level, start at 0.5 seconds.
    float sampleRate = audioSettings->sampleRate;
    size_t blockSize = audioSettings->blockSize;
    struct sig_dsp_TempoDetector* detector =
        sig_dsp_TempoDetector_new(&allocator, context);
    float_array_ptr source = sig_AudioBlock_newSilent(&allocator,
        audioSettings);
    detector->inputs.source = source;
    struct testTempo_Drums drums;
    testTempo_Drums_init(&drums, sampleRate);
    size_t interval = (size_t) (0.5f * sampleRate);
    size_t numBlocks = (size_t) (12.0f * sampleRate) / blockSize;
    float previousClock = 0.0f;
    size_t numTriggers = 0;

    for (size_t block = 0; block < numBlocks; block++) {
        for (size_t i = 0; i < blockSize; i++) {
            size_t sample = block * blockSize + i;
            float value = 0.0f;
            if (sample >= interval) {
                value += 0.25f * testTempo_Drums_kick(&drums,
                    (double) (sample % interval) / sampleRate);
            }
            if (sample >= interval / 2) {
                value += 0.5f * testTempo_Drums_hat(&drums,
                    (double) ((sample + interval / 2) % interval) /
                        sampleRate, sample);
            }
            FLOAT_ARRAY(source)[i] = value;
        }

        detector->signal.generate(detector);

        for (size_t i = 0; i < blockSize; i++) {
            float clock = FLOAT_ARRAY(detector->outputs.clock)[i];
            double time = (double) (block * blockSize + i) / sampleRate;
            if (clock > 0.5f && previousClock <= 0.5f && time >= 4.0) {
                // Each trigger falls within 30 ms of a thump.
                double offset = fmod(time, 0.5);
                if (offset > 0.25) {
                    offset -= 0.5;
                }
                TEST_ASSERT_TRUE(fabs(offset) <= 0.03);
                numTriggers++;
            }
            previousClock = clock;
        }
    }

    TEST_ASSERT_TRUE(numTriggers >= 15);

    sig_AudioBlock_destroy(&allocator, source);
    sig_dsp_TempoDetector_destroy(&allocator, detector);
}

void test_sig_dsp_TempoDetector_followsSyncTaps(void) {
    // Loud clicks on the off-beats and quieter clicks on the beats at
    // 120 BPM, with taps on the sync input on the beats at 6.25 and 6.75
    // seconds.
    float sampleRate = audioSettings->sampleRate;
    size_t blockSize = audioSettings->blockSize;
    struct sig_dsp_TempoDetector* detector =
        sig_dsp_TempoDetector_new(&allocator, context);
    float_array_ptr source = sig_AudioBlock_newSilent(&allocator,
        audioSettings);
    float_array_ptr sync = sig_AudioBlock_newSilent(&allocator,
        audioSettings);
    detector->inputs.source = source;
    detector->inputs.sync = sync;
    size_t interval = (size_t) (0.5f * sampleRate);
    size_t numBlocks = (size_t) (16.0f * sampleRate) / blockSize;
    size_t tapLength = (size_t) (0.01f * sampleRate);
    float previousClock = 0.0f;
    size_t numOffBeatTriggers = 0;
    size_t numBeatTriggers = 0;

    for (size_t block = 0; block < numBlocks; block++) {
        for (size_t i = 0; i < blockSize; i++) {
            size_t sample = block * blockSize + i;
            size_t position = (sample + interval / 2) % interval;
            float value = 0.0f;
            if (sample >= interval / 2 && position == 0) {
                value = 0.5f;
            } else if (sample >= interval / 2 &&
                position == interval / 2) {
                value = 1.0f;
            }
            FLOAT_ARRAY(source)[i] = value;

            bool isTapping = (sample >= 12 * interval + interval / 2 &&
                sample < 12 * interval + interval / 2 + tapLength) ||
                (sample >= 13 * interval + interval / 2 &&
                sample < 13 * interval + interval / 2 + tapLength);
            FLOAT_ARRAY(sync)[i] = isTapping ? 1.0f : 0.0f;
        }

        detector->signal.generate(detector);

        for (size_t i = 0; i < blockSize; i++) {
            float clock = FLOAT_ARRAY(detector->outputs.clock)[i];
            double time = (double) (block * blockSize + i) / sampleRate;
            if (clock > 0.5f && previousClock <= 0.5f) {
                double offset = fmod(time - 0.25, 0.5);
                if (offset > 0.25) {
                    offset -= 0.5;
                }
                // Before the taps, the clock follows the off-beats...
                if (time >= 4.0 && time < 6.0) {
                    TEST_ASSERT_TRUE(fabs(offset) > 0.2);
                    numOffBeatTriggers++;
                }
                // ...and from the second beat after the taps, the
                // tapped beats.
                if (time >= 7.5) {
                    TEST_ASSERT_TRUE(fabs(offset) <= 0.03);
                    numBeatTriggers++;
                }
            }
            previousClock = clock;
        }
    }

    TEST_ASSERT_TRUE(numOffBeatTriggers >= 3);
    TEST_ASSERT_TRUE(numBeatTriggers >= 16);

    sig_AudioBlock_destroy(&allocator, source);
    sig_AudioBlock_destroy(&allocator, sync);
    sig_dsp_TempoDetector_destroy(&allocator, detector);
}

void test_sig_dsp_TempoDetector_tracksClickTrackAt44100(void) {
    testTempo_checkClickTrack(44100.0f);
}

void test_sig_dsp_TempoDetector_tracksClickTrackAt96000(void) {
    testTempo_checkClickTrack(96000.0f);
}

void test_sig_dsp_ClockPhasor(void) {
    struct sig_dsp_ClockPhasor* phasor = sig_dsp_ClockPhasor_new(&allocator,
        context);
    float_array_ptr pulse = sig_AudioBlock_new(&allocator, audioSettings);
    phasor->inputs.pulse = pulse;
    size_t period = 100;
    size_t blockSize = audioSettings->blockSize;

    // The phasor follows the pulses across blocks. After its second pulse,
    // it completes one cycle per period.
    for (size_t block = 0; block < 20; block++) {
        for (size_t i = 0; i < blockSize; i++) {
            size_t n = block * blockSize + i;
            FLOAT_ARRAY(pulse)[i] = n % period < CLOCK_PULSE_WIDTH ?
                1.0f : 0.0f;
        }

        phasor->signal.generate(phasor);

        for (size_t i = 0; i < blockSize; i++) {
            size_t n = block * blockSize + i;
            float expected = n < period ? 0.0f :
                (float) ((n - period) % period) / (float) period;
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, phaseDistance(expected,
                FLOAT_ARRAY(phasor->outputs.main)[i]));
        }
    }

    sig_dsp_ClockPhasor_destroy(&allocator, phasor);
}

void test_sig_dsp_Sequencer_stepsInOrder(void) {
    // With 1-sample steps, the output advances one value per sample,
    // cycling through all values when looping.
    struct sig_AudioSettings* localSettings = sig_AudioSettings_new(&allocator);
    localSettings->blockSize = 9;
    struct sig_SignalContext* localContext =
        sig_SignalContext_new(&allocator, localSettings);

    float oneSampleDur = 1.0f / localSettings->sampleRate;
    float durations[3] = {oneSampleDur, oneSampleDur, oneSampleDur};
    float values[3] = {1.0f, 2.0f, 3.0f};
    struct sig_Buffer durationsBuffer = {.length = 3, .samples = durations};
    struct sig_Buffer valuesBuffer = {.length = 3, .samples = values};

    struct sig_dsp_Sequencer* seq =
        sig_dsp_Sequencer_new(&allocator, localContext);
    seq->durations = &durationsBuffer;
    seq->values = &valuesBuffer;

    seq->signal.generate(seq);

    float expected[9] = {1, 2, 3, 1, 2, 3, 1, 2, 3};
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected,
        FLOAT_ARRAY(seq->outputs.main), 9);

    sig_dsp_Sequencer_destroy(&allocator, seq);
}

void test_sig_dsp_Sequencer_stepDuration(void) {
    // Each step's value should be held for the correct number of samples.
    struct sig_AudioSettings* localSettings = sig_AudioSettings_new(&allocator);
    localSettings->blockSize = 8;
    struct sig_SignalContext* localContext =
        sig_SignalContext_new(&allocator, localSettings);

    float twoSampleDur = 2.0f / localSettings->sampleRate;
    float durations[2] = {twoSampleDur, twoSampleDur};
    float values[2] = {1.0f, 2.0f};
    struct sig_Buffer durationsBuffer = {.length = 2, .samples = durations};
    struct sig_Buffer valuesBuffer = {.length = 2, .samples = values};

    struct sig_dsp_Sequencer* seq =
        sig_dsp_Sequencer_new(&allocator, localContext);
    seq->durations = &durationsBuffer;
    seq->values = &valuesBuffer;

    seq->signal.generate(seq);

    float expected[8] = {1, 1, 2, 2, 1, 1, 2, 2};
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected,
        FLOAT_ARRAY(seq->outputs.main), 8);

    sig_dsp_Sequencer_destroy(&allocator, seq);
}

void test_sig_dsp_Sequencer_looping(void) {
    // When the sequencer loops, the output cycles through values
    // in order indefinitely.
    struct sig_AudioSettings* localSettings = sig_AudioSettings_new(&allocator);
    localSettings->blockSize = 8;
    struct sig_SignalContext* localContext =
        sig_SignalContext_new(&allocator, localSettings);

    float oneSampleDur = 1.0f / localSettings->sampleRate;
    float durations[2] = {oneSampleDur, oneSampleDur};
    float values[2] = {3.0f, 7.0f};
    struct sig_Buffer durationsBuffer = {.length = 2, .samples = durations};
    struct sig_Buffer valuesBuffer = {.length = 2, .samples = values};

    struct sig_dsp_Sequencer* seq =
        sig_dsp_Sequencer_new(&allocator, localContext);
    seq->parameters.loop = 1.0f;
    seq->durations = &durationsBuffer;
    seq->values = &valuesBuffer;

    seq->signal.generate(seq);

    float expected[8] = {3, 7, 3, 7, 3, 7, 3, 7};
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected,
        FLOAT_ARRAY(seq->outputs.main), 8);

    sig_dsp_Sequencer_destroy(&allocator, seq);
}

void test_sig_dsp_Sequencer_noLoop(void) {
    // With loop disabled,
    // the output should go silent after the last step ends.
    struct sig_AudioSettings* localSettings = sig_AudioSettings_new(&allocator);
    localSettings->blockSize = 6;
    struct sig_SignalContext* localContext =
        sig_SignalContext_new(&allocator, localSettings);

    float oneSampleDur = 1.0f / localSettings->sampleRate;
    float durations[2] = {oneSampleDur, oneSampleDur};
    float values[2] = {1.0f, 2.0f};
    struct sig_Buffer durationsBuffer = {.length = 2, .samples = durations};
    struct sig_Buffer valuesBuffer = {.length = 2, .samples = values};

    struct sig_dsp_Sequencer* seq =
        sig_dsp_Sequencer_new(&allocator, localContext);
    seq->durations = &durationsBuffer;
    seq->values = &valuesBuffer;
    seq->parameters.loop = 0.0f;

    seq->signal.generate(seq);

    float expected[6] = {1, 2, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected,
        FLOAT_ARRAY(seq->outputs.main), 6);

    sig_dsp_Sequencer_destroy(&allocator, seq);
}

void test_sig_dsp_Sequencer_holdLastValue(void) {
    // With holdLastValue enabled and loop disabled,
    // the last value should be held after the sequence ends,
    // rather than going silent.
    struct sig_AudioSettings* localSettings = sig_AudioSettings_new(&allocator);
    localSettings->blockSize = 6;
    struct sig_SignalContext* localContext =
        sig_SignalContext_new(&allocator, localSettings);

    float oneSampleDur = 1.0f / localSettings->sampleRate;
    float durations[2] = {oneSampleDur, oneSampleDur};
    float values[2] = {1.0f, 2.0f};
    struct sig_Buffer durationsBuffer = {.length = 2, .samples = durations};
    struct sig_Buffer valuesBuffer = {.length = 2, .samples = values};

    struct sig_dsp_Sequencer* seq =
        sig_dsp_Sequencer_new(&allocator, localContext);
    seq->durations = &durationsBuffer;
    seq->values = &valuesBuffer;
    seq->parameters.loop = 0.0f;
    seq->parameters.holdLastValue = 1.0f;

    seq->signal.generate(seq);

    float expected[6] = {1, 2, 2, 2, 2, 2};
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected,
        FLOAT_ARRAY(seq->outputs.main), 6);

    sig_dsp_Sequencer_destroy(&allocator, seq);
}

void test_sig_dsp_Sequencer_resetOnNext(void) {
    // With resetOnNext enabled,
    // the first sample of each step is zero.
    struct sig_AudioSettings* localSettings = sig_AudioSettings_new(&allocator);
    localSettings->blockSize = 8;
    struct sig_SignalContext* localContext =
        sig_SignalContext_new(&allocator, localSettings);

    float twoSampleDur = 2.0f / localSettings->sampleRate;
    float durations[2] = {twoSampleDur, twoSampleDur};
    float values[2] = {1.0f, 2.0f};
    struct sig_Buffer durationsBuffer = {.length = 2, .samples = durations};
    struct sig_Buffer valuesBuffer = {.length = 2, .samples = values};

    struct sig_dsp_Sequencer* seq =
        sig_dsp_Sequencer_new(&allocator, localContext);
    seq->durations = &durationsBuffer;
    seq->values = &valuesBuffer;
    seq->parameters.resetOnNext = 1.0f;

    seq->signal.generate(seq);

    float expected[8] = {0, 1, 0, 2, 0, 1, 0, 2};
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected,
        FLOAT_ARRAY(seq->outputs.main), 8);

    sig_dsp_Sequencer_destroy(&allocator, seq);
}

void test_sig_dsp_Sequencer_indexOutput(void) {
    // The index output should reflect the current step index,
    // cycling alongside the main output values.
    struct sig_AudioSettings* localSettings = sig_AudioSettings_new(&allocator);
    localSettings->blockSize = 9;
    struct sig_SignalContext* localContext =
        sig_SignalContext_new(&allocator, localSettings);

    float oneSampleDur = 1.0f / localSettings->sampleRate;
    float durations[3] = {oneSampleDur, oneSampleDur, oneSampleDur};
    float values[3] = {1.0f, 2.0f, 3.0f};
    struct sig_Buffer durationsBuffer = {.length = 3, .samples = durations};
    struct sig_Buffer valuesBuffer = {.length = 3, .samples = values};

    struct sig_dsp_Sequencer* seq =
        sig_dsp_Sequencer_new(&allocator, localContext);
    seq->durations = &durationsBuffer;
    seq->values = &valuesBuffer;

    seq->signal.generate(seq);

    float expectedMain[9]  = {1, 2, 3, 1, 2, 3, 1, 2, 3};
    float expectedIndex[9] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expectedMain,
        FLOAT_ARRAY(seq->outputs.main), 9);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expectedIndex,
        FLOAT_ARRAY(seq->outputs.index), 9);

    sig_dsp_Sequencer_destroy(&allocator, seq);
}

void test_sig_dsp_Sequencer_invalidSequence(void) {
    // Mismatched durations and values lengths should produce silence.
    struct sig_AudioSettings* localSettings = sig_AudioSettings_new(&allocator);
    localSettings->blockSize = 8;
    struct sig_SignalContext* localContext =
        sig_SignalContext_new(&allocator, localSettings);

    float oneSampleDur = 1.0f / localSettings->sampleRate;
    float durations[2] = {oneSampleDur, oneSampleDur};
    float values[3] = {1.0f, 2.0f, 3.0f};
    struct sig_Buffer durationsBuffer = {.length = 2, .samples = durations};
    struct sig_Buffer valuesBuffer = {.length = 3, .samples = values};

    struct sig_dsp_Sequencer* seq =
        sig_dsp_Sequencer_new(&allocator, localContext);
    seq->durations = &durationsBuffer;
    seq->values = &valuesBuffer;

    seq->signal.generate(seq);

    float expected[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected,
        FLOAT_ARRAY(seq->outputs.main), 8);

    sig_dsp_Sequencer_destroy(&allocator, seq);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_sig_fastMod1f);
    RUN_TEST(test_sig_flooredfmodf);
    RUN_TEST(test_sig_clamp);
    RUN_TEST(test_sig_linearMap_zeroSourceRange);
    RUN_TEST(test_sig_linearMap_identity);
    RUN_TEST(test_sig_linearMap_scaling);
    RUN_TEST(test_sig_linearMap_offset);
    RUN_TEST(test_sig_linearMap_inverted);
    RUN_TEST(test_sig_linearMap_negativeRange);
    RUN_TEST(test_sig_linearMap_bothNegative);
    RUN_TEST(test_sig_linearMap_clampedBelow);
    RUN_TEST(test_sig_linearMap_clampedAbove);
    RUN_TEST(test_sig_linearMap_zeroTargetRange);
    RUN_TEST(test_sig_unipolarToUint12);
    RUN_TEST(test_sig_bipolarToUint12);
    RUN_TEST(test_sig_bipolarToInvUint12);
    RUN_TEST(test_sig_randf);
    RUN_TEST(test_sig_randf_seed);
    RUN_TEST(test_sig_Random_withinRange);
    RUN_TEST(test_sig_Random_mean);
    RUN_TEST(test_sig_Random_reproducibility);
    RUN_TEST(test_sig_Random_independence);
    RUN_TEST(test_sig_midiToFreq);
    RUN_TEST(test_sig_freqToMidi);
    RUN_TEST(test_sig_sig_linearToFreq);
    RUN_TEST(test_sig_sig_freqToLinear);
    RUN_TEST(test_sig_fillWithValue);
    RUN_TEST(test_sig_fillWithSilence);
    RUN_TEST(test_sig_AudioSettings_new);
    RUN_TEST(test_sig_MALLOC_acceptsAddressOfAllocator);
    RUN_TEST(test_sig_samplesToSeconds);
    RUN_TEST(test_sig_AudioBlock_newWithValue);
    RUN_TEST(test_sig_interpolate_linear_wrapsAtEnd);
    RUN_TEST(test_sig_osc_Wavetable_negativePhaseStaysWithinTable);
    RUN_TEST(test_sig_FFT_forward);
    RUN_TEST(test_sig_FFT_inverse);
    RUN_TEST(test_sig_waveform_harmonicGain);
    RUN_TEST(test_sig_waveform_harmonicTaper);
    RUN_TEST(test_sig_STANDARD_WAVEFORMS);
    RUN_TEST(test_sig_waveform_sinePhaseTriangle);
    RUN_TEST(test_sig_waveform_sinePhaseSaw);
    RUN_TEST(test_sig_waveform_sinePhaseHarmonicGain);
    RUN_TEST(test_sig_SINE_PHASE_WAVEFORMS);
    RUN_TEST(test_sig_Buffer_fillWithHarmonics_matchesWaveforms);
    RUN_TEST(test_sig_Buffer_fillWithHarmonics_matchesSinePhaseWaveforms);
    RUN_TEST(test_sig_Buffer_fillWithHarmonics_lanczosLimitsOvershoot);
    RUN_TEST(test_sig_Buffer_fillWithHarmonics_isBandlimited);
    RUN_TEST(test_sig_WavetableBank_readLinearAtPhase_wrapsTableIndex);
    RUN_TEST(test_sig_osc_WavetableBank_tableIndexIsCyclic);
    RUN_TEST(test_sig_osc_WavetableBank_tableIndex);
    RUN_TEST(test_sig_AdditiveWavetableBank_harmonicsStayBelowMax);
    RUN_TEST(test_sig_AdditiveWavetableBank_customShapes);
    RUN_TEST(test_sig_AdditiveWavetableBank_tapersEachLevel);
    RUN_TEST(test_sig_AdditiveWavetableBank_levelForFrequency);
    RUN_TEST(test_sig_AdditiveWavetableBank_readLinearAtPhase);
    RUN_TEST(test_sig_osc_AdditiveWavetableBank_sine);
    RUN_TEST(test_sig_osc_AdditiveWavetableBank_aliasesLess);
    RUN_TEST(test_sig_osc_AdditiveWavetableBank_tableIndexIsCyclic);
    RUN_TEST(test_sig_dsp_AdditiveWavetableBankOscillator);
    RUN_TEST(test_sig_Buffer);
    RUN_TEST(test_sig_BufferView);
    RUN_TEST(test_sig_linearXFade);
    RUN_TEST(test_sig_DelayLine_linearComb_fractionalDelay);
    RUN_TEST(test_sig_DelayLine_linearComb_feedback);
    RUN_TEST(test_sig_dsp_Value);
    RUN_TEST(test_sig_dsp_ConstantValue);
    RUN_TEST(test_sig_dsp_TimedTriggerCounter);
    RUN_TEST(test_sig_dsp_Mul);
    RUN_TEST(test_sig_dsp_SineOscillator);
    RUN_TEST(test_sig_dsp_SineOscillator_accumulatesPhase);
    RUN_TEST(test_sig_dsp_SineOscillator_phaseWrapsAt2PI);
    RUN_TEST(test_test_sig_dsp_SineOscillator_isOffset);
    RUN_TEST(test_sig_dsp_Dust);
    RUN_TEST(test_sig_dsp_ClockDetector_square);
    RUN_TEST(test_sig_dsp_ClockDetector_sine);
    RUN_TEST(test_sig_dsp_ClockDetector_slowDown);
    RUN_TEST(test_sig_dsp_ClockDetector_stop);
    RUN_TEST(test_sig_dsp_TimedGate_unipolar);
    RUN_TEST(test_sig_dsp_TimedGate_resetOnTrigger);
    RUN_TEST(test_sig_dsp_TimedGate_bipolar);
    RUN_TEST(test_sig_dsp_List_wrapping);
    RUN_TEST(test_sig_dsp_List_noNormalization);
    RUN_TEST(test_sig_dsp_List_clamping);
    RUN_TEST(test_sig_dsp_List_interpolation);
    RUN_TEST(test_sig_dsp_List_noList);
    RUN_TEST(test_sig_dsp_DCBlock_AC);
    RUN_TEST(test_sig_dsp_DCBlock_DC);
    RUN_TEST(test_sig_softClip);
    RUN_TEST(test_sig_dsp_SoftClip);
    RUN_TEST(test_sig_filter_svf_shapes);
    RUN_TEST(test_sig_filter_SVF_generate);
    RUN_TEST(test_sig_filter_SVF_calculateCoefficients_atHalfSampleRate);
    RUN_TEST(test_sig_dsp_SVF_lowPass);
    RUN_TEST(test_sig_dsp_SVF_lowPass_blocksHighFrequency);
    RUN_TEST(test_sig_dsp_SVF_highPass);
    RUN_TEST(test_sig_dsp_SVF_highPass_passesHighFrequency);
    RUN_TEST(test_sig_dsp_SVF_bandPass);
    RUN_TEST(test_sig_dsp_SVF_notch);
    RUN_TEST(test_sig_dsp_SVF_allPass);
    RUN_TEST(test_sig_dsp_SVF_peak);
    RUN_TEST(test_sig_dsp_SVF_resonance);
    RUN_TEST(test_sig_dsp_SVF_highResonanceIsStable);
    RUN_TEST(test_sig_dsp_SVF_maxResonanceIsStable);
    RUN_TEST(test_sig_dsp_SVFLowPass);
    RUN_TEST(test_sig_filter_Ladder_generate);
    RUN_TEST(test_sig_filter_Ladder_calcStageInput);
    RUN_TEST(test_sig_filter_Ladder_calcStages);
    RUN_TEST(test_sig_dsp_Ladder_outputs);
    RUN_TEST(test_sig_osc_RatioPhasor_locksToWholeRatio);
    RUN_TEST(test_sig_osc_RatioPhasor_locksAcrossReferenceCycles);
    RUN_TEST(test_sig_osc_RatioPhasor_locksWhenTransposedByOctaves);
    RUN_TEST(test_sig_osc_RatioPhasor_glidesAfterRatioChange);
    RUN_TEST(test_sig_osc_RatioPhasor_followsWithoutLocking);
    RUN_TEST(test_sig_osc_RatioPhasor_followsFractionalTransposition);
    RUN_TEST(test_sig_osc_RatioPhasor_followsDenominatorBelowOne);
    RUN_TEST(test_sig_osc_RatioPhasor_limitsFrequency);
    RUN_TEST(test_sig_osc_RatioPhasor_limitsToNyquistByDefault);
    RUN_TEST(test_sig_osc_PeakFrequency_measuresPhaseRate);
    RUN_TEST(test_sig_osc_PeakFrequency_holdsAndReleasesPeak);
    RUN_TEST(test_sig_snapToInteger);
    RUN_TEST(test_sig_dsp_SnapToInteger);
    RUN_TEST(test_sig_dsp_Phasor);
    RUN_TEST(test_sig_dsp_RatioPhasor);
    RUN_TEST(test_sig_dsp_RatioPhasor_transposesByOctaves);
    RUN_TEST(test_sig_dsp_RatioPhasor_limitsFrequency);
    RUN_TEST(test_sig_dsp_WavetableReader);
    RUN_TEST(test_sig_dsp_AdditiveWavetableBankReader);
    RUN_TEST(test_sig_dsp_AdditiveWavetableBankReader_selectsLevelFromPeak);
    RUN_TEST(test_sig_dsp_RatioPhasor_phaseModulationHasNoDC);
    RUN_TEST(test_sig_dsp_WavetableBankReader);
    RUN_TEST(test_sig_osc_ClockPhasor_followsPulses);
    RUN_TEST(test_sig_osc_ClockPhasor_spreadsCorrection);
    RUN_TEST(test_sig_osc_ClockPhasor_slowsWithoutJumping);
    RUN_TEST(test_sig_osc_ClockPhasor_keepsFrequencyWhenPulsesStop);
    RUN_TEST(test_sig_dsp_ClockPhasor);
    RUN_TEST(test_sig_tempo_OnsetDetector_silence);
    RUN_TEST(test_sig_tempo_OnsetDetector_detectsOnsets);
    RUN_TEST(test_sig_tempo_OnsetDetector_ignoresVibrato);
    RUN_TEST(test_sig_tempo_OnsetDetector_whitensDynamics);
    RUN_TEST(test_sig_tempo_OnsetDetector_ignoresVibratoOnRichTone);
    RUN_TEST(test_sig_tempo_OnsetDetector_ignoresQuietNoise);
    RUN_TEST(test_sig_tempo_OnsetDetector_detectsPlucksInNoise);
    RUN_TEST(test_sig_tempo_OnsetDetector_appliesThreshold);
    RUN_TEST(test_sig_tempo_OnsetDetector_detectsPluckOverToneQuickly);
    RUN_TEST(test_sig_tempo_OnsetDetector_matchesAcrossSampleRates);
    RUN_TEST(test_sig_tempo_OnsetDetector_favoursLowFrequencies);
    RUN_TEST(test_sig_tempo_OnsetDetector_reportsNewValues);
    RUN_TEST(test_sig_tempo_OnsetDetector_countsBands);
    RUN_TEST(test_sig_tempo_TempoEstimator_tracksTempo);
    RUN_TEST(test_sig_tempo_TempoEstimator_tracksSlowTempo);
    RUN_TEST(test_sig_tempo_TempoEstimator_followsTempoChange);
    RUN_TEST(test_sig_tempo_TempoEstimator_holdsTempoInSilence);
    RUN_TEST(test_sig_tempo_TempoEstimator_restrictsTempoRange);
    RUN_TEST(test_sig_tempo_TempoEstimator_holdsEstablishedLevel);
    RUN_TEST(test_sig_tempo_TempoEstimator_followsTempoAfterLongRun);
    RUN_TEST(test_sig_tempo_BeatTracker_runsAtEstimatedTempo);
    RUN_TEST(test_sig_tempo_BeatTracker_locksToSteadyBeats);
    RUN_TEST(test_sig_tempo_BeatTracker_followsRitardando);
    RUN_TEST(test_sig_tempo_BeatTracker_followsTempoStep);
    RUN_TEST(test_sig_tempo_BeatTracker_keepsTempoThroughGap);
    RUN_TEST(test_sig_tempo_BeatTracker_ignoresOffBeats);
    RUN_TEST(test_sig_tempo_BeatTracker_reportsSilence);
    RUN_TEST(test_sig_tempo_BeatTracker_reportsConfidence);
    RUN_TEST(test_sig_tempo_BeatTracker_tapMovesPhaseOntoBeats);
    RUN_TEST(test_sig_tempo_BeatTracker_tapsSetTempo);
    RUN_TEST(test_sig_tempo_BeatTracker_twoTapsStartFollower);
    RUN_TEST(test_sig_tempo_BeatTracker_reanchorsAfterSilenceFollowingTaps);
    RUN_TEST(test_sig_tempo_ClockGenerator_firesEarlyByLead);
    RUN_TEST(test_sig_tempo_ClockGenerator_firesOncePerBeat);
    RUN_TEST(test_sig_tempo_ClockGenerator_holdsTriggerDuration);
    RUN_TEST(test_sig_tempo_ClockGenerator_followsSilenceBehaviour);
    RUN_TEST(test_sig_dsp_TempoDetector_tracksClickTrack);
    RUN_TEST(test_sig_dsp_TempoDetector_tracksPluckedTones);
    RUN_TEST(test_sig_dsp_TempoDetector_followsSilenceBehaviour);
    RUN_TEST(test_sig_dsp_TempoDetector_tracksQuietPassage);
    RUN_TEST(test_sig_dsp_TempoDetector_followsKicksUnderHiHats);
    RUN_TEST(test_sig_dsp_TempoDetector_followsSyncTaps);
    RUN_TEST(test_sig_dsp_TempoDetector_tracksClickTrackAt44100);
    RUN_TEST(test_sig_dsp_TempoDetector_tracksClickTrackAt96000);
    RUN_TEST(test_sig_dsp_Sequencer_stepsInOrder);
    RUN_TEST(test_sig_dsp_Sequencer_stepDuration);
    RUN_TEST(test_sig_dsp_Sequencer_looping);
    RUN_TEST(test_sig_dsp_Sequencer_noLoop);
    RUN_TEST(test_sig_dsp_Sequencer_holdLastValue);
    RUN_TEST(test_sig_dsp_Sequencer_resetOnNext);
    RUN_TEST(test_sig_dsp_Sequencer_indexOutput);
    RUN_TEST(test_sig_dsp_Sequencer_invalidSequence);

    return UNITY_END();
}
