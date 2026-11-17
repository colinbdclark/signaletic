/*! \file libsignaletic.h
    \brief The core Signaletic library.

    Signaletic is music signal processing library designed for use
    in embedded environments and Web Assembly.
*/

#ifndef LIBSIGNALETIC_H
#define LIBSIGNALETIC_H

#include <math.h> // Included here for use by the inline function definitions.

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> // For size_t
#include <stdbool.h>
#include <stdint.h> // For int32_t, uint32_t

// Macro for the restrict keyword for
// C++ and C compilers that support it.
#if defined(__cplusplus)
    #if defined(__GNUC__) || defined(__clang__)
        #define SIG_RESTRICT __restrict__
    #elif defined(_MSC_VER)
        #define SIG_RESTRICT __restrict
    #else
        #define SIG_RESTRICT
    #endif
#else
    #if __STDC_VERSION__ >= 199901L
        #define SIG_RESTRICT restrict
    #elif defined(__GNUC__) || defined(__clang__)
        #define SIG_RESTRICT __restrict__
    #else
        #define SIG_RESTRICT
    #endif
#endif

// This typedef is necessary because Emscripten's
// WebIDL binder is unable to produce viable
// (i.e. performant and bug-free) bindings for
// array pointers. As a result, for Emscripten
// we handle float arrays as void pointers instead,
// so that we can do manual deferencing on the
// JavaScript side of the universe.
// As a result, all float* variables in Signaletic
// in public interface must use this type instead.
#ifdef __EMSCRIPTEN__
    typedef void* float_array_ptr;
    #define FLOAT_ARRAY(array) ((float*)array)
#else
    typedef float* float_array_ptr;
    #define FLOAT_ARRAY(array) array
#endif

#define sig_PI 3.14159265358979323846f
#define sig_TWOPI 6.283185307179586f
#define sig_RECIP_TWOPI 0.159154943091895f
#define sig_LOG0_001 (-6.907755278982137f)
#define sig_LOG2 0.6931471805599453f
#define sig_FREQ_C4 261.6256f

/**
 * Returns the smaller of two floating point arguments.
 *
 * ARM-optimized 32-bit floating point implementation
 * from DaisySP (Copyright 2020 Electrosmith, Corp),
 * written by Stephen McCaul.
 * https://github.com/electro-smith/DaisySP/blob/master/Source/Utility/dsp.h
 *
 * @param a the first value to compare
 * @param b the second value to compare
 * @return the samller of the two arguments
*/
inline float sig_fminf(float a, float b);

/**
 * Returns the larger of two floating point arguments.
 *
 * ARM-optimized 32-bit floating point implementation
 * from DaisySP written by Stephen McCaul.
 * https://github.com/electro-smith/DaisySP/blob/master/Source/Utility/dsp.h
 *
 * @param a the first value to compare
 * @param b the second value to compare
 * @return the larger of the two arguments
*/
inline float sig_fmaxf(float a, float b);

/**
 * Clamps the value between the specified minimum and maximum.
 *
 * @param value the sample to clamp
 * @param min the minimum value
 * @param max the maximum value
 * @return the clamped value
 */
inline float sig_clamp(float value, float min, float max);

/**
 * @brief Rounds a value to the nearest whole number if it is within a
 * tolerance of it, and otherwise returns the value unchanged.
 *
 * Snapping holds a control at exact whole-number values despite noise,
 * while leaving it continuous elsewhere. For example, snapping a pitch in
 * octaves holds it at exact octaves. To snap to another step size, such
 * as semitones, scale the value so that each step is 1.0 before snapping.
 *
 * @param value the value to snap
 * @param tolerance the largest distance from a whole number at which the
 * value snaps to it
 * @return the nearest whole number, or the value
 */
inline float sig_snapToInteger(float value, float tolerance);

/**
 * @brief Computes the fractional part of a value,
 * wrapped to the range 0.0-1.0.
 * This is useful as a faster alternative to sig_flooredfmodf
 * for phase normalization.
 *
 * @param x the input value
 * @return float the wrapped fractional part
 */
inline float sig_fastMod1f(float x);

/**
 * @brief Computes the remainder of the floored division of two arguments.
 * This is useful when implementing through zero wrap arounds.
 * See https://en.wikipedia.org/wiki/Modulo#Variants_of_the_definition
 * for more information.
 *
 * @param num the numerator
 * @param denom the denominator
 * @return float the remainder of the floored division operation
 */
inline float sig_flooredfmodf(float num, float denom);

/**
 * Generates a random float between 0.0 and 1.0
 * using the global default xoshiro128+ generator.
 *
 * @return a random value
 */
float sig_randf(void);

/**
 * @brief Seeds the global default generator used by sig_randf.
 *
 * @param seed the seed value
 */
void sig_randf_seed(uint32_t seed);

/**
 * @brief A fast tanh approximation
 *
 * @param x floating point value representing a hyperbolic angle
 * @return float the hyperbolic tangent of x
 */
inline float sig_fastTanhf(float x);

/**
 * @brief Linearly maps a value from one range to another,
 * clamping out of range values to the min and max.
 *
 * @param value the value to map
 * @param fromMin the minimum of the current range
 * @param fromMax the maximum of the current range
 * @param toMin the minimum of the target range
 * @param toMax the maximum of the target range
 * @return float the mapped value
 */
inline float sig_linearMap(float value,
    float fromMin, float fromMax, float toMin, float toMax);

/**
 * Converts a unipolar floating point sample in the range 0.0 to 1.0
 * into to an unsigned 12-bit integer in the range 0 to 4095.
 *
 * This function does not clamp the sample.
 *
 * @param sample the floating point sample to convert
 * @return the sample converted to an unsigned 12-bit integer
 */
inline uint16_t sig_unipolarToUint12(float sample);

/**
 * Converts a bipolar floating point sample in the range -1.0 to 1.0
 * into to an unsigned 12-bit integer in the range 0 to 4095.
 *
 * This function does not clamp the sample.
 *
 * @param sample the floating point sample to convert
 * @return the sample converted to an unsigned 12-bit integer
 */
inline uint16_t sig_bipolarToUint12(float sample);

/**
 * Converts a bipolar floating point sample in the range -1.0 to 1.0
 * into to an unsigned 12-bit integer in the range 4095-0.
 *
 * This function does not clamp the sample.
 *
 * @param sample the floating point sample to convert
 * @return the sample converted to an unsigned 12-bit integer
 */
inline uint16_t sig_bipolarToInvUint12(float sample);

/**
 * Converts an unsigned 16-bit integer in the range 0-65536 to a
 * bipolar floating point sample in the range -1.0 to 1.0.
 *
 * This function does not clamp the sample.
 *
 * @param sample the unsigned 16-bit sample to convert
 * @return the sample converted to normalized floating point
 */
inline float sig_uint16ToBipolar(uint16_t sample);

/**
 * Converts an unsigned 16-bit integer in the range 0-65536 to a
 * unipolar floating point sample in the range 0.0 to 1.0.
 *
 * This function does not clamp the sample.
 *
 * @param sample the unsigned 16-bit sample to convert
 * @return the sample converted to normalized unipolar floating point
 */
inline float sig_uint16ToUnipolar(uint16_t sample);

/**
 * Converts an unsigned 16-bit integer in the range 65536-0 to a
 * bipolar floating point sample in the range -1.0 to 1.0.
 *
 * This function does not clamp the sample.
 *
 * @param sample the inverted unsigned 16-bit sample to convert
 * @return the sample converted to normalized floating point
 */
inline float sig_invUint16ToBipolar(uint16_t sample);

/**
 * Converts MIDI note numbers into frequencies in hertz.
 * This algorithm assumes A4 = 440 Hz = MIDI note #69.
 *
 * @param midiNum the MIDI note number to convert
 * @return the frequency in Hz of the note number
 */
float sig_midiToFreq(float midiNum);

/**
 * Converts frequencies in hertz to MIDI note numbers.
 * This algorithm assumes A4 = 440 Hz = MIDI note #69.
 *
 * @param frequency the frequency in hertz to convert
 * @return the MIDI note number corresponding to the frequency
 */
float sig_freqToMidi(float frequency);

/**
 * @brief Converts a floating point value that represents pitch in
 * a linear scale such as Eurorack-style volts/octave into a frequency in Hz.
 *
 * @param value the value to convert
 * @param middleFreq the frequency at the midpoint (e.g. 261.6256 for middle C4 at 0.0f)
 * @return float the frequency in hz
 */
float sig_linearToFreq(float value, float middleFreq);

/**
 * @brief Converts a frequency in Hz in to a linear volts per octave-style
 * floating point value.
 *
 * @param freq the frequency to convert
 * @param middleFreq the frequency at the midpoint (e.g. 261.6256 for middle C4 at 0.0f)
 * @return float the linear value
 */
float sig_freqToLinear(float freq, float middleFreq);

/**
 * @brief Sums all values in an array.
 *
 * @param values an array of floating point values
 * @param length the length of the array
 * @return float the sum of all values
 */
inline float sig_sum(float_array_ptr values, size_t length);

/**
 * @brief Returns the index of the smallest value in an array.
 *
 * @param values an array of floating point values
 * @param length the length of the warray
 * @return size_t the index to the smallest item in the array
 */
inline size_t sig_indexOfMin(float_array_ptr values, size_t length);

/**
 * @brief Returns the index of the largest value in an array.
 *
 * @param values an array of floating point values
 * @param length the length of the warray
 * @return size_t the index to the largest item in the array

 */
inline size_t sig_indexOfMax(float_array_ptr values, size_t length);

/**
 * Type definition for array fill functions.
 *
 * @param i the current index of the array
 * @param array the array being filled
 */
typedef float (*sig_array_filler)(size_t i, float_array_ptr array);

/**
 * A fill function that returns random floats.
 *
 * @param i unused
 * @param array unused
 */
float sig_randomFill(size_t i, float_array_ptr array);

/**
 * Fills an array of floats using the specified filler function.
 *
 * @param array the array to fill
 * @param length the lengthg of the array
 * @param filler a pointer to a fill function
 */
void sig_fill(float_array_ptr array, size_t length,
    sig_array_filler filler);

/**
 * Fills an array of floats with the specified value.
 *
 * @param value the value to fill the array with
 * @param array the array to fill
 * @param length the length of the array to fill
 **/
void sig_fillWithValue(float_array_ptr array, size_t length,
    float value);

/**
 * Fills an array of floats with zeroes.
 *
 * @param array the array to fill with silence
 * @param length the length of the array to fill
 **/
void sig_fillWithSilence(float_array_ptr array, size_t length);

/**
 * Interpolates a value from the specified lookup table
 * using linear interpolation. This implementation will
 * wrap to the beginning of the table if needed.
 *
 * @param idx an index into the table
 * @param table the table from which values around idx should be drawn and interpolated
 * @param length the length of the buffer
 * @return the interpolated value
 */
float sig_interpolate_linear(float idx, float_array_ptr table,
    size_t length);

/**
 * Interpolates a value from the specified lookup table
 * using Hermite cubic interpolation. This implementation will
 * wrap around at either boundary of the table if needed.
 *
 * Based on Laurent De Soras' implementation at:
 * http://www.musicdsp.org/showArchiveComment.php?ArchiveID=93
 *
 * @param idx an index into the table
 * @param table the table from which values around idx should be drawn and interpolated
 * @param length the length of the buffer
 * @return the interpolated value
 */
float sig_interpolate_cubic(float idx, float_array_ptr table, size_t length);

/**
 * @brief Returns the mean average of all values in the specified array.
 *
 * @param values an array of floating point values to average
 * @param length the number of values in the array
 * @return float the mean of all values
 */
inline float sig_filter_mean(float_array_ptr values, size_t length);

/**
 * @brief Returns the mean average of values in the array, after excluding the
 * largest and smallest values.
 *
 * @param values an array of floating point values to average
 * @param length the number of values in the array
 * @return float float the mean of all values
 */
inline float sig_filter_meanExcludeMinMax(float_array_ptr values, size_t length);

/**
 * @brief An exponential moving average filter that implements the formula
 * y[n] = a * x[n] + (1 - a) * y[n-1]
 *
 * @param current the current sample (i.e. x[n])
 * @param previous the previous output sample (i.e. y[n-1])
 * @param a the coefficient (between 0 and 1); values closer to 0 apply more filtering)
 * @return float the filtered sample (i.e. y[n])
 */
inline float sig_filter_ema(float current, float previous, float a);

/**
 * @brief A one pole filter that implements the formula
 * y[n] = b0 * x[n] + a1 * y[n-1].
 *
 * @param current the current sample (i.e. x[n])
 * @param previous the previous output sample (i.e. y[n-1])
 * @param b0 the forward coefficient
 * @param a1 the feedback coefficient
 * @return float the filtered sample (i.e. y[n])
 */
inline float sig_filter_onepole(float current, float previous, float b0,
    float a1);

/**
 * @brief Calculates the feedback coefficient for a high pass one pole filter.
 *
 * @param frequency the cutoff frequency of the filter
 * @param sampleRate the current sample rate
 * @return float the feedback coeffient (a1)
 */
inline float sig_filter_onepole_HPF_calculateA1(float frequency, float sampleRate);

/**
 * @brief Calculates the forward coefficient for a high pass one pole filter.
 *
 * @param a1 the feedback coefficient
 * @return float the forward coefficient (b0)
 */
inline float sig_filter_onepole_HPF_calculateB0(float a1);

/**
 * @brief Calculates the feedback coefficient for a low pass one pole filter.
 *
 * @param frequency the cutoff frequency of the filter
 * @param sampleRate the current sample rate
 * @return float the feedback coeffient (a1)
 */
inline float sig_filter_onepole_LPF_calculateA1(float frequency, float sampleRate);

/**
 * @brief Calculates the forward coefficient for a low pass one pole filter.
 *
 * @param a1 the feedback coefficient
 * @return float the forward coefficient (b0)
 */
inline float sig_filter_onepole_LPF_calculateB0(float a1);

/**
 * An optimized one pole low-pass filter,
 * typically used for smoothing parameter values.
 *
 * @param current the current sample (n)
 * @param previous the previous sample (n-1)
 * @param coeff the filter coefficient (a1)
 */
inline float sig_filter_smooth(float current, float previous, float coeff);

/**
 * @brief Calculates the coefficient from a 60 dB convergence time for
 * a low pass one pole filter.
 *
 * @param timeSecs the time (in seconds) for the filter to converge on the value
 * @param sampleRate the current sample rate
 * @return float the coefficient (a1)
 */
inline float sig_filter_smooth_calculateCoefficient(float timeSecs,
    float sampleRate);

/**
 * @brief Removes DC offset by subtracting 1-pole LPF from the input.
 *
 * @param current the current sample
 * @param previousInput the previous input sample
 * @param previousOutput the previous output sample
 * @param r the filter coefficient
 * @return float the filtered sample
 */
inline float sig_filter_dcBlock(float current, float previousInput,
    float previousOutput, float r);

/**
 * @brief Calculates the R coefficient for a DC blocking filter.
 *
 * @param frequency the cutoff frequency of the filter
 * @param sampleRate the current sample rate
 * @return float the R coefficient
 */
inline float sig_filter_dcBlock_calculateCoefficient(float frequency, float sampleRate);

/**
 * @brief Calculates the high pass output of a state variable filter from
 * its input and internal taps.
 *
 * @param input the filter's input sample (v0)
 * @param band the filter's band pass output (v1)
 * @param low the filter's low pass output (v2)
 * @param k the filter's damping coefficient
 * @return float the high pass output
 */
inline float sig_filter_svf_highPass(float input, float band, float low, float k);

/**
 * @brief Calculates the notch output of a state variable filter from
 * its input and internal taps.
 *
 * @param input the filter's input sample (v0)
 * @param band the filter's band pass output (v1)
 * @param low the filter's low pass output (v2)
 * @param k the filter's damping coefficient
 * @return float the notch output
 */
inline float sig_filter_svf_notch(float input, float band, float low, float k);

/**
 * @brief Calculates the peak output of a state variable filter from
 * its input and internal taps.
 *
 * @param input the filter's input sample (v0)
 * @param band the filter's band pass output (v1)
 * @param low the filter's low pass output (v2)
 * @param k the filter's damping coefficient
 * @return float the peak output
 */
inline float sig_filter_svf_peak(float input, float band, float low, float k);

/**
 * @brief Calculates the all pass output of a state variable filter from
 * its input and internal taps.
 *
 * @param input the filter's input sample (v0)
 * @param band the filter's band pass output (v1)
 * @param low the filter's low pass output (v2)
 * @param k the filter's damping coefficient
 * @return float the all pass output
 */
inline float sig_filter_svf_allPass(float input, float band, float low, float k);


// TODO: Documentation!
struct sig_filter_Smooth {
    float coeff;
    float previous;
};

void sig_filter_Smooth_init(struct sig_filter_Smooth* self, float coeff);

inline float sig_filter_Smooth_generate(struct sig_filter_Smooth* self, float value);

struct sig_filter_DCBlock {
    float r;
    float previousInput;
    float previousOutput;
};

void sig_filter_DCBlock_init(
    struct sig_filter_DCBlock* self, float frequency, float sampleRate);

inline float sig_filter_DCBlock_generate(
    struct sig_filter_DCBlock* self, float value);

/**
 * @brief The three signals that a state variable filter produces for one
 * input sample.
 *
 * These are the input, band pass, and low pass node signals of the filter's
 * underlying circuit. Every response is a weighted sum of them.
 */
struct sig_filter_SVF_Taps {
    /**
     * @brief The filter's input (v0).
     */
    float input;

    /**
     * @brief The band pass output (v1).
     */
    float band;

    /**
     * @brief The low pass output (v2).
     */
    float low;
};

/**
 * @brief The state of a state variable filter.
 *
 * The damping coefficient is clamped at 0.01 to keep the filter bounded,
 * so the filter's maximum Q is 100.
 */
struct sig_filter_SVF {
    /**
     * @brief The damping coefficient (1/Q).
     */
    float k;

    /**
     * @brief The coefficient of the band pass integrator's input term.
     */
    float a1;

    /**
     * @brief The coefficient of the input and state terms scaled by g.
     */
    float a2;

    /**
     * @brief The coefficient of the input term scaled by g squared.
     */
    float a3;

    /**
     * @brief The equivalent current state of the band pass integrator.
     */
    float ic1eq;

    /**
     * @brief The equivalent current state of the low pass integrator.
     */
    float ic2eq;
};

/**
 * @brief Initialises a state variable filter's state.
 *
 * @param self the state to initialise
 */
void sig_filter_SVF_init(struct sig_filter_SVF* self);

/**
 * @brief Calculates a state variable filter's coefficients.
 *
 * @param self the state to update
 * @param frequency the cutoff frequency in Hz, clamped to 0.49 times the sample rate
 * @param resonance the resonance, normalized to 0.0 to 1.0
 * @param sampleRate the sample rate in Hz
 */
inline void sig_filter_SVF_calculateCoefficients(struct sig_filter_SVF* self,
    float frequency, float resonance, float sampleRate);

/**
 * @brief Generates one sample.
 *
 * @param self the state to update
 * @param value the input sample
 * @param taps the struct to write the filter's input, band pass, and low pass signals to
 */
inline void sig_filter_SVF_generate(struct sig_filter_SVF* self, float value,
    struct sig_filter_SVF_Taps* taps);

/**
 * @brief The five signals that a ladder filter produces from one input sample.
 */
struct sig_filter_Ladder_Taps {
    /**
     * @brief The signal entering the first pole.
     */
    float input;

    /**
     * @brief The first pole's output.
     */
    float pole1;

    /**
     * @brief The second pole's output.
     */
    float pole2;

    /**
     * @brief The third pole's output.
     */
    float pole3;

    /**
     * @brief The fourth pole's output.
     */
    float pole4;
};

/**
 * @brief The state of a ladder filter.
 */
struct sig_filter_Ladder {
    /**
     * @brief The number of oversampled steps per sample.
     */
    uint8_t interpolation;

    /**
     * @brief The reciprocal of the number of oversampled steps.
     */
    float interpolationRecip;

    /**
     * @brief The alpha coefficient of the pole stages.
     */
    float alpha;

    /**
     * @brief The adjustment applied to the feedback amount.
     */
    float qAdjust;

    /**
     * @brief The previous input of each pole stage.
     *
     * Each stage uses its previous input in its feedforward term.
     */
    float z0[4];

    /**
     * @brief The previous output of each pole stage.
     */
    float z1[4];

    /**
     * @brief The previous input sample, for the oversampling interpolation.
     */
    float previousInput;
};

/**
 * @brief Initializes a ladder filter's state.
 *
 * @param self the state to initialise
 * @param interpolation the number of oversampled steps per sample
 */
void sig_filter_Ladder_init(struct sig_filter_Ladder* self,
    uint8_t interpolation);

/**
 * @brief Calculates a ladder filter's frequency-dependent coefficients.
 *
 * @param self the state to update
 * @param frequency the cutoff frequency
 * @param sampleRate the sample rate
 */
inline void sig_filter_Ladder_calculateCoefficients(struct sig_filter_Ladder* self,
    float frequency, float sampleRate);

/**
 * @brief Calculates a pole stage's output for the given input.
 *
 * @param self the state to update
 * @param s the stage's input sample
 * @param i the index of the stage, from 0 to 3
 * @return float the stage's output sample
 */
inline float sig_filter_Ladder_calcStage(struct sig_filter_Ladder* self, float s,
    uint8_t i);

/**
 * @brief Calculates the signal entering the first pole for one sub-step.
 *
 * @param self the state to read
 * @param value the current input sample
 * @param interp the sub-step's position, from 0.0 to 1.0
 * @param resonance the resonance, in the range 0.0 to 1.8
 * @param passbandGain the passband gain, best between 0.0 and 0.5
 * @return float the unsaturated signal entering the first pole
 */
inline float sig_filter_Ladder_calcStageInput(struct sig_filter_Ladder* self,
    float value, float interp, float resonance, float passbandGain);

/**
 * @brief Runs a sub-step's signal through all four poles.
 *
 * @param self the state to update
 * @param u the saturated signal entering the first pole
 * @param taps the struct to write this sub-step's taps to
 */
inline void sig_filter_Ladder_calcStages(struct sig_filter_Ladder* self, float u,
    struct sig_filter_Ladder_Taps* taps);

/**
 * @brief Generates one sample.
 *
 * @param self the state to update
 * @param value the input sample
 * @param resonance the resonance, in the range 0.0 to 1.8
 * @param passbandGain the passband gain, best between 0.0 and 0.5
 * @param taps the struct to write the filter's taps to
 */
inline void sig_filter_Ladder_generate(struct sig_filter_Ladder* self, float value,
    float resonance, float passbandGain,
    struct sig_filter_Ladder_Taps* taps);

/**
 * Type definition for a waveform generator function.
 *
 * @param phase the current phase
 * @return a sample for the current phase of the waveform
 */
typedef float (*sig_waveform_generator)(float phase);

/**
 * Generates one sample of a sine wave at the specified phase.
 *
 * @param phase the current phase of the waveform
 * @return the generated sample
 */
float sig_waveform_sine(float phase);

/**
 * Generates one sample of a square wave at the specified phase.
 *
 * @param phase the current phase of the waveform
 * @return the generated sample
 */
float sig_waveform_square(float phase);

/**
 * Generates one sample of a saw wave at the specified phase.
 *
 * @param phase the current phase of the waveform
 * @return the generated sample
 */
float sig_waveform_saw(float phase);

/**
 * Generates one sample of a reverse saw wave at the specified phase.
 *
 * @param phase the current phase of the waveform
 * @return the generated sample
 */
float sig_waveform_reverseSaw(float phase);

/**
 * Generates one sample of a triangle wave at the specified phase.
 *
 * @param phase the current phase of the waveform
 * @return the generated sample
 */
float sig_waveform_triangle(float phase);

/**
 * @brief Generates one sample of a triangle wave whose fundamental is in
 * phase with a sine.
 *
 * Like sig_waveform_sine, the triangle starts at zero and rises, peaking a
 * quarter of the way through the cycle. It is sig_waveform_triangle delayed
 * by a quarter cycle.
 *
 * @param phase the current phase of the waveform, in radians from 0 to 2pi
 * @return the generated sample
 */
float sig_waveform_sinePhaseTriangle(float phase);

/**
 * @brief Generates one sample of a saw wave whose fundamental is in phase
 * with a sine.
 *
 * Like sig_waveform_sine, the saw starts at zero and rises. It drops from
 * its peak to its trough halfway through the cycle. It is sig_waveform_saw
 * delayed by half a cycle.
 *
 * @param phase the current phase of the waveform, in radians from 0 to 2pi
 * @return the generated sample
 */
float sig_waveform_sinePhaseSaw(float phase);

/**
 * Type definition for a harmonic series generator function, which
 * calculates the gain of one harmonic in a waveform's Fourier series.
 *
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @return the harmonic's gain, or 0.0 if the waveform does not contain it
 */
typedef float (*sig_waveform_harmonicGenerator)(size_t harmonic);

/**
 * @brief Calculates the gain of one harmonic of a sine wave.
 *
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @return 1.0 for the fundamental, and 0.0 for every other harmonic
 */
float sig_waveform_sineHarmonicGain(size_t harmonic);

/**
 * @brief Calculates the gain of one harmonic of a triangle wave.
 *
 * The harmonics are cosines, matching the shape of sig_waveform_triangle.
 *
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @return the harmonic's gain, or 0.0 for even harmonics
 */
float sig_waveform_triangleHarmonicGain(size_t harmonic);

/**
 * @brief Calculates the gain of one harmonic of a saw wave.
 *
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @return the harmonic's gain
 */
float sig_waveform_sawHarmonicGain(size_t harmonic);

/**
 * @brief Calculates the gain of one harmonic of a square wave.
 *
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @return the harmonic's gain, or 0.0 for even harmonics
 */
float sig_waveform_squareHarmonicGain(size_t harmonic);

/**
 * @brief Calculates the gain of one harmonic of a reverse saw wave.
 *
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @return the harmonic's gain
 */
float sig_waveform_reverseSawHarmonicGain(size_t harmonic);

/**
 * @brief Calculates the gain of one harmonic of a triangle wave whose
 * fundamental is in phase with a sine.
 *
 * The harmonics are sines, matching the shape of
 * sig_waveform_sinePhaseTriangle.
 *
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @return the harmonic's gain, or 0.0 for even harmonics
 */
float sig_waveform_sinePhaseTriangleHarmonicGain(size_t harmonic);

/**
 * @brief Calculates the gain of one harmonic of a saw wave whose
 * fundamental is in phase with a sine.
 *
 * The harmonics are sines, matching the shape of sig_waveform_sinePhaseSaw.
 *
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @return the harmonic's gain
 */
float sig_waveform_sinePhaseSawHarmonicGain(size_t harmonic);

/**
 * @brief A waveform, described both by its shape over one cycle and by
 * its harmonic series.
 *
 * Both descriptions must produce the same shape and polarity.
 */
struct sig_WaveformShape {
    /**
     * @brief The function that generates the waveform's shape.
     *
     * Its phase is in radians.
     */
    sig_waveform_generator generate;

    /**
     * @brief The function that calculates the gain of each harmonic.
     *
     */
    sig_waveform_harmonicGenerator harmonic;

    /**
     * @brief The phase at which every harmonic starts.
     *
     * In cycles. Sine harmonics start at 0.0, and cosine harmonics at 0.25.
     */
    float harmonicPhase;
};

/**
 * @brief The standard waveforms, which index sig_STANDARD_WAVEFORMS.
 */
enum sig_StandardWaveform {
    sig_StandardWaveform_SINE = 0,
    sig_StandardWaveform_TRIANGLE,
    sig_StandardWaveform_SAW,
    sig_StandardWaveform_SQUARE,
    sig_StandardWaveform_REVERSE_SAW,
    sig_StandardWaveform_COUNT
};

/**
 * @brief The shapes of the standard waveforms, in the order of
 * sig_StandardWaveform.
 */
extern const struct sig_WaveformShape
    sig_STANDARD_WAVEFORMS[sig_StandardWaveform_COUNT];

/**
 * @brief The shapes of the standard waveforms, each delayed so that its
 * fundamental is in phase with a sine, in the order of
 * sig_StandardWaveform.
 *
 * Every waveform starts its cycle either by rising through zero or by
 * jumping up from its trough to its peak. Use these shapes instead of
 * sig_STANDARD_WAVEFORMS when waveforms need to line up with each other or
 * with a shared phase:
 *  - Crossfading between neighbouring waveforms does not cancel their
 *    fundamentals, so the level does not dip midway through a crossfade.
 *  - Waveforms that follow a shared phase, such as a sig_dsp_ClockPhasor,
 *    cross zero or jump at the same point in the cycle.
 *  - Every waveform is odd-symmetric, so phase modulation between waveforms
 *    whose phases are locked together produces no DC offset.
 *
 * The sine, square, and reverse saw are the same as in
 * sig_STANDARD_WAVEFORMS. The triangle is sig_waveform_sinePhaseTriangle,
 * and the saw is sig_waveform_sinePhaseSaw.
 */
extern const struct sig_WaveformShape
    sig_SINE_PHASE_WAVEFORMS[sig_StandardWaveform_COUNT];

/**
 * @brief Calculates the gain of a harmonic in a standard waveform's
 * Fourier series.
 *
 * @param waveform the standard waveform
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @return the harmonic's gain
 */
float sig_waveform_harmonicGain(enum sig_StandardWaveform waveform,
    size_t harmonic);

/**
 * @brief Calculates the phase at which every harmonic of a waveform starts.
 *
 * @param waveform the waveform to synthesize
 * @return the starting phase of a sine-shaped harmonic, in cycles
 */
float sig_waveform_harmonicPhase(enum sig_StandardWaveform waveform);

/**
 * @brief Taper types for the harmonics of a bandlimited waveform.
 *
 * Summing a truncated series of harmonics makes waveforms with
 * discontinuities, such as squares and saws, overshoot each jump by about
 * 9% (the Gibbs phenomenon). Tapering the upper harmonics reduces the
 * overshoot at the cost of slower edges and a slightly darker tone.
 */
enum sig_HarmonicTaper {
    sig_HarmonicTaper_NONE = 0,
    sig_HarmonicTaper_LANCZOS
};

/**
 * @brief Calculates the weight that a taper applies to one harmonic.
 *
 * sig_HarmonicTaper_LANCZOS applies Lanczos sigma factors,
 * sinc(harmonic / (numHarmonics + 1)), which reduce the overshoot of
 * a square wave to about 1.2% of each jump. The factors are divided by the
 * fundamental's factor, so the fundamental always has a weight of 1.0. Then
 * a waveform's loudness doesn't change between tables with different
 * numbers of harmonics.
 *
 * @param taper the taper to apply
 * @param harmonic the harmonic number, starting at 1 for the fundamental
 * @param numHarmonics the number of harmonics in the series
 * @return the harmonic's weight, between 0.0 and 1.0
 */
float sig_waveform_harmonicTaper(enum sig_HarmonicTaper taper,
    size_t harmonic, size_t numHarmonics);

/**
 * @brief State for an oscillator.
 */
struct sig_osc_Oscillator {
    float phaseAccumulator;
};

/**
 * @brief Initializes the oscillator state.
 *
 * @param self the oscillator state to initialize
 */
void sig_osc_Oscillator_init(struct sig_osc_Oscillator* self);

/**
 * @brief Determines whether the current sample is the end of the
 * oscillator's cycle. Returns 1.0f if it's the end of cycle,
 * and 0.0f if not.
 *
 * @param phase the oscillator's phase accumulator
 * @return float
 */
inline float sig_osc_Oscillator_eoc(float phase);

/**
 * @brief Wraps an oscillator's phase accumulator around in
 * the range between 0.0f and 1.0f. Supports through-zero modulation.
 *
 * @param phase the current phase of the oscillator
 * @return float the wrapped phase
 */
inline float sig_osc_Oscillator_wrapPhase(float phase);

/**
 * @brief Updates an oscillator's phase accumulator based on the
 * current frequency and sample rate.
 *
 * @param phaseAccumulator a pointer to the phase accumulator
 * @param frequency the oscillator's frequency
 * @param sampleRate the current sample rate
 */
inline void sig_osc_Oscillator_accumulatePhase(float* phaseAccumulator,
    float frequency, float sampleRate);

/**
 * @brief The state for a wavetable oscillator.
 */
struct sig_osc_Wavetable {
    float phaseAccumulator;
    struct sig_Buffer* wavetable;
};

/**
 * @brief Initializes a wavetable oscillator's state with the supplied wavetable.
 *
 * @param self the wavetable oscillator to initialize
 * @param wavetable the wavetable buffer to use
 */
void sig_osc_Wavetable_init(struct sig_osc_Wavetable* self,
    struct sig_Buffer* wavetable);

/**
 * @brief Generates a single sample of the wavetable oscillator's output.
 *
 * @param self the wavetable oscillator state
 * @param frequency the current frequency
 * @param phaseOffset the phase offset to apply
 * @param sampleRate the sample rate
 * @param eocOut a float pointer into which the end of cycle sample will be written
 * @return float the oscillator's output sample
 */
inline float sig_osc_Wavetable_generate(struct sig_osc_Wavetable* self,
    float frequency, float phaseOffset, float sampleRate, float* eocOut);

/**
 * @brief An oscillator that reads from a modulatable bank of wavetables.
 * i.e. what in the sythesizer world is called a "wavetable synth."
 */
struct sig_osc_WavetableBank {
    float phaseAccumulator;
    struct sig_WavetableBank* wavetables;
};

/**
 * @brief Initializes the state of a WavetableBank oscillator.
 *
 * @param self the wavetable bank state
 * @param wavetables a pointer to a bank of wavetables to use
 */
void sig_osc_WavetableBank_init(struct sig_osc_WavetableBank* self,
    struct sig_WavetableBank* wavetables);

/**
 * @brief Generates a single sample of the wavetable bank's output.
 *
 * The tableIndex input is cyclic: each table occupies an equal share of
 * one cycle, and the oscillator linearly interpolates between neighbouring
 * tables, including from the last table back to the first.
 *
 * @param self the wavetable bank oscillator state
 * @param frequency the oscillator's frequency
 * @param phaseOffset the phase offset to apply
 * @param tableIndex the table to read, in cycles of the bank
 * @param sampleRate the sample rate
 * @param eocOut a float pointer into which the end of cycle sample will be written
 * @return float the oscillator's output sample
 */
inline float sig_osc_WavetableBank_generate(struct sig_osc_WavetableBank* self,
    float frequency, float phaseOffset, float tableIndex, float sampleRate,
    float* eocOut);

/**
 * @brief Calculates the cyclic table index at which a wavetable bank
 * oscillator reads exactly one of its tables.
 *
 * Passing the last table index (i.e. numTables - 1) gives the span that a
 * control should cover in order to reach every table without crossfading
 * from the last back to the first. This applies to both
 * sig_osc_WavetableBank and sig_osc_AdditiveWavetableBank, whose bank
 * contains numShapes tables.
 *
 * @param table the number of the table to read, starting at 0
 * @param numTables the number of tables in the bank
 * @return the table index, in cycles of the bank
 */
float sig_osc_WavetableBank_tableIndex(size_t table, size_t numTables);

/**
 * @brief The state for an oscillator that reads from a band-limited
 * wavetable bank, choosing tables by the octave of its frequency.
 */
struct sig_osc_AdditiveWavetableBank {
    /**
     * @brief The oscillator's current phase.
     *
     * Normalized to 0.0-1.0.
     */
    float phaseAccumulator;

    /**
     * @brief The additive wavetable bank to read from.
     *
     */
    struct sig_AdditiveWavetableBank* wavetables;

    /**
     * @brief The frequency at which the current level was calculated.
     *
     * In Hz.
     */
    float previousFrequency;

    /**
     * @brief The octave level to read for the current frequency.
     *
     */
    float level;
};

/**
 * @brief Initializes the state of a additive wavetable bank oscillator.
 *
 * @param self the oscillator state
 * @param wavetables the additive wavetable bank to read from
 */
void sig_osc_AdditiveWavetableBank_init(
    struct sig_osc_AdditiveWavetableBank* self,
    struct sig_AdditiveWavetableBank* wavetables);

/**
 * @brief Reads the additive wavetable bank at a phase, from the octave
 * for a frequency.
 *
 * The oscillator's phase accumulator is neither read nor updated, so a
 * caller can supply a phase from elsewhere, such as a shared phasor.
 *
 * @param self the oscillator state
 * @param frequency the frequency that selects the octave, in Hz
 * @param phase the phase at which to read, normalized to 0.0-1.0
 * @param tableIndex the waveform to read, in cycles of the bank
 * @return the sample at the phase
 */
inline float sig_osc_AdditiveWavetableBank_readAtPhase(
    struct sig_osc_AdditiveWavetableBank* self, float frequency, float phase,
    float tableIndex);

/**
 * @brief Generates a single sample of the additive wavetable bank's
 * output.
 *
 * The bank contains one set of tables for each of its octaves. The
 * oscillator reads the set for the octave that contains its frequency,
 * interpolating linearly with the set for the octave above. Within the
 * bank's octaves, this keeps its harmonics at or below the bank's maximum
 * harmonic frequency. Phase modulation through phaseOffset can still push
 * harmonics above that limit, because the oscillator chooses tables by
 * frequency alone.
 *
 * @param self the oscillator state
 * @param frequency the oscillator's frequency, in Hz
 * @param phaseOffset the phase offset to apply, in cycles
 * @param tableIndex the waveform to read, in cycles of the bank
 * @param sampleRate the sample rate, in Hz
 * @param eocOut a float pointer into which the end of cycle sample will be
 * written
 * @return the oscillator's output sample
 */
inline float sig_osc_AdditiveWavetableBank_generate(
    struct sig_osc_AdditiveWavetableBank* self, float frequency,
    float phaseOffset, float tableIndex, float sampleRate, float* eocOut);

/**
 * @brief The state of a phasor that runs at a ratio of a reference phase.
 *
 * The ratio is numerator / denominator, transposed by a number of octaves.
 * When the numerator, the denominator, and the octaves are all whole
 * numbers, the phasor locks to the reference. It counts the reference's
 * cycles and corrects its phase toward exactly the ratio times the
 * reference's accumulated phase, so the two phases align at the start of
 * every (transposed) denominator cycles of the reference. Otherwise, the
 * phasor follows the reference at the ratio without locking.
 */
struct sig_osc_RatioPhasor {
    /**
     * @brief The phasor's current phase.
     *
     * Normalized to 0.0-1.0.
     */
    float phase;

    /**
     * @brief The reference phase from the previous sample.
     *
     */
    float previousReference;

    /**
     * @brief The reference cycle within the current period of denominator
     * cycles.
     *
     */
    uint32_t cycle;

    /**
     * @brief The fraction of the phase error that the phasor corrects on
     * each sample.
     *
     * A value of 1.0 corrects the error immediately.
     */
    float correction;

    /**
     * @brief The largest amount by which the phase can advance on each
     * sample.
     *
     * In cycles per sample. When the ratio would advance the phase further,
     * the phasor advances by this amount instead, and does not lock to
     * the reference.
     */
    float maxIncrement;
};

/**
 * @brief Initialises a ratio phasor's state.
 *
 * The phasor starts aligned with a reference phase of 0.0, corrects
 * phase errors immediately, and is limited to the Nyquist frequency.
 *
 * @param self the state to initialise
 */
void sig_osc_RatioPhasor_init(struct sig_osc_RatioPhasor* self);

/**
 * @brief Calculates how quickly a ratio phasor corrects its phase error.
 *
 * When the ratio changes, the phasor's phase stays continuous, and the
 * difference between its phase and the newly locked phase decays with the
 * glide time as its time constant.
 *
 * @param self the state to update
 * @param glide the time constant of the phase correction, in seconds; 0.0
 * corrects the phase immediately
 * @param sampleRate the sample rate, in Hz
 */
void sig_osc_RatioPhasor_calculateCoefficient(
    struct sig_osc_RatioPhasor* self, float glide, float sampleRate);

/**
 * @brief Sets the highest frequency at which a ratio phasor can run.
 *
 * A reference multiplied by a large ratio or transposed up by many octaves
 * can exceed the Nyquist frequency, which aliases. Above the maximum
 * frequency, the phasor runs at the maximum frequency instead, without
 * locking to the reference.
 *
 * @param self the state to update
 * @param maxFrequency the highest frequency, in Hz, up to half the sample
 * rate
 * @param sampleRate the sample rate, in Hz
 */
void sig_osc_RatioPhasor_setMaxFrequency(struct sig_osc_RatioPhasor* self,
    float maxFrequency, float sampleRate);

/**
 * @brief Generates one sample of a ratio phasor's phase.
 *
 * @param self the state to update
 * @param reference the reference phase, normalized to 0.0-1.0, which must
 * advance by less than half a cycle per sample
 * @param numerator the ratio's numerator
 * @param denominator the ratio's denominator
 * @param octaves the number of octaves by which to transpose the ratio
 * @return the phasor's phase, normalized to 0.0-1.0
 */
inline float sig_osc_RatioPhasor_generate(struct sig_osc_RatioPhasor* self,
    float reference, float numerator, float denominator, float octaves);

/**
 * @brief The state of a phasor that follows a train of clock pulses.
 *
 * The phasor completes one cycle per clock period, and wraps at each pulse.
 * It measures the period between each pair of pulses, and runs at the
 * frequency of the most recent period. If the pulses stop, it continues at
 * that frequency.
 *
 * When a pulse arrives before or after the phasor wraps, the phasor
 * does not jump. Instead, it spreads the phase error over the next period,
 * so that it wraps in time for the next pulse.
 */
struct sig_osc_ClockPhasor {
    /**
     * @brief The phasor's current phase.
     *
     * Normalized to 0.0-1.0.
     */
    float phase;

    /**
     * @brief The amount by which the phase advances on each sample.
     *
     * In cycles per sample.
     */
    float increment;

    /**
     * @brief The amount added to the increment while the phasor corrects
     * its phase error.
     *
     * In cycles per sample.
     */
    float correction;

    /**
     * @brief The number of samples for which the phasor continues to
     * correct its phase error.
     *
     */
    uint32_t correctionSamples;

    /**
     * @brief The number of samples since the most recent pulse.
     *
     */
    uint32_t samplesSinceLastPulse;

    /**
     * @brief Whether the phasor has received a pulse.
     *
     * The phasor measures the clock's period from its second pulse onward.
     */
    bool hasPulsed;

    /**
     * @brief The pulse input's value on the previous sample.
     *
     */
    float previousPulse;
};

/**
 * @brief Initialises a clock phasor's state.
 *
 * The phasor's phase stays at 0.0 until it receives its second pulse.
 *
 * @param self the state to initialise
 */
void sig_osc_ClockPhasor_init(struct sig_osc_ClockPhasor* self);

/**
 * @brief Generates one sample of a clock phasor's phase.
 *
 * @param self the state to update
 * @param pulse the clock signal; a pulse begins when it rises to or above
 * the threshold
 * @param threshold the value at which a rising clock signal begins a pulse
 * @return the phasor's phase, normalized to 0.0-1.0
 */
inline float sig_osc_ClockPhasor_generate(struct sig_osc_ClockPhasor* self,
    float pulse, float threshold);

/**
 * @brief The state of a peak frequency follower, which measures how fast
 * a phase advances, and holds the peak.
 *
 * The phase's frequency on each sample is the distance it moved since the
 * previous sample. When the phase is modulated, this is its instantaneous
 * frequency, which can be much higher than its unmodulated frequency.
 * The peak rises immediately, and falls back with a release time.
 */
struct sig_osc_PeakFrequency {
    /**
     * @brief The phase on the previous sample.
     *
     * Normalized to 0.0-1.0.
     */
    float previousPhase;

    /**
     * @brief Whether the follower has received a phase yet.
     *
     */
    bool hasPreviousPhase;

    /**
     * @brief The peak frequency.
     *
     * In Hz.
     */
    float peak;

    /**
     * @brief The amount by which the peak is multiplied on each sample
     * as it falls.
     *
     */
    float release;
};

/**
 * @brief Initialises a peak frequency follower's state.
 *
 * The peak starts at 0 Hz, and falls back immediately.
 *
 * @param self the state to initialise
 */
void sig_osc_PeakFrequency_init(struct sig_osc_PeakFrequency* self);

/**
 * @brief Calculates how quickly a peak frequency follower's peak falls.
 *
 * @param self the state to update
 * @param release the time constant with which the peak falls, in seconds;
 * 0.0 makes it fall immediately
 * @param sampleRate the sample rate, in Hz
 */
void sig_osc_PeakFrequency_calculateCoefficient(
    struct sig_osc_PeakFrequency* self, float release, float sampleRate);

/**
 * @brief Measures the frequency of one sample of a phase, and returns
 * the peak.
 *
 * @param self the state to update
 * @param phase the phase, normalized to 0.0-1.0, which must advance by
 * less than half a cycle per sample
 * @param sampleRate the sample rate, in Hz
 * @return the peak frequency, in Hz
 */
inline float sig_osc_PeakFrequency_generate(
    struct sig_osc_PeakFrequency* self, float phase, float sampleRate);

/**
 * @brief A fast sine approximation implemented using a Chamberlin SVF.
 * This approximation is good up to about 1/6 the sampleRate.
 *
 * For details, see Dattoro "Effect Design Part 3" and
 * https://www.earlevel.com/main/2003/03/02/the-digital-state-variable-filter/
 */
struct sig_osc_FastLFSine {
    float sampleRate;
    float f;
    float sinZ;
    float cosZ;
};

/**
 * @brief Initializes a FastLFSine oscillator.
 * The default frequency is 1 Hz.
 *
 * @param self the oscillator to initialize
 */
void sig_osc_FastLFSine_init(struct sig_osc_FastLFSine* self,
    float sampleRate);

/**
 * @brief Updates the frequency coefficient for the specified frequency.
 *
 * @param self the oscillator instance
 * @param frequency the frequency
 */
inline void sig_osc_FastLFSine_setFrequency(struct sig_osc_FastLFSine* self,
    float frequency);

/**
 * @brief Updates the frequency coefficient using an approximation
 * that is suitable for low frequencies.
 *
 * @param self the oscillator instance
 * @param frequency the frequency
 */
inline void sig_osc_FastLFSine_setFrequencyFast(struct sig_osc_FastLFSine* self,
    float frequency);

/**
 * @brief Generates new values for the sinZ and cosZ delays,
 * which provide the output of the system. cosZ is 90 degrees offset from sinZ.
 *
 * See:
 * https://www.earlevel.com/DigitalAudio/images/StateVarLfoBlock.gif
 *
 * @param self the oscillator instance
 */
inline void sig_osc_FastLFSine_generate(struct sig_osc_FastLFSine* self);

/**
 * An AudioSettings structure holds key information
 * about the current configuration of the audio system,
 * including sample rate, number of channels, and the
 * audio block size.
 */
struct sig_AudioSettings {
    /**
     * The sampling rate in samples per second.
     */
    float sampleRate;

    // TODO: Should this just be an unsigned short? (gh-24)
    /**
     * The number of audio output channels.
     */
    size_t numChannels;

    // TODO: Should this also be an unsigned short? (gh-24)
    /**
     * The number of samples in an audio block.
     */
    size_t blockSize;
};

/**
 * The default audio settings used by Signaletic
 * for 48KHz mono audio with a block size of 48.
 */
static const struct sig_AudioSettings sig_DEFAULT_AUDIOSETTINGS = {
    .sampleRate = 48000.0f,
    .numChannels = 1,
    .blockSize = 48
};


/**
 * Converts a duration from seconds to number of samples.
 *
 * @param audioSettings a pointer to the audio settings
 * @param duration the duration in seconds
 * @return the duration in number of samples
 */
size_t sig_secondsToSamples(struct sig_AudioSettings* audioSettings,
    float duration);


/**
 * A memory heap for an  allocator.
 */
struct sig_AllocatorHeap {
    size_t length;
    void* memory;
};

struct sig_Allocator;

/**
 * Type definition for an Allocator init function.
 */
typedef void (*sig_Allocator_init)(struct sig_Allocator* allocator);

/**
 * Type definition for an Allocator malloc function.
 */
typedef void* (*sig_Allocator_malloc)(struct sig_Allocator* allocator,
    size_t size);

/**
 * Type definition for an Allocator free function.
 */
typedef void (*sig_Allocator_free)(struct sig_Allocator* allocator,
    void* obj);

/**
 * A memory allocator implementation that provides
 * function pointers for essential memory operations.
 */
struct sig_AllocatorImpl {
    sig_Allocator_init init;
    sig_Allocator_malloc malloc;
    sig_Allocator_free free;
};

struct sig_Allocator {
    struct sig_AllocatorImpl* impl;
    struct sig_AllocatorHeap* heap;
};

#define sig_MALLOC(allocator, T) ((T*) (allocator)->impl->malloc( \
    (allocator), sizeof(T)))

/**
 * TLSF Allocator init function.
 */
void sig_TLSFAllocator_init(struct sig_Allocator* allocator);

/**
 * TLSF Allocator malloc function.
 */
void* sig_TLSFAllocator_malloc(struct sig_Allocator* allocator,
    size_t size);

/**
 * TLSF Allocator free function.
 */
void sig_TLSFAllocator_free(struct sig_Allocator* allocator,
    void* obj);

/**
 * A realtime-capable memory allocator based on
 * Matt Conte's TLSF library.
 */
extern struct sig_AllocatorImpl sig_TLSFAllocatorImpl;


/**
 * @brief An xoshiro128+-based pseudo-random number generator.
 *
 * xoshiro128+ 1.0 by David Blackman and
 * Sebastiano Vigna (public domain, https://prng.di.unimi.it/).
 */
struct sig_Random {
    uint32_t seed;
    uint32_t state[4];
};

/**
 * @brief Allocates and initializes a new random number generator,
 * seeded with the specified value.
 *
 * @param allocator the allocator to use
 * @param seed the seed value
 * @return a pointer to the new generator
 */
struct sig_Random* sig_Random_new(struct sig_Allocator* allocator,
    uint32_t seed);

/**
 * @brief Initializes a random number generator and seeds it.
 *
 * @param self the generator to initialize
 * @param seed the seed value
 */
void sig_Random_init(struct sig_Random* self, uint32_t seed);

/**
 * @brief Seeds the generator's state.
 *
 * @param self the generator to seed
 * @param seed the seed value
 */
void sig_Random_seed(struct sig_Random* self, uint32_t seed);

/**
 * @brief Generates the next 32-bit unsigned integer
 * in the sequence.
 *
 * @param self the generator
 * @return a pseudo-random 32-bit unsigned integer
 */
uint32_t sig_Random_nextU32(struct sig_Random* self);

/**
 * @brief Generates a random float between 0.0 and 1.0 exclusive.
 * Every output is an exact multiple of 2^-24.
 *
 * @param self the generator
 * @return a pseudo-random unquantized float
 */
float sig_Random_nextFloat(struct sig_Random* self);

/**
 * @brief Generates a random float between 0.0 and 1.0 inclusive.
 *
 * @param self the generator
 * @return a pseudo-random float
 */
float sig_Random_generate(struct sig_Random* self);

/**
 * @brief Destroys the random number generator.
 *
 * @param allocator the allocator to use
 * @param self the generator to destroy
 */
void sig_Random_destroy(struct sig_Allocator* allocator,
    struct sig_Random* self);


enum sig_Result {
    SIG_RESULT_NONE,
    SIG_RESULT_SUCCESS,
    SIG_ERROR_INDEX_OUT_OF_BOUNDS,
    SIG_ERROR_EXCEEDS_CAPACITY
};

struct sig_Status {
    enum sig_Result result;
};

struct sig_Status* sig_Status_new(struct sig_Allocator* allocator);

void sig_Status_init(struct sig_Status* status);

void sig_Status_reset(struct sig_Status* status);

inline void sig_Status_reportResult(struct sig_Status* status,
    enum sig_Result result);


/**
 * A mutable list, which stores pointers to any type of object ("items").
 */
struct sig_List {
    void** items;
    size_t capacity;
    size_t length;
};

/**
 * @brief Initializes a new List with the specified storage capacity.
 *
 * @param allocator the allocator to use
 * @param capacity the maximum number of entries that the list can hold
 * @return struct sig_List* the new List instance
 */
struct sig_List* sig_List_new(struct sig_Allocator* allocator,
    size_t capacity);

/**
 * @brief Initializes a List.
 *
 * @param self the sig_List struct
 * @param items an array of pointers to use to store list members
 * @param capacity the maximum number of entries that the list can hold
 */
void sig_List_init(struct sig_List* self, void** items, size_t capacity);

/**
 * @brief Inserts an item into the list at the given position.
 *
 * @param self the List instance to insert into
 * @param index the position at which to insert the item
 * @param item the item to add
 * @param status a pointer to the result of the operation
 */
void sig_List_insert(struct sig_List* self, size_t index, void* item,
    struct sig_Status* status);

/**
 * @brief Adds the item to the end of the list.
 *
 * @param self the list to append into
 * @param item the item to append
 * @param status a pointer to the result of the operation
 */
void sig_List_append(struct sig_List* self, void* item,
    struct sig_Status* status);

/**
 * @brief Removes the last item from the list, and returns it.
 *
 * @param self the list from which to pop the last item
 * @return void* the item that was removed from the list
 * @param status a pointer to the result of the operation
 */
void* sig_List_pop(struct sig_List* self, struct sig_Status* status);

/**
 * @brief Removes an item from the given index, and returns it.
 *
 * @param self the list from which to remove an item
 * @param index the index at which to remove the item
 * @param status a pointer to the result of the operation
 * @return void* the item that was removed
 */
void* sig_List_remove(struct sig_List* self, size_t index,
    struct sig_Status* status);

/**
 * @brief Frees a List instance and its underlying item storage
 * (but not items themselves).
 *
 * @param allocator the allocator to use
 * @param self the List to destroy
 */
void sig_List_destroy(struct sig_Allocator* allocator,
    struct sig_List* self);



/**
 * Allocates a new AudioSettings instance with
 * the values from sig_DEFAULT_AUDIO_SETTINGS.
 *
 * @param allocator the memory allocator to use
 * @return the AudioSettings
 */
struct sig_AudioSettings* sig_AudioSettings_new(struct sig_Allocator* allocator);

/**
 * Destroys an AudioSettings instance.
 *
 * @param allocator the memory allocator to use
 * @param self the AudioSettings instance to destroy
 */
void sig_AudioSettings_destroy(struct sig_Allocator* allocator,
    struct sig_AudioSettings* self);



// TODO: Move to the DSP namespace.
struct sig_SignalContext {
    struct sig_AudioSettings* audioSettings;
    struct sig_Buffer* emptyBuffer;
    struct sig_DelayLine* oneSampleDelayLine;
    struct sig_dsp_ConstantValue* silence;
    struct sig_dsp_ConstantValue* unity;
};

struct sig_SignalContext* sig_SignalContext_new(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings);

void sig_SignalContext_destroy(
    struct sig_Allocator* allocator, struct sig_SignalContext* self);


/**
 * A Buffer holds an array of samples and its length.
 */
struct sig_Buffer {
    /**
     * The number of samples the buffer can store.
     */
    size_t length;

    /**
     * A pointer to the samples.
     */
    float_array_ptr samples;
};

/**
 * Allocates a new Buffer of the specified length.
 *
 * @param allocator the memory allocator to use
 * @param length the number of samples this Buffer should store
 */
struct sig_Buffer* sig_Buffer_new(struct sig_Allocator* allocator,
    size_t length);

/**
 * Fills a buffer using the specified array fill function.
 *
 * @param self the buffer to fill
 * @param filler a pointer to an array filler
 */
void sig_Buffer_fill(struct sig_Buffer* self, sig_array_filler filler);

/**
 * Fills a Buffer instance with a value.
 *
 * @param self the buffer instance to fill
 * @param value the value to which all samples will be set
 */
void sig_Buffer_fillWithValue(struct sig_Buffer* self, float value);

/**
 * Fills a Buffer with silence.
 *
 * @param self the buffer to fill with zeroes
 */
void sig_Buffer_fillWithSilence(struct sig_Buffer* self);

/**
 * Fills a Buffer with a waveform using the specified
 * waveform generator function.
 *
 * @param self the buffer to fill
 * @param generate a pointer to a waveform generator function
 * @param sampleRate the sample rate at which to generate the waveform
 * @param phase the initial phase of the waveform
 * @param freq the frequency of the waveform
 */
void sig_Buffer_fillWithWaveform(struct sig_Buffer* self,
    sig_waveform_generator generate, float sampleRate,
    float phase, float freq);

/**
 * Reads from the buffer, truncating the fractional portion of the index.
 *
 * @param self the buffer from which to read
 * @param idx the index (can be fractional) at which to read a sample
 * @return the linearly interpolated sample
 */
float sig_Buffer_read(struct sig_Buffer* self, float idx);

/**
 * Reads from the buffer using linear interpolation.
 *
 * @param self the buffer from which to read
 * @param idx the index (can be fractional) at which to read a sample
 * @return the linearly interpolated sample
 */
float sig_Buffer_readLinear(struct sig_Buffer* self, float idx);

/**
 * Reads from the buffer using linear interpolation using
 * a phase value between 0.0 and 1.0.
 *
 * @param self the buffer from which to read
 * @param phase the phase value from which the index will be derived
 * @return the linearly interpolated sample
 */
inline float sig_Buffer_readLinearAtPhase(struct sig_Buffer* self, float phase);

/**
 * @brief The precomputed factors for a fast Fourier transform of one
 * length.
 *
 * The transforms work in place on separate arrays of real and imaginary
 * parts, using the radix-2 Cooley-Tukey algorithm.
 */
struct sig_FFT {
    /**
     * @brief The number of samples in each transform.
     *
     * Must be a power of two.
     */
    size_t length;

    /**
     * @brief The cosine of each twiddle factor's angle.
     *
     * Contains length / 2 values, cos(2pi * m / length).
     */
    float_array_ptr cosines;

    /**
     * @brief The sine of each twiddle factor's angle.
     *
     * Contains length / 2 values, sin(2pi * m / length).
     */
    float_array_ptr sines;
};

/**
 * @brief Initialises an FFT, calculating its twiddle factors.
 *
 * @param self the FFT to initialise
 * @param length the number of samples in each transform, which must be a
 * power of two
 * @param cosines an array of length / 2 samples, to fill with the cosines
 * of the twiddle factors' angles
 * @param sines an array of length / 2 samples, to fill with the sines of
 * the twiddle factors' angles
 */
void sig_FFT_init(struct sig_FFT* self, size_t length,
    float_array_ptr cosines, float_array_ptr sines);

/**
 * @brief Allocates and initialises an FFT.
 *
 * @param allocator the allocator to use
 * @param length the number of samples in each transform, which must be a
 * power of two
 * @return the new FFT
 */
struct sig_FFT* sig_FFT_new(struct sig_Allocator* allocator, size_t length);

/**
 * @brief Transforms a signal into its spectrum, in place.
 *
 * Calculates X[k] = sum over n of x[n] * e^(-i * 2pi * k * n / length).
 *
 * @param self the FFT
 * @param real the real parts of the signal, which are replaced by the real
 * parts of the spectrum
 * @param imaginary the imaginary parts of the signal, which are replaced by
 * the imaginary parts of the spectrum
 */
void sig_FFT_forward(struct sig_FFT* self, float_array_ptr real,
    float_array_ptr imaginary);

/**
 * @brief Transforms a spectrum into its signal, in place.
 *
 * Calculates x[n] = sum over k of X[k] * e^(i * 2pi * k * n / length).
 * The result is not divided by the length, so a forward transform followed
 * by an inverse transform multiplies the signal by the length.
 *
 * @param self the FFT
 * @param real the real parts of the spectrum, which are replaced by the
 * real parts of the signal
 * @param imaginary the imaginary parts of the spectrum, which are replaced
 * by the imaginary parts of the signal
 */
void sig_FFT_inverse(struct sig_FFT* self, float_array_ptr real,
    float_array_ptr imaginary);

/**
 * @brief Frees an FFT.
 *
 * @param allocator the allocator that created the FFT
 * @param self the FFT to free
 */
void sig_FFT_destroy(struct sig_Allocator* allocator, struct sig_FFT* self);

/**
 * @brief Adds one harmonic of a sine table into a Buffer.
 *
 * @param self the buffer to add into
 * @param sineTable a buffer containing exactly one cycle of a sine wave,
 * the same length as self
 * @param harmonic the harmonic number, which must be less than half the
 * length of the buffer
 * @param gain the gain to apply to the harmonic
 * @param phase the starting phase of the harmonic, in cycles
 */
void sig_Buffer_addHarmonic(struct sig_Buffer* self,
    struct sig_Buffer* sineTable, size_t harmonic, float gain, float phase);

/**
 * @brief Fills a Buffer with one cycle of a band-limited waveform made of
 * its first numHarmonics harmonics.
 *
 * Writes each harmonic into one bin of a spectrum, then transforms the
 * spectrum into the waveform with one inverse FFT. The cost does not depend
 * on the number of harmonics.
 *
 * @param self the buffer to fill, whose length must be a power of two
 * @param fft an FFT of the same length as self
 * @param imaginary a scratch array of the same length as self, which is
 * overwritten
 * @param shape the waveform to synthesize
 * @param numHarmonics the number of harmonics, which must be less than half
 * the length of the buffer
 * @param taper the taper to apply to the harmonics
 */
void sig_Buffer_fillWithHarmonics(struct sig_Buffer* self,
    struct sig_FFT* fft, float_array_ptr imaginary,
    const struct sig_WaveformShape* shape, size_t numHarmonics,
    enum sig_HarmonicTaper taper);

/**
 * Reads from the buffer using cubic interpolation.
 *
 * @param self the buffer from which to read
 * @param idx the index (can be fractional) at which to read a sample
 * @return the cubic interpolated sample
 */
float sig_Buffer_readCubic(struct sig_Buffer* self, float idx);

/**
 * Destroys a Buffer and frees its memory.
 *
 * After calling this function, the pointer to
 * the buffer instance will no longer be usable.
 *
 * @param allocator the memory allocator to use
 * @param self the buffer instance to destroy
 */
void sig_Buffer_destroy(struct sig_Allocator* allocator, struct sig_Buffer* self);


/**
 * Creates a new Buffer that references a portion of another Buffer.
 * Be aware that destroying the parent buffer will
 * invalidate any BufferViews that have been created from it.
 *
 * @param allocator the allocator to use
 * @param buffer the parent buffer
 * @param startIdx the index in the parent buffer to start at
 * @param length the length of the subbuffer
 */
struct sig_Buffer* sig_BufferView_new(
    struct sig_Allocator* allocator, struct sig_Buffer* buffer,
    size_t startIdx, size_t length);

/**
 * Destroys a BufferView.
 * Note that this will not free the samples array,
 * since it is a shared object borrowed from another Buffer.
 *
 * @param allocator the allocator to use
 * @param self the subbuffer to destroy
 */
void sig_BufferView_destroy(struct sig_Allocator* allocator,
    struct sig_Buffer* self);


/**
 * @brief An array of sig_Buffers representing a wavetable.
 *
 */
struct sig_WavetableBank {
    size_t length;
    struct sig_Buffer** waves;
};

struct sig_WavetableBank* sig_WavetableBank_new(struct sig_Allocator* allocator,
    size_t numTables, size_t tableLength);

/**
 * @brief Reads from the bank using linear interpolation between samples
 * and between tables.
 *
 * @param self the wavetable bank
 * @param tableIdx the index of the table to read; fractional values
 * crossfade between neighbouring tables, the last table crossfades back
 * into the first, and values outside 0 to length wrap around
 * @param phase the phase to read at, normalized to 0.0-1.0
 * @return the interpolated sample
 */
inline float sig_WavetableBank_readLinearAtPhase(struct sig_WavetableBank* self,
    float tableIdx, float phase);

void sig_WavetableBank_destroy(struct sig_Allocator* allocator,
    struct sig_WavetableBank* self);

/**
 * @brief A set of wavetable banks, one per octave, whose tables are
 * band-limited for the frequencies in that octave.
 *
 * The bank synthesizes its tables additively from each shape's harmonic
 * series, so it can only contain shapes whose harmonic series is known,
 * such as the shapes in sig_STANDARD_WAVEFORMS.
 * Each level contains one table for each of the bank's shapes, in the
 * order they were supplied. Level n is used for frequencies from
 * lowestFrequency * 2^n up to lowestFrequency * 2^(n + 1), and contains
 * only the harmonics that remain at or below maxHarmonicFrequency at the
 * top of that octave.
 */
struct sig_AdditiveWavetableBank {
    /**
     * @brief The shapes whose tables the bank contains.
     *
     * The bank allocates its own copy of the shapes it was created with.
     */
    struct sig_WaveformShape* shapes;

    /**
     * @brief The number of shapes.
     *
     */
    size_t numShapes;

    /**
     * @brief The number of octave levels.
     *
     */
    size_t numLevels;

    /**
     * @brief The number of samples in each table.
     *
     */
    size_t tableLength;

    /**
     * @brief The lowest frequency in the bottom octave.
     */
    float lowestFrequency;

    /**
     * @brief The highest frequency that any harmonic may reach.
     */
    float maxHarmonicFrequency;

    /**
     * @brief The taper applied to each level's harmonics.
     */
    enum sig_HarmonicTaper taper;

    /**
     * @brief The wavetable bank for each octave level, lowest first.
     */
    struct sig_WavetableBank** levels;
};

/**
 * @brief Allocates an additive wavetable bank and generates its tables.
 *
 * The tables require a total of numLevels * numShapes * tableLength
 * samples.
 *
 * @param allocator the allocator to use
 * @param shapes the shapes to synthesize, such as sig_STANDARD_WAVEFORMS;
 * the bank copies them
 * @param numShapes the number of shapes
 * @param numLevels the number of octave levels
 * @param tableLength the number of samples in each table, which must be a
 * power of two
 * @param lowestFrequency the frequency at the bottom of the lowest level
 * @param maxHarmonicFrequency the highest frequency any harmonic may
 * reach, in Hz
 * @param taper the taper to apply to each level's harmonics
 * @return the new additive wavetable bank
 */
struct sig_AdditiveWavetableBank* sig_AdditiveWavetableBank_new(
    struct sig_Allocator* allocator, const struct sig_WaveformShape* shapes,
    size_t numShapes, size_t numLevels, size_t tableLength,
    float lowestFrequency, float maxHarmonicFrequency,
    enum sig_HarmonicTaper taper);

/**
 * @brief Calculates how many harmonics a level's tables contain.
 *
 * @param self the additive wavetable bank
 * @param level the octave level
 * @return the number of harmonics, which is at least one and less than
 * half the table length
 */
size_t sig_AdditiveWavetableBank_numHarmonics(
    struct sig_AdditiveWavetableBank* self, size_t level);

/**
 * @brief Synthesizes the tables for all shapes at all levels.
 *
 * @param self the additive wavetable bank
 * @param fft an FFT of the bank's table length
 * @param imaginary a scratch array of the bank's table length, which is
 * overwritten
 */
void sig_AdditiveWavetableBank_fill(
    struct sig_AdditiveWavetableBank* self, struct sig_FFT* fft,
    float_array_ptr imaginary);

/**
 * @brief Calculates which octave level to read for a frequency.
 *
 * @param self the additive wavetable bank
 * @param frequency the frequency; negative frequencies use the level of their magnitude
 * @return the level, where fractional values fall between two levels,
 * clamped to the range of available levels
 */
inline float sig_AdditiveWavetableBank_levelForFrequency(
    struct sig_AdditiveWavetableBank* self, float frequency);

/**
 * @brief Reads from the bank using linear interpolation between samples,
 * waveforms, and octave levels.
 *
 * @param self the additive wavetable bank
 * @param level the octave level to read, which must be between 0 and
 * numLevels - 1; fractional values crossfade between two levels
 * @param tableIdx the index of the shape to read; fractional values
 * crossfade between neighbouring shapes, and values outside 0 to
 * numShapes wrap around
 * @param phase the phase to read at, normalized to 0.0-1.0
 * @return the interpolated sample
 */
inline float sig_AdditiveWavetableBank_readLinearAtPhase(
    struct sig_AdditiveWavetableBank* self, float level, float tableIdx,
    float phase);

void sig_AdditiveWavetableBank_destroy(struct sig_Allocator* allocator,
    struct sig_AdditiveWavetableBank* self);


/**
 * @brief Allocates an array of samples.
 *
 * The samples are not initialised.
 *
 * @param allocator the allocator to use
 * @param length the number of samples in the array
 * @return the new array
 */
float_array_ptr sig_samples_new(struct sig_Allocator* allocator,
    size_t length);

float_array_ptr sig_AudioBlock_new(struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings);
float_array_ptr sig_AudioBlock_newWithValue(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    float value);
float_array_ptr sig_AudioBlock_newSilent(struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings);
void sig_AudioBlock_destroy(struct sig_Allocator* allocator,
    float_array_ptr self);



/**
 * @brief A modulatable delay line
 * with support for comb and allpass configurations.
 *
 */
struct sig_DelayLine {
    struct sig_Buffer* buffer;
    size_t writeIdx;
};

struct sig_DelayLine* sig_DelayLine_new(struct sig_Allocator* allocator,
    size_t maxDelayLength);

struct sig_DelayLine* sig_DelayLine_newSeconds(struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings, float maxDelaySecs);

struct sig_DelayLine* sig_DelayLine_newWithTransferredBuffer(
    struct sig_Allocator* allocator, struct sig_Buffer* buffer);

void sig_DelayLine_init(struct sig_DelayLine* self);

typedef void (*sig_DelayLine_readFn)(void* signal);

/**
 * @brief Reads from the delay line without interpolation.
 * This function does not support fractional delay times.
 *
 * @param self the delay line to read from
 * @param readPos the position (in samples) at which to read from the delay
 * @return float the output of the delay line
 */
inline float sig_DelayLine_readAt(struct sig_DelayLine* self, size_t readPos);

/**
 * @brief Reads from the delay line using linear interpolation.
 * This function supports fractional delay times.
 *
 * @param self the delay line to read from
 * @param readPos the position (in samples) at which to read from the delay
 * @return float the output of the delay line
 */
inline float sig_DelayLine_linearReadAt(struct sig_DelayLine* self, float readPos);

/**
 * @brief Reads from the delay line using cubic interpolation.
 * This function supports fractional delay times.
 *
 * @param self the delay line to read from
 * @param readPos the position (in samples) at which to read from the delay
 * @return float the output of the delay line
 */
inline float sig_DelayLine_cubicReadAt(struct sig_DelayLine* self, float readPos);

/**
 * @brief Reads from the delay line using allpass interpolation.
 * This function supports fractional delay times.
 *
 * @param self the delay line to read from
 * @param readPos the position (in samples) to read from the delay at
 * @param previousSample the previous interpolated sample from the delay line
 * @return float the output of the delay line
 */
inline float sig_DelayLine_allpassReadAt(struct sig_DelayLine* self,
    float readPos, float previousSample);

/**
 * @brief Reads from the delay line without interpolation.
 * This function does not support fractional delay times, and will truncate
 * the delay time tap down to the nearest sample.
 *
 * @param self the delay line to read from
 * @param tapTime the time (in seconds) at which to read from the delay
 * @param sampleRate the current sample rate
 * @return float the output of the delay line
 */
inline float sig_DelayLine_readAtTime(struct sig_DelayLine* self, float source,
    float tapTime, float sampleRate);

/**
 * @brief Reads from the delay line at the specified time
 * (in seconds) using linear interpolation.
 * This function supports fractional delay times.
 *
 * @param self the delay line to read from
 * @param tapTime the time (in seconds) at which to read from the delay
 * @param sampleRate the current sample rate
 * @return float the output of the delay line
 */
inline float sig_DelayLine_linearReadAtTime(struct sig_DelayLine* self, float source,
    float tapTime, float sampleRate);

/**
 * @brief Reads from the delay line at the specified time
 * (in seconds) using cubic interpolation.
 * This function supports fractional delay times.
 *
 * @param self the delay line to read from
 * @param tapTime the time (in seconds) at which to read from the delay
 * @param sampleRate the current sample rate
 * @return float the output of the delay line
 */
inline float sig_DelayLine_cubicReadAtTime(struct sig_DelayLine* self, float source,
    float tapTime, float sampleRate);

/**
 * @brief Reads from the delay line at the specified time using allpass interpolation.
 *
 * This function supports fractional delay times. All times are in seconds.
 *
 * @param self the delay line to read from
 * @param tapTime the time at which to read from the delay
 * @param sampleRate the current sample rate
 * @param previousSample the previous interpolated sample from the delay line
 * @return float the output of the delay line
 */
inline float sig_DelayLine_allpassReadAtTime(struct sig_DelayLine* self,
    float source, float tapTime, float sampleRate, float previousSample);

inline float sig_DelayLine_readAtTimes(struct sig_DelayLine* self, float source,
    float_array_ptr SIG_RESTRICT tapTimes,
    float_array_ptr SIG_RESTRICT tapGains, size_t numTaps,
    float sampleRate, float timeScale);

inline float sig_DelayLine_linearReadAtTimes(struct sig_DelayLine* self,
    float source, float_array_ptr SIG_RESTRICT tapTimes,
    float_array_ptr SIG_RESTRICT tapGains,
    size_t numTaps, float sampleRate, float timeScale);

inline float sig_DelayLine_cubicReadAtTimes(struct sig_DelayLine* self,
    float source, float_array_ptr SIG_RESTRICT tapTimes,
    float_array_ptr SIG_RESTRICT tapGains,
    size_t numTaps, float sampleRate, float timeScale);

inline void sig_DelayLine_write(struct sig_DelayLine* self, float sample);

inline float sig_DelayLine_calcFeedbackGain(float delayTime, float decayTime);

inline float sig_DelayLine_feedback(float sample, float read, float g);

inline float sig_DelayLine_comb(struct sig_DelayLine* self, float sample,
    size_t readPos, float g);

/**
 * @brief A linearly interpolated feedback comb filter.
 *
 * This function supports fractional read positions.
 *
 * @param self the delay line to read from and write to
 * @param sample the input sample
 * @param readPos the sample position in the delay line to read from, relative to the last write
 * @param g the feedback gain applied to the delayed sample before it is
 * mixed with the input and written back into the delay line
 * @return float the delayed sample read from the delay line
 */
inline float sig_DelayLine_linearComb(struct sig_DelayLine* self, float sample,
    float readPos, float g);

inline float sig_DelayLine_cubicComb(struct sig_DelayLine* self, float sample,
    float readPos, float g);

inline float sig_DelayLine_allpass(struct sig_DelayLine* self, float sample,
    size_t readPos, float g);

inline float sig_DelayLine_linearAllpass(struct sig_DelayLine* self,
    float sample, float readPos, float g);

inline float sig_DelayLine_cubicAllpass(struct sig_DelayLine* self, float sample,
    float readPos, float g);

void sig_DelayLine_destroy(struct sig_Allocator* allocator,
    struct sig_DelayLine* self);

inline float sig_linearXFade(float left, float right, float mix);

inline float sig_sineWavefolder(float x, float gain, float factor);

/**
 * @brief Soft clips a sample to the range -1.0 to 1.0.
 *
 * Below the threshold, the sample will pass through unchanged.
 * Above it, the sample will be compressed, approaching full scale
 * asymptotically without exceeding it.
 *
 * The threshold is specified in the range 0.0 to 0.99. A value
 * of 0.0 compresses the whole signal.
 *
 * @param x the sample to clip
 * @param threshold the magnitude above which compression begins
 * @return float the clipped sample
 */
inline float sig_softClip(float x, float threshold);

/**
 * @brief An onset detector that converts audio into an onset strength
 * envelope.
 *
 * The detector follows the SuperFlux design (Bock and Widmer, 2013),
 * with adaptive whitening (Stowell and Plumbley, 2007). It analyses
 * one Hann-windowed frame per hop. A filterbank of triangular filters,
 * 24 per octave from 30 Hz to 6 kHz or 0.45 times the sample rate,
 * reduces each frame's magnitude spectrum to logarithmically spaced
 * bands. The detector divides each band by the largest of its peak
 * follower, -30 dB of the loudest band's peak, and the threshold, and
 * compresses the result with ln(1 + x). The envelope value is the
 * weighted mean over bands of each band's increase over the largest of
 * the three neighbouring bands in the frame diffLagHops hops earlier,
 * less 0.022, and clipped at zero. Each band's weight is half the
 * distance between its neighbours' centres, in steps of 1/24 octave,
 * capped at 2, which gives the merged low-frequency bands their share
 * of the envelope. The envelope therefore lies between 0 and ln 2,
 * and is exactly zero in silence and in noise below the threshold.
 */
struct sig_tempo_OnsetDetector {
    /**
     * @brief The sample rate in Hz.
     */
    float sampleRate;

    /**
     * @brief The number of samples in each analysis frame.
     *
     * Must be a power of two. The frame is also the FFT length.
     */
    size_t windowLength;

    /**
     * @brief The number of samples between the starts of consecutive
     * frames.
     */
    size_t hopLength;

    /**
     * @brief The number of hops between a frame and the earlier frame
     * that it is compared with.
     */
    size_t diffLagHops;

    /**
     * @brief The level below which the detector ignores sound.
     *
     * In dB relative to a full-scale sine.
     */
    float threshold;

    /**
     * @brief The threshold as a band magnitude, where a full-scale
     * sine's peak bin has a magnitude of about 1.
     */
    float thresholdMagnitude;

    /**
     * @brief The number of bands in the filterbank.
     */
    size_t numBands;

    /**
     * @brief The FFT that analyses frames.
     *
     * Its length must be windowLength.
     */
    struct sig_FFT* fft;

    /**
     * @brief The Hann window applied to each frame.
     *
     * Contains windowLength samples.
     */
    float_array_ptr window;

    /**
     * @brief The ring buffer that collects the samples of the current
     * frame.
     *
     * Contains windowLength samples.
     */
    float_array_ptr frame;

    /**
     * @brief The real parts of the current frame's spectrum.
     *
     * Contains windowLength samples.
     */
    float_array_ptr real;

    /**
     * @brief The imaginary parts of the current frame's spectrum.
     *
     * Contains windowLength samples.
     */
    float_array_ptr imaginary;

    /**
     * @brief The FFT bins at the filterbank's centre frequencies.
     *
     * Contains numBands + 2 bin indices, in increasing order. Band j
     * spans from bin bandEdges[j] to bin bandEdges[j + 2], and peaks at
     * bin bandEdges[j + 1].
     */
    float_array_ptr bandEdges;

    /**
     * @brief Each band's weight in the envelope.
     *
     * Contains numBands values.
     */
    float_array_ptr bandWeights;

    /**
     * @brief The sum of the band weights.
     */
    float totalWeight;

    /**
     * @brief Each band's peak follower, which whitens the bands.
     *
     * Contains numBands values.
     */
    float_array_ptr bandPeaks;

    /**
     * @brief The whitened and compressed bands of the most recent
     * frames.
     *
     * A ring buffer of diffLagHops + 1 rows of numBands values.
     */
    float_array_ptr bandRows;

    /**
     * @brief The position in frame where the next sample is written,
     * wrapping at windowLength.
     */
    size_t frameWriteIdx;

    /**
     * @brief The number of samples since the last analysis, from 0 to
     * hopLength - 1.
     */
    size_t hopIdx;

    /**
     * @brief The row of bandRows that contains the newest frame's
     * bands.
     */
    size_t currentRow;

    /**
     * @brief Whether the most recent call to generate completed an
     * analysis.
     */
    bool hasNewValue;

    /**
     * @brief The most recent onset strength value.
     */
    float envelope;
};

/**
 * @brief Calculates the number of bands in an onset detector's
 * filterbank.
 *
 * @param sampleRate the sample rate in Hz
 * @param windowLength the number of samples in each analysis frame
 * @return the number of bands
 */
size_t sig_tempo_OnsetDetector_calcNumBands(float sampleRate,
    size_t windowLength);

/**
 * @brief Initialises an onset detector, filling its window and
 * filterbank, and clearing its buffers and state.
 *
 * @param self the detector to initialise
 * @param sampleRate the sample rate in Hz
 * @param windowLength the number of samples in each analysis frame,
 * which must be a power of two
 * @param hopLength the number of samples between the starts of
 * consecutive frames
 * @param diffLagHops the number of hops between a frame and the earlier
 * frame that it is compared with
 * @param threshold the level below which the detector ignores sound, in
 * dB relative to a full-scale sine
 * @param fft an FFT of windowLength samples
 * @param window an array of windowLength samples, which is filled with
 * the Hann window
 * @param frame an array of windowLength samples, which is cleared
 * @param real an array of windowLength samples, which is cleared
 * @param imaginary an array of windowLength samples, which is cleared
 * @param bandEdges an array of numBands + 2 values, which is filled
 * with the filterbank's bins, where numBands is the value that
 * sig_tempo_OnsetDetector_calcNumBands returns
 * @param bandWeights an array of numBands values, which is filled with
 * the bands' weights
 * @param bandPeaks an array of numBands values, which is cleared
 * @param bandRows an array of (diffLagHops + 1) * numBands values,
 * which is cleared
 */
void sig_tempo_OnsetDetector_init(struct sig_tempo_OnsetDetector* self,
    float sampleRate, size_t windowLength, size_t hopLength,
    size_t diffLagHops, float threshold, struct sig_FFT* fft,
    float_array_ptr window, float_array_ptr frame, float_array_ptr real,
    float_array_ptr imaginary, float_array_ptr bandEdges,
    float_array_ptr bandWeights, float_array_ptr bandPeaks,
    float_array_ptr bandRows);

/**
 * @brief Allocates and initialises an onset detector.
 *
 * @param allocator the allocator to use
 * @param sampleRate the sample rate in Hz
 * @param windowLength the number of samples in each analysis frame,
 * which must be a power of two
 * @param hopLength the number of samples between the starts of
 * consecutive frames
 * @param diffLagHops the number of hops between a frame and the earlier
 * frame that it is compared with
 * @param threshold the level below which the detector ignores sound, in
 * dB relative to a full-scale sine
 * @return the new detector
 */
struct sig_tempo_OnsetDetector* sig_tempo_OnsetDetector_new(
    struct sig_Allocator* allocator, float sampleRate, size_t windowLength,
    size_t hopLength, size_t diffLagHops, float threshold);

/**
 * @brief Processes one sample and reports the current onset strength.
 *
 * The detector collects samples into a frame and analyses the frame
 * once per hop. The returned value changes only when the detector
 * completes an analysis.
 *
 * @param self the detector
 * @param sample the input sample
 * @return the current onset strength, from 0 to ln 2
 */
float sig_tempo_OnsetDetector_generate(struct sig_tempo_OnsetDetector* self,
    float sample);

/**
 * @brief Reports whether the most recent call to generate completed an
 * analysis and produced a new envelope value.
 *
 * @param self the detector
 * @return true if the most recent sample completed an analysis
 */
inline bool sig_tempo_OnsetDetector_hasNewValue(
    struct sig_tempo_OnsetDetector* self);

/**
 * @brief Sets the level below which the detector ignores sound.
 *
 * @param self the detector
 * @param threshold the threshold in dB relative to a full-scale sine,
 * from -90 to -30
 */
inline void sig_tempo_OnsetDetector_setThreshold(
    struct sig_tempo_OnsetDetector* self, float threshold);

/**
 * @brief Frees an onset detector.
 *
 * @param allocator the allocator that created the detector
 * @param self the detector to free
 */
void sig_tempo_OnsetDetector_destroy(struct sig_Allocator* allocator,
    struct sig_tempo_OnsetDetector* self);

/**
 * @brief A tempo estimator that tracks the tempo of an onset strength
 * envelope.
 *
 * The estimator keeps an 8-second window of envelope values. Every 16
 * values, it scores each candidate tempo in its current range, in steps
 * of 1 BPM, by autocorrelating the window at the candidate's beat
 * interval and at twice that interval. A log-normal prior, centred at
 * 120 BPM with a width of one octave, weights the scores, so that
 * octave-related tempos resolve to a consistent metrical level. The
 * estimator adopts the best candidate immediately when it lies within
 * 2 BPM of the current estimate. A more distant candidate must score 30
 * percent higher than the current estimate to replace it. When the
 * estimate has stayed within 8 percent of itself for 4 seconds, a
 * candidate within 8 percent of 2, 3, or 4 times or a half, third, or
 * quarter of it must score twice as high. When the window contains no
 * onsets, the estimator holds its last estimate.
 */
struct sig_tempo_TempoEstimator {
    /**
     * @brief The number of envelope values per second.
     */
    float envelopeRate;

    /**
     * @brief The slowest tempo that the estimator supports, in beats
     * per minute.
     *
     * The window contains four beats at this tempo.
     */
    float lowestTempo;

    /**
     * @brief The fastest tempo that the estimator supports, in beats
     * per minute.
     */
    float highestTempo;

    /**
     * @brief The slowest tempo that the estimator currently considers,
     * in beats per minute.
     */
    float minTempo;

    /**
     * @brief The fastest tempo that the estimator currently considers,
     * in beats per minute.
     */
    float maxTempo;

    /**
     * @brief The centre of the tempo prior, in beats per minute.
     */
    float priorCenter;

    /**
     * @brief The width of the tempo prior, in octaves.
     */
    float priorWidth;

    /**
     * @brief The number of envelope values that the window contains.
     */
    size_t windowLength;

    /**
     * @brief The number of envelope values between estimations.
     */
    size_t estimateInterval;

    /**
     * @brief The number of envelope values required before the first
     * estimation.
     */
    size_t minimumValues;

    /**
     * @brief The number of candidate tempos from lowestTempo to
     * highestTempo.
     */
    size_t numCandidates;

    /**
     * @brief The difference between consecutive candidate tempos, in
     * beats per minute.
     */
    float candidateStep;

    /**
     * @brief The ring buffer of envelope values.
     *
     * Contains windowLength values.
     */
    float_array_ptr envelope;

    /**
     * @brief The prior-weighted score of each candidate tempo.
     *
     * Contains numCandidates values.
     */
    float_array_ptr scores;

    /**
     * @brief The position in envelope where the next value is written,
     * wrapping at windowLength.
     */
    size_t writeIdx;

    /**
     * @brief The number of envelope values received.
     */
    size_t valueCount;

    /**
     * @brief The current tempo estimate, in beats per minute.
     *
     * Zero until the first estimate.
     */
    float tempo;

    /**
     * @brief The number of consecutive estimations in which the
     * estimate stayed within 8 percent of the previous estimate.
     *
     * The estimator does not change it while it holds its estimate
     * through silence.
     */
    size_t stableCount;
};

/**
 * @brief Calculates the number of envelope values in a tempo
 * estimator's or beat tracker's window, which contains four beats at
 * the slowest tempo.
 *
 * @param envelopeRate the number of envelope values per second
 * @param lowestTempo the slowest tempo, in beats per minute
 * @return the number of values in the window
 */
size_t sig_tempo_calcWindowLength(float envelopeRate, float lowestTempo);

/**
 * @brief Initialises a tempo estimator, clearing its window and state.
 *
 * The estimator considers every tempo from lowestTempo to highestTempo
 * until sig_tempo_TempoEstimator_setTempoRange restricts the range.
 *
 * @param self the estimator to initialise
 * @param envelopeRate the number of envelope values per second
 * @param lowestTempo the slowest tempo to support, in beats per minute
 * @param highestTempo the fastest tempo to support, in beats per minute
 * @param envelope an array of windowLength values, which is cleared,
 * where windowLength is the value that sig_tempo_calcWindowLength
 * returns
 * @param scores an array of (highestTempo - lowestTempo + 1) values,
 * which is cleared
 */
void sig_tempo_TempoEstimator_init(struct sig_tempo_TempoEstimator* self,
    float envelopeRate, float lowestTempo, float highestTempo,
    float_array_ptr envelope, float_array_ptr scores);

/**
 * @brief Allocates and initialises a tempo estimator.
 *
 * @param allocator the allocator to use
 * @param envelopeRate the number of envelope values per second
 * @param lowestTempo the slowest tempo to support, in beats per minute
 * @param highestTempo the fastest tempo to support, in beats per minute
 * @return the new estimator
 */
struct sig_tempo_TempoEstimator* sig_tempo_TempoEstimator_new(
    struct sig_Allocator* allocator, float envelopeRate, float lowestTempo,
    float highestTempo);

/**
 * @brief Restricts the tempos that the estimator considers, without
 * allocating.
 *
 * The estimator limits the range to the tempos it supports. When the
 * current estimate lies outside the new range, the estimator replaces
 * it with the best candidate in the range at its next estimation.
 *
 * @param self the estimator
 * @param minTempo the slowest tempo to consider, in beats per minute
 * @param maxTempo the fastest tempo to consider, in beats per minute
 */
inline void sig_tempo_TempoEstimator_setTempoRange(
    struct sig_tempo_TempoEstimator* self, float minTempo, float maxTempo);

/**
 * @brief Processes one envelope value and reports the current tempo
 * estimate.
 *
 * The estimate changes only when the estimator runs an estimation. The
 * window needs about a second of envelope before the first estimate.
 *
 * @param self the estimator
 * @param value the envelope value
 * @return the current tempo estimate in beats per minute, or zero before
 * the first estimate
 */
float sig_tempo_TempoEstimator_generate(struct sig_tempo_TempoEstimator* self,
    float value);

/**
 * @brief Frees a tempo estimator.
 *
 * @param allocator the allocator that created the estimator
 * @param self the estimator to free
 */
void sig_tempo_TempoEstimator_destroy(struct sig_Allocator* allocator,
    struct sig_tempo_TempoEstimator* self);

/**
 * @brief The number of consecutive outliers after which a beat
 * tracker's follower re-anchors its grid.
 */
#define sig_tempo_BeatTracker_RELOCK_OUTLIERS 3

/**
 * @brief The number of consecutive outliers after which a beat
 * tracker's follower re-anchors its grid while it holds a tapped phase.
 */
#define sig_tempo_BeatTracker_HELD_RELOCK_OUTLIERS 8

/**
 * @brief The number of tap intervals over which a beat tracker measures
 * a tapped tempo.
 */
#define sig_tempo_BeatTracker_TAP_INTERVALS 4

/**
 * @brief The gain with which a beat tracker's output closes its gap to
 * the grid, per beat.
 */
#define sig_tempo_BeatTracker_CHASE_GAIN 2.0

/**
 * @brief The slowest rate at which a beat tracker's output advances,
 * relative to the grid's tempo.
 */
#define sig_tempo_BeatTracker_MIN_RATE 0.5

/**
 * @brief The fastest rate at which a beat tracker's output advances,
 * relative to the grid's tempo.
 */
#define sig_tempo_BeatTracker_MAX_RATE 1.5

/**
 * @brief The states of a beat tracker's follower.
 */
enum sig_tempo_BeatTracker_State {
    /**
     * @brief The follower has no tempo, because no tempo estimate has
     * arrived.
     */
    sig_tempo_BeatTracker_State_WAITING = 0,

    /**
     * @brief The follower has a tempo, but has not anchored its grid to
     * a beat.
     */
    sig_tempo_BeatTracker_State_SEARCHING,

    /**
     * @brief Strong beats are arriving, and the follower corrects its
     * grid with them.
     */
    sig_tempo_BeatTracker_State_TRACKING,

    /**
     * @brief No strong beat has arrived for the silence timeout. The
     * grid keeps running.
     */
    sig_tempo_BeatTracker_State_SILENT
};

/**
 * @brief A beat tracker that finds beats in an onset strength envelope,
 * and follows them with a continuous beat phase.
 *
 * The tracker keeps an 8-second window of the envelope. Every 4
 * envelope values, it finds the window's beats with Ellis's (2007)
 * dynamic program. Beats more than half a beat interval behind the
 * newest envelope value commit to the follower, oldest first. The
 * tracker refines each committed beat's time to a fraction of a hop,
 * and subtracts the onset detector's latency.
 *
 * The follower predicts beats from a grid, with a beat interval and
 * the sample time of its most recent beat. Each strong committed beat
 * within a quarter of a beat of the grid corrects the grid's phase,
 * and the interval since the previous accepted beat corrects the
 * grid's beat interval. After three consecutive outliers, the follower
 * re-anchors the grid. The follower's output is a beat position that
 * advances on every sample, chases the grid without jumping, never
 * moves backward, and never runs faster than one and a half times the
 * tempo.
 *
 * A tap anchors the grid at the current sample, and taps less than a
 * beat at the slowest allowed tempo apart set the beat interval. After
 * a tap, until the follower becomes silent, the follower holds the
 * tapped phase and tempo: the dynamic program favours the follower's
 * predicted beats, a relock needs eight consecutive outliers, and the
 * follower does not reseed from the tempo estimate.
 *
 * The tracker assumes that the caller passes envelope value i to
 * sig_tempo_BeatTracker_analyse after advancing the follower
 * (i + 1) * hopLength samples.
 */
struct sig_tempo_BeatTracker {
    /**
     * @brief The sample rate in Hz.
     */
    float sampleRate;

    /**
     * @brief The number of audio samples per envelope value.
     */
    size_t hopLength;

    /**
     * @brief The number of envelope values per second.
     */
    float envelopeRate;

    /**
     * @brief The onset detector's latency, which the tracker subtracts
     * from each beat's time.
     *
     * In samples.
     */
    float latency;

    /**
     * @brief The slowest tempo that the tracker supports, in beats per
     * minute.
     *
     * The window contains four beats at this tempo.
     */
    float lowestTempo;

    /**
     * @brief The fastest tempo that the tracker supports, in beats per
     * minute.
     */
    float highestTempo;

    /**
     * @brief The slowest tempo that the tracker currently allows, in
     * beats per minute.
     */
    float minTempo;

    /**
     * @brief The fastest tempo that the tracker currently allows, in
     * beats per minute.
     */
    float maxTempo;

    /**
     * @brief How strongly the follower corrects its grid, from 0 to 1.
     */
    float responsiveness;

    /**
     * @brief The number of beats without a strong beat after which the
     * follower becomes silent.
     */
    float silenceTimeout;

    /**
     * @brief The number of envelope values that the window contains.
     */
    size_t windowLength;

    /**
     * @brief The ring buffer of envelope values.
     *
     * Contains windowLength values.
     */
    float_array_ptr envelope;

    /**
     * @brief The dynamic program's cumulative score at each window
     * position.
     *
     * Contains windowLength values.
     */
    float_array_ptr scores;

    /**
     * @brief The distance to the best predecessor of each window
     * position, in envelope values.
     *
     * Zero marks a position with no predecessor. Contains windowLength
     * values.
     */
    float_array_ptr predecessors;

    /**
     * @brief The dynamic program's penalty for each predecessor
     * distance.
     *
     * Contains windowLength values.
     */
    float_array_ptr penalties;

    /**
     * @brief The position in envelope where the next value is written,
     * wrapping at windowLength.
     */
    size_t writeIdx;

    /**
     * @brief The number of envelope values received.
     */
    size_t valueCount;

    /**
     * @brief The index of the most recently committed beat's envelope
     * value, counted from the first value received.
     */
    size_t lastCommittedIndex;

    /**
     * @brief Whether any beat has committed.
     */
    bool hasCommitted;

    /**
     * @brief The envelope value against which the tracker measures the
     * strength of committed beats.
     */
    float strengthReference;

    /**
     * @brief The follower's state.
     */
    enum sig_tempo_BeatTracker_State state;

    /**
     * @brief The grid's beat interval, in samples.
     */
    double period;

    /**
     * @brief The sample time of the grid's most recent beat.
     */
    double anchor;

    /**
     * @brief The number of samples that the follower has advanced.
     */
    double sampleClock;

    /**
     * @brief The follower's output beat position, in beats.
     *
     * The phase is its fractional part.
     */
    double position;

    /**
     * @brief The sample time of the most recently accepted beat.
     */
    double lastAcceptedTime;

    /**
     * @brief Whether lastAcceptedTime contains a beat that can measure
     * the beat interval.
     */
    bool hasLastAccepted;

    /**
     * @brief The sample time of the most recent strong beat.
     */
    double lastStrongBeatTime;

    /**
     * @brief The number of consecutive outliers.
     */
    size_t numOutliers;

    /**
     * @brief The sample times of the consecutive outliers.
     */
    double outlierTimes[sig_tempo_BeatTracker_HELD_RELOCK_OUTLIERS];

    /**
     * @brief Whether the follower holds a tapped phase and tempo.
     *
     * A tap sets it, and silence clears it.
     */
    bool isHoldingTaps;

    /**
     * @brief The sample times of the most recent taps, oldest first.
     */
    double tapTimes[sig_tempo_BeatTracker_TAP_INTERVALS + 1];

    /**
     * @brief The number of taps in tapTimes.
     */
    size_t numTaps;

    /**
     * @brief The follower's confidence, from 0 to 1.
     */
    float confidence;
};

/**
 * @brief Initialises a beat tracker, clearing its buffers and state.
 *
 * The tracker allows every tempo from lowestTempo to highestTempo,
 * with a responsiveness of 0.6 and a silence timeout of four beats,
 * until its setter functions change them.
 *
 * @param self the tracker to initialise
 * @param sampleRate the sample rate in Hz
 * @param hopLength the number of audio samples per envelope value
 * @param latency the onset detector's latency, in samples
 * @param lowestTempo the slowest tempo to support, in beats per minute
 * @param highestTempo the fastest tempo to support, in beats per minute
 * @param envelope an array of windowLength values, which is cleared,
 * where windowLength is the value that sig_tempo_calcWindowLength
 * returns
 * @param scores an array of windowLength values, which is cleared
 * @param predecessors an array of windowLength values, which is cleared
 * @param penalties an array of windowLength values, which is cleared
 */
void sig_tempo_BeatTracker_init(struct sig_tempo_BeatTracker* self,
    float sampleRate, size_t hopLength, float latency, float lowestTempo,
    float highestTempo, float_array_ptr envelope, float_array_ptr scores,
    float_array_ptr predecessors, float_array_ptr penalties);

/**
 * @brief Allocates and initialises a beat tracker.
 *
 * @param allocator the allocator to use
 * @param sampleRate the sample rate in Hz
 * @param hopLength the number of audio samples per envelope value
 * @param latency the onset detector's latency, in samples
 * @param lowestTempo the slowest tempo to support, in beats per minute
 * @param highestTempo the fastest tempo to support, in beats per minute
 * @return the new tracker
 */
struct sig_tempo_BeatTracker* sig_tempo_BeatTracker_new(
    struct sig_Allocator* allocator, float sampleRate, size_t hopLength,
    float latency, float lowestTempo, float highestTempo);

/**
 * @brief Processes one envelope value.
 *
 * Every 4 values, the tracker runs the dynamic program and commits
 * beats to the follower. The first tempo estimate starts the follower.
 * When the estimate's beat interval differs from the grid's by more
 * than a factor of 1.33, the follower re-anchors its grid at the
 * estimate's tempo and searches again.
 *
 * @param self the tracker
 * @param value the onset strength value
 * @param tempo the current tempo estimate in beats per minute, or zero
 * when no estimate exists, in which case a follower started by taps
 * uses its own tempo
 */
void sig_tempo_BeatTracker_analyse(struct sig_tempo_BeatTracker* self,
    float value, float tempo);

/**
 * @brief Taps a beat at the current sample.
 *
 * The follower anchors its grid at the current sample, without
 * subtracting the detector's latency, raises its confidence to at least
 * 0.5, and tracks. When the previous tap lies within a beat at the
 * slowest allowed tempo, the follower sets its beat interval to the
 * mean of the intervals between its last four taps, limited to the
 * tempo range. Before the follower has a tempo, the first tap only
 * starts an interval, and the second starts the follower.
 *
 * @param self the tracker
 */
void sig_tempo_BeatTracker_tap(struct sig_tempo_BeatTracker* self);

/**
 * @brief Advances the follower by one audio sample and reports the
 * beat phase.
 *
 * @param self the tracker
 * @return the beat phase, from 0 to 1, which wraps at each beat and
 * otherwise never falls; zero before the first tempo estimate
 */
inline float sig_tempo_BeatTracker_follow(struct sig_tempo_BeatTracker* self);

/**
 * @brief Reports the follower's tempo.
 *
 * @param self the tracker
 * @return the grid's tempo in beats per minute, or zero before the
 * first tempo estimate
 */
inline float sig_tempo_BeatTracker_tempo(struct sig_tempo_BeatTracker* self);

/**
 * @brief Reports the follower's confidence.
 *
 * @param self the tracker
 * @return the confidence, from 0 to 1, which is zero unless the
 * follower is tracking
 */
inline float sig_tempo_BeatTracker_confidence(
    struct sig_tempo_BeatTracker* self);

/**
 * @brief Reports the follower's state.
 *
 * @param self the tracker
 * @return the follower's state
 */
inline enum sig_tempo_BeatTracker_State sig_tempo_BeatTracker_state(
    struct sig_tempo_BeatTracker* self);

/**
 * @brief Sets how strongly the follower corrects its grid.
 *
 * The phase gain is 0.2 + 0.7 * responsiveness, and the beat interval
 * gain is 0.1 + 0.7 * responsiveness.
 *
 * @param self the tracker
 * @param responsiveness the responsiveness, from 0 to 1
 */
inline void sig_tempo_BeatTracker_setResponsiveness(
    struct sig_tempo_BeatTracker* self, float responsiveness);

/**
 * @brief Restricts the tempos that the tracker allows, without
 * allocating.
 *
 * The tracker limits the range to the tempos it supports, and limits
 * the dynamic program's tempo and the grid's tempo to the range.
 *
 * @param self the tracker
 * @param minTempo the slowest tempo to allow, in beats per minute
 * @param maxTempo the fastest tempo to allow, in beats per minute
 */
inline void sig_tempo_BeatTracker_setTempoRange(
    struct sig_tempo_BeatTracker* self, float minTempo, float maxTempo);

/**
 * @brief Sets how many beats without a strong beat make the follower
 * silent.
 *
 * @param self the tracker
 * @param silenceTimeout the timeout in beats, from 1 to 16
 */
inline void sig_tempo_BeatTracker_setSilenceTimeout(
    struct sig_tempo_BeatTracker* self, float silenceTimeout);

/**
 * @brief Frees a beat tracker.
 *
 * @param allocator the allocator that created the tracker
 * @param self the tracker to free
 */
void sig_tempo_BeatTracker_destroy(struct sig_Allocator* allocator,
    struct sig_tempo_BeatTracker* self);

/**
 * @brief The start of the shifted phase range that arms a clock
 * generator's next trigger.
 */
#define sig_tempo_ClockGenerator_ARM_START 0.25f

/**
 * @brief The end of the shifted phase range that arms a clock
 * generator's next trigger.
 */
#define sig_tempo_ClockGenerator_ARM_END 0.75f

/**
 * @brief The outputs of a clock generator for one sample.
 */
struct sig_tempo_ClockGenerator_Outputs {
    /**
     * @brief The clock gate, 1 during a trigger and 0 otherwise.
     */
    float clock;

    /**
     * @brief The beat phase, shifted earlier by the lead, from 0 to 1.
     *
     * Zero while the clock is muted.
     */
    float phase;

    /**
     * @brief The follower's tempo, in beats per minute.
     *
     * Zero before the first tempo estimate.
     */
    float bpm;

    /**
     * @brief The follower's confidence, from 0 to 1.
     */
    float confidence;
};

/**
 * @brief A clock generator that converts a beat tracker's phase into
 * clock triggers.
 *
 * The generator shifts the phase earlier by the lead, converted to
 * beats at the follower's tempo, and fires a trigger when the shifted
 * phase wraps, that is, when it falls by more than half a cycle
 * between samples. After a trigger, the generator fires again only
 * after the shifted phase has passed through the middle of the beat,
 * from 0.25 to 0.75, so a tempo change that steps the shifted phase
 * back across the wrap cannot fire a second trigger for one beat.
 *
 * The clock is active while the follower is tracking, and while it is
 * silent if the silence behaviour is free-run. While the clock is
 * muted, the clock gate stays low and the phase is zero.
 */
struct sig_tempo_ClockGenerator {
    /**
     * @brief The sample rate in Hz.
     */
    float sampleRate;

    /**
     * @brief How far ahead of each predicted beat the clock fires.
     *
     * In seconds, from 0 to 0.25.
     */
    float lead;

    /**
     * @brief The length of each trigger.
     *
     * In seconds, from 0.001 to 0.1.
     */
    float triggerDuration;

    /**
     * @brief Whether the clock mutes, 0, or free-runs, 1, while the
     * follower is silent.
     */
    float silenceBehaviour;

    /**
     * @brief The length of each trigger, in samples.
     */
    uint32_t triggerSamples;

    /**
     * @brief The number of samples that remain in the current trigger.
     */
    uint32_t triggerSamplesRemaining;

    /**
     * @brief The shifted phase on the previous sample.
     */
    float previousPhase;

    /**
     * @brief Whether the clock was active on the previous sample.
     */
    bool wasActive;

    /**
     * @brief Whether the next wrap of the shifted phase fires a
     * trigger.
     */
    bool isArmed;
};

/**
 * @brief Initialises a clock generator, with no lead, triggers of
 * 10 ms, and a clock that mutes while the follower is silent.
 *
 * @param self the generator to initialise
 * @param sampleRate the sample rate in Hz
 */
void sig_tempo_ClockGenerator_init(struct sig_tempo_ClockGenerator* self,
    float sampleRate);

/**
 * @brief Sets how far ahead of each predicted beat the clock fires.
 *
 * @param self the generator
 * @param lead the lead in seconds, from 0 to 0.25
 */
inline void sig_tempo_ClockGenerator_setLead(
    struct sig_tempo_ClockGenerator* self, float lead);

/**
 * @brief Sets the length of each trigger.
 *
 * @param self the generator
 * @param triggerDuration the trigger duration in seconds, from 0.001
 * to 0.1
 */
inline void sig_tempo_ClockGenerator_setTriggerDuration(
    struct sig_tempo_ClockGenerator* self, float triggerDuration);

/**
 * @brief Sets whether the clock mutes or free-runs while the follower
 * is silent.
 *
 * @param self the generator
 * @param silenceBehaviour 0 to mute, or 1 to free-run
 */
inline void sig_tempo_ClockGenerator_setSilenceBehaviour(
    struct sig_tempo_ClockGenerator* self, float silenceBehaviour);

/**
 * @brief Generates one sample of the clock generator's outputs.
 *
 * @param self the generator
 * @param phase the beat tracker's phase, from 0 to 1
 * @param tempo the beat tracker's tempo, in beats per minute
 * @param confidence the beat tracker's confidence, from 0 to 1
 * @param state the beat tracker's state
 * @param outputs the outputs to write
 */
inline void sig_tempo_ClockGenerator_generate(
    struct sig_tempo_ClockGenerator* self, float phase, float tempo,
    float confidence, enum sig_tempo_BeatTracker_State state,
    struct sig_tempo_ClockGenerator_Outputs* outputs);

// TODO: Should the signal argument at least be defined
// as a struct sig_dsp_Signal*, rather than void*?
// Either way, this is cast by the implementation to whatever
// concrete Signal type is appropriate.
typedef void (*sig_dsp_generateFn)(void* signal);

struct sig_dsp_Signal {
    struct sig_AudioSettings* audioSettings;
    sig_dsp_generateFn generate;
};

// TODO: Signal initializer will need to take some additional information:
//  - type information about its inputs
//  - a pointer to its input container
//  - type information about its outputs
//  - a pointer to its output container
// This will allow for the implementation of a generic connector API,
// which will connect different signals together appropriately. Maybe?
void sig_dsp_Signal_init(void* signal, struct sig_SignalContext* context,
    sig_dsp_generateFn generate);
void sig_dsp_Signal_generate(void* signal);
void sig_dsp_Signal_destroy(struct sig_Allocator* allocator,
    void* signal);

#define sig_CONNECT_TO_SILENCE(signal, inputName, context)\
    signal->inputs.inputName = context->silence->outputs.main;

#define sig_CONNECT_TO_UNITY(signal, inputName, context)\
    signal->inputs.inputName = context->unity->outputs.main;

struct sig_dsp_Signal_SingleSourceInput {
    float_array_ptr source;
};

struct sig_dsp_Signal_SingleMonoOutput {
    float_array_ptr main;
};

void sig_dsp_Signal_SingleMonoOutput_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_Signal_SingleMonoOutput* outputs);
void sig_dsp_Signal_SingleMonoOutput_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_Signal_SingleMonoOutput* outputs);


void sig_dsp_evaluateSignals(struct sig_List* signalList);

struct sig_dsp_SignalEvaluator;

typedef void (*sig_dsp_SignalEvaluator_evaluate)(
    struct sig_dsp_SignalEvaluator* self);

struct sig_dsp_SignalEvaluator {
    sig_dsp_SignalEvaluator_evaluate evaluate;
};

struct sig_dsp_SignalListEvaluator {
    sig_dsp_SignalEvaluator_evaluate evaluate;
    struct sig_List* signalList;
};

struct sig_dsp_SignalListEvaluator* sig_dsp_SignalListEvaluator_new(
    struct sig_Allocator* allocator, struct sig_List* signalList);

void sig_dsp_SignalListEvaluator_init(
    struct sig_dsp_SignalListEvaluator* self, struct sig_List* signalList);

void sig_dsp_SignalListEvaluator_evaluate(
    struct sig_dsp_SignalEvaluator* self);

void sig_dsp_SignalListEvaluator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SignalListEvaluator* self);


struct sig_dsp_Value_Parameters {
    float value;
};

/**
 * @brief A signal that outputs a value.
 * This is intended to be used for values that may change periodically as a
 * result of user input.
 */
struct sig_dsp_Value {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Value_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    float lastSample;
};

struct sig_dsp_Value* sig_dsp_Value_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context);
void sig_dsp_Value_init(struct sig_dsp_Value* self,
    struct sig_SignalContext* context);
void sig_dsp_Value_generate(void* signal);
void sig_dsp_Value_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Value* self);

// TODO: Could this be replaced with a sig_dsp_Value that is
// only generated once, at instantiation time?
/**
 * @brief A signal whose value is constant throughout its lifetime.
 */
struct sig_dsp_ConstantValue {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_ConstantValue* sig_dsp_ConstantValue_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context,
    float value);
void sig_dsp_ConstantValue_init(struct sig_dsp_ConstantValue* self,
    struct sig_SignalContext* context, float value);
void sig_dsp_ConstantValue_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ConstantValue* self);


struct sig_dsp_Abs {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Signal_SingleSourceInput inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_Abs* sig_dsp_Abs_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Abs_init(struct sig_dsp_Abs* self,
    struct sig_SignalContext* context);
void sig_dsp_Abs_generate(void* signal);
void sig_dsp_Abs_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Abs* self);


struct sig_dsp_Clamp_Parameters {
    float min;
    float max;
};

struct sig_dsp_Clamp {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Signal_SingleSourceInput inputs;
    struct sig_dsp_Clamp_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_Clamp* sig_dsp_Clamp_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Clamp_init(struct sig_dsp_Clamp* self,
    struct sig_SignalContext* context);
void sig_dsp_Clamp_generate(void* signal);
void sig_dsp_Clamp_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Clamp* self);

/**
 * @brief The parameters of a SnapToInteger node.
 */
struct sig_dsp_SnapToInteger_Parameters {
    /**
     * @brief The largest distance from a whole number at which a sample
     * snaps to it.
     *
     * The default of 0.0 snaps only samples that are already whole numbers.
     */
    float tolerance;
};

/**
 * @brief Rounds each sample to the nearest whole number if it is within a
 * tolerance of it, and passes the other samples through unchanged.
 *
 * See sig_snapToInteger.
 *
 * Inputs:
 *  - source: the signal to snap
 */
struct sig_dsp_SnapToInteger {
    /**
     * @brief The signal's base state.
     *
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The node's input.
     *
     */
    struct sig_dsp_Signal_SingleSourceInput inputs;

    /**
     * @brief The node's parameters.
     *
     */
    struct sig_dsp_SnapToInteger_Parameters parameters;

    /**
     * @brief The node's output, the snapped signal.
     *
     */
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

/**
 * @brief Creates a new SnapToInteger node.
 *
 * @param allocator the allocator to use
 * @param context the signal context
 * @return a new SnapToInteger node
 */
struct sig_dsp_SnapToInteger* sig_dsp_SnapToInteger_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);

/**
 * @brief Initialises a SnapToInteger node.
 *
 * @param self the node to initialise
 * @param context the signal context
 */
void sig_dsp_SnapToInteger_init(struct sig_dsp_SnapToInteger* self,
    struct sig_SignalContext* context);

/**
 * @brief Generates a block of samples.
 *
 * @param signal the signal to generate, cast to a
 * struct sig_dsp_SnapToInteger*
 */
void sig_dsp_SnapToInteger_generate(void* signal);

/**
 * @brief Frees a SnapToInteger node.
 *
 * @param allocator the allocator that created the node
 * @param self the node to free
 */
void sig_dsp_SnapToInteger_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SnapToInteger* self);


struct sig_dsp_ScaleOffset_Parameters {
    float scale;
    float offset;
};

struct sig_dsp_ScaleOffset {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Signal_SingleSourceInput inputs;
    struct sig_dsp_ScaleOffset_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_ScaleOffset* sig_dsp_ScaleOffset_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_ScaleOffset_init(struct sig_dsp_ScaleOffset* self,
    struct sig_SignalContext* context);
void sig_dsp_ScaleOffset_generate(void* signal);
void sig_dsp_ScaleOffset_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ScaleOffset* self);


struct sig_dsp_Sine {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Signal_SingleSourceInput inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_Sine* sig_dsp_Sine_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Sine_init(struct sig_dsp_Sine* self,
    struct sig_SignalContext* context);
void sig_dsp_Sine_generate(void* signal);
void sig_dsp_Sine_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Sine* self);


struct sig_dsp_BinaryOp_Inputs {
    float_array_ptr left;
    float_array_ptr right;
};

struct sig_dsp_BinaryOp {
    struct sig_dsp_Signal signal;
    struct sig_dsp_BinaryOp_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_BinaryOp* sig_dsp_BinaryOp_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context, sig_dsp_generateFn generate);
void sig_dsp_BinaryOp_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context, sig_dsp_generateFn generate);

struct sig_dsp_BinaryOp* sig_dsp_Add_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Add_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context);
void sig_dsp_Add_generate(void* signal);
void sig_dsp_Add_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_BinaryOp* self);

struct sig_dsp_BinaryOp* sig_dsp_Sub_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Sub_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context);
void sig_dsp_Sub_generate(void* signal);
void sig_dsp_Sub_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_BinaryOp* self);

struct sig_dsp_BinaryOp* sig_dsp_Mul_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Mul_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context);
void sig_dsp_Mul_generate(void* signal);
void sig_dsp_Mul_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_BinaryOp* self);

struct sig_dsp_BinaryOp* sig_dsp_Div_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Div_init(struct sig_dsp_BinaryOp* self,
    struct sig_SignalContext* context);
void sig_dsp_Div_generate(void* signal);
void sig_dsp_Div_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_BinaryOp* self);


struct sig_dsp_Invert_Inputs {
    float_array_ptr source;
};

struct sig_dsp_Invert {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Invert_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_Invert* sig_dsp_Invert_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Invert_init(struct sig_dsp_Invert* self,
    struct sig_SignalContext* context);
void sig_dsp_Invert_generate(void* signal);
void sig_dsp_Invert_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Invert* self);


/**
 * The inputs for an Accumulator Signal.
 */
struct sig_dsp_Accumulate_Inputs {
    /**
     * The source input.
     * Only the first value of each block will be read.
     */
    float_array_ptr source;

    /**
     * When a positive trigger is received from this input,
     * the accumulator will be reset to zero.
     */
    float_array_ptr reset;
};

struct sig_dsp_Accumulate_Parameters {
    float accumulatorStart;
    float wrap;
    float maxValue;
};

/**
 * A Signal that accumulates its "source" input.
 *
 * Note: while Signaletic doesn't have a formal
 * concept of a control rate, this Signal
 * currently only reads the first sample from each block
 * of its source input.
 */
struct sig_dsp_Accumulate {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Accumulate_Inputs inputs;
    struct sig_dsp_Accumulate_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    float accumulator;
    float previousReset;
};

struct sig_dsp_Accumulate* sig_dsp_Accumulate_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Accumulate_init(struct sig_dsp_Accumulate* self,
    struct sig_SignalContext* context);
void sig_dsp_Accumulate_generate(void* signal);
void sig_dsp_Accumulate_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Accumulate* self);


/**
 * Inputs for a GatedTimer.
 */
struct sig_dsp_GatedTimer_Inputs {
    /**
     * A gate signal that, when open, causes the timer to run.
     */
    float_array_ptr gate;

    /**
     * The duration, in seconds, that the timer should run for.
     */
    float_array_ptr duration;

    /**
     * A boolean signal specifying whether the time should run again
     * after it has finished. Values >0.0 denote true.
     */
    float_array_ptr loop;
};

/**
 * GatedTimer is a Signal that fires a trigger after
 * a specified duration has elapsed.
 *
 * The timer will only run while the gate is open,
 * and will reset whenever the gate closes.
 */
struct sig_dsp_GatedTimer {
    struct sig_dsp_Signal signal;
    struct sig_dsp_GatedTimer_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    unsigned long timer;
    bool hasFired;
    float prevGate;
};

struct sig_dsp_GatedTimer* sig_dsp_GatedTimer_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_GatedTimer_init(struct sig_dsp_GatedTimer* self,
    struct sig_SignalContext* context);
void sig_dsp_GatedTimer_generate(void* signal);
void sig_dsp_GatedTimer_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_GatedTimer* self);


/**
 * The inputs for a TimedTriggerCounter Signal.
 */
struct sig_dsp_TimedTriggerCounter_Inputs {
    /**
     * The source input, from which incoming triggers will be counted.
     */
    float_array_ptr source;

    /**
     * The time (in seconds) before which the triggers
     * should have fired.
     */
    float_array_ptr duration;

    /**
     * The number of triggers to count within the specified duration
     * before firing an output trigger.
     * Decimal points will be truncated and ignored.
     */
    float_array_ptr count;
};

/**
 * A signal that counts incoming triggers.
 *
 * This signal will fire trigger if the expected number
 * of triggers are received within the specified duration.
 */
struct sig_dsp_TimedTriggerCounter {
    struct sig_dsp_Signal signal;
    struct sig_dsp_TimedTriggerCounter_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    int numTriggers;
    long timer;
    bool isTimerActive;
    float previousSource;
};

struct sig_dsp_TimedTriggerCounter* sig_dsp_TimedTriggerCounter_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_TimedTriggerCounter_init(
    struct sig_dsp_TimedTriggerCounter* self,
    struct sig_SignalContext* context);
void sig_dsp_TimedTriggerCounter_generate(void* signal);
void sig_dsp_TimedTriggerCounter_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_TimedTriggerCounter* self);


/**
 * The inputs for a ToggleGate Signal.
 */
struct sig_dsp_ToggleGate_Inputs {

    /**
     * The incoming trigger signal used to
     * toggle the gate on and off.
     *
     * Any transition from a zero or negative value
     * to a positive value will be interpreted as a trigger.
     */
    float_array_ptr trigger;
};

/**
 * A ToggleGate Signal outputs a positive gate
 * when it receives a trigger to open the gate,
 * and will close it when another trigger is received.
 */
struct sig_dsp_ToggleGate {
    struct sig_dsp_Signal signal;
    struct sig_dsp_ToggleGate_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    bool isGateOpen;
    float prevTrig;
};

struct sig_dsp_ToggleGate* sig_dsp_ToggleGate_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_ToggleGate_init(struct sig_dsp_ToggleGate* self,
    struct sig_SignalContext* context);
void sig_dsp_ToggleGate_generate(void* signal);
void sig_dsp_ToggleGate_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_ToggleGate* self);

// TODO: Rename mul and add to scale and offset.
struct sig_dsp_Oscillator_Inputs {
    float_array_ptr freq;
    float_array_ptr phaseOffset;
    float_array_ptr mul;
    float_array_ptr add;
};

struct sig_dsp_Oscillator_Outputs {
    float_array_ptr main;
    float_array_ptr eoc;
};

void sig_dsp_Oscillator_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_Oscillator_Outputs* outputs);
void sig_dsp_Oscillator_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator_Outputs* outputs);

struct sig_dsp_Oscillator {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Oscillator_Inputs inputs;
    struct sig_dsp_Oscillator_Outputs outputs;
    struct sig_osc_Oscillator state;
};

struct sig_dsp_Oscillator* sig_dsp_Oscillator_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context,
    sig_dsp_generateFn generate);
void sig_dsp_Oscillator_init(struct sig_dsp_Oscillator* self,
    struct sig_SignalContext* context, sig_dsp_generateFn generate);
float sig_dsp_Oscillator_eoc(float phase);
float sig_dsp_Oscillator_wrapPhase(float phase);
void sig_dsp_Oscillator_accumulatePhase(struct sig_dsp_Oscillator* self,
    size_t i);
void sig_dsp_Oscillator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator* self);


void sig_dsp_SineOscillator_init(struct sig_dsp_Oscillator* self,
    struct sig_SignalContext* context);
struct sig_dsp_Oscillator* sig_dsp_SineOscillator_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context);
void sig_dsp_SineOscillator_generate(void* signal);
void sig_dsp_SineOscillator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator* self);

struct sig_dsp_FastLFSineOscillator_Inputs {
    float_array_ptr frequency;
    float_array_ptr mul;
    float_array_ptr add;
};

struct sig_dsp_FastLFSineOscillator {
    struct sig_dsp_Signal signal;
    struct sig_dsp_FastLFSineOscillator_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    struct sig_osc_FastLFSine state;
};

void sig_dsp_FastLFSineOscillator_init(struct sig_dsp_FastLFSineOscillator* self,
    struct sig_SignalContext* context);
struct sig_dsp_FastLFSineOscillator* sig_dsp_FastLFSineOscillator_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_FastLFSineOscillator_generate(void* signal);
void sig_dsp_FastLFSineOscillator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_FastLFSineOscillator* self);


void sig_dsp_LFTriangle_init(struct sig_dsp_Oscillator* self,
    struct sig_SignalContext* context);
struct sig_dsp_Oscillator* sig_dsp_LFTriangle_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_LFTriangle_generate(void* signal);
void sig_dsp_LFTriangle_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Oscillator* self);


struct sig_dsp_WavetableOscillator {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Oscillator_Inputs inputs;
    struct sig_dsp_Oscillator_Outputs outputs;
    struct sig_osc_Wavetable state;

    struct sig_Buffer* wavetable;
};

void sig_dsp_WavetableOscillator_init(struct sig_dsp_WavetableOscillator* self,
    struct sig_SignalContext* context);
struct sig_dsp_WavetableOscillator* sig_dsp_WavetableOscillator_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_WavetableOscillator_generate(void* signal);
void sig_dsp_WavetableOscillator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_WavetableOscillator* self);

struct sig_dsp_WavetableBankOscillator_Inputs {
    float_array_ptr freq;
    float_array_ptr phaseOffset;
    float_array_ptr mul;
    float_array_ptr add;
    float_array_ptr tableIndex;
};

struct sig_dsp_WavetableBankOscillator {
    struct sig_dsp_Signal signal;
    struct sig_dsp_WavetableBankOscillator_Inputs inputs;
    struct sig_dsp_Oscillator_Outputs outputs;
    struct sig_osc_WavetableBank state;
    struct sig_WavetableBank* wavetables;
};

void sig_dsp_WavetableBankOscillator_init(struct sig_dsp_WavetableBankOscillator* self,
    struct sig_SignalContext* context);
struct sig_dsp_WavetableBankOscillator* sig_dsp_WavetableBankOscillator_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_WavetableBankOscillator_generate(void* signal);
void sig_dsp_WavetableBankOscillator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_WavetableBankOscillator* self);

/**
 * @brief An oscillator that crossfades between band-limited waveforms,
 * reading from the tables for the octave of its frequency.
 *
 * Inputs:
 *  - freq: the frequency, in Hz
 *  - phaseOffset: the phase offset, in cycles
 *  - mul: the gain applied to the output
 *  - add: the offset added to the output
 *  - tableIndex: the shape, in cycles of the bank; each of the bank's
 *    shapes occupies an equal share of one cycle, and values wrap around
 */
struct sig_dsp_AdditiveWavetableBankOscillator {
    /**
     * @brief The signal's base state.
     *
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The oscillator's inputs.
     *
     */
    struct sig_dsp_WavetableBankOscillator_Inputs inputs;

    /**
     * @brief The oscillator's main and end of cycle outputs.
     *
     */
    struct sig_dsp_Oscillator_Outputs outputs;

    /**
     * @brief The oscillator's state.
     *
     */
    struct sig_osc_AdditiveWavetableBank state;

    /**
     * @brief The additive wavetable bank to read from.
     *
     * Must be assigned before the oscillator is first generated.
     */
    struct sig_AdditiveWavetableBank* wavetables;
};

void sig_dsp_AdditiveWavetableBankOscillator_init(
    struct sig_dsp_AdditiveWavetableBankOscillator* self,
    struct sig_SignalContext* context);
struct sig_dsp_AdditiveWavetableBankOscillator*
    sig_dsp_AdditiveWavetableBankOscillator_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_AdditiveWavetableBankOscillator_generate(void* signal);
void sig_dsp_AdditiveWavetableBankOscillator_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_AdditiveWavetableBankOscillator* self);

/**
 * @brief The inputs of a phasor.
 */
struct sig_dsp_Phasor_Inputs {
    /**
     * @brief The phasor's frequency, in Hz.
     *
     */
    float_array_ptr frequency;
};

/**
 * @brief A phasor, which outputs a phase that ramps from 0.0 to 1.0 at its
 * frequency.
 *
 * Inputs:
 *  - frequency: the frequency, in Hz
 */
struct sig_dsp_Phasor {
    /**
     * @brief The signal's base state.
     *
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The phasor's inputs.
     *
     */
    struct sig_dsp_Phasor_Inputs inputs;

    /**
     * @brief The phasor's output, its phase normalized to 0.0-1.0.
     *
     */
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    /**
     * @brief The phasor's state.
     *
     */
    struct sig_osc_Oscillator state;
};

/**
 * @brief Initialises a phasor node.
 *
 * @param self the node to initialise
 * @param context the signal context
 */
void sig_dsp_Phasor_init(struct sig_dsp_Phasor* self,
    struct sig_SignalContext* context);

/**
 * @brief Creates a new phasor node.
 *
 * @param allocator the allocator to use
 * @param context the signal context
 * @return a new phasor node
 */
struct sig_dsp_Phasor* sig_dsp_Phasor_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context);

/**
 * @brief Generates a block of samples.
 *
 * @param signal the signal to generate, cast to a struct sig_dsp_Phasor*
 */
void sig_dsp_Phasor_generate(void* signal);

/**
 * @brief Frees a phasor node.
 *
 * @param allocator the allocator that created the node
 * @param self the node to free
 */
void sig_dsp_Phasor_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Phasor* self);

/**
 * @brief The inputs of a ratio phasor.
 */
struct sig_dsp_RatioPhasor_Inputs {
    /**
     * @brief The reference phase, normalized to 0.0-1.0.
     *
     */
    float_array_ptr reference;

    /**
     * @brief The ratio's numerator.
     *
     */
    float_array_ptr numerator;

    /**
     * @brief The ratio's denominator.
     *
     */
    float_array_ptr denominator;

    /**
     * @brief The number of octaves by which to transpose the ratio.
     *
     * Whole numbers of octaves keep the phasor locked to the reference.
     */
    float_array_ptr octaves;
};

/**
 * @brief The parameters of a ratio phasor.
 */
struct sig_dsp_RatioPhasor_Parameters {
    /**
     * @brief The time constant of the phase correction after the ratio
     * changes, in seconds.
     *
     * A value of 0.0 corrects the phase immediately.
     */
    float glide;

    /**
     * @brief The highest frequency at which the phasor can run, in Hz.
     *
     * Defaults to half the sample rate. See
     * sig_osc_RatioPhasor_setMaxFrequency.
     */
    float maxFrequency;
};

/**
 * @brief A phasor that runs at a ratio of a reference phase.
 *
 * See sig_osc_RatioPhasor for how the phasor locks to the reference.
 *
 * Inputs:
 *  - reference: the reference phase, normalized to 0.0-1.0
 *  - numerator: the ratio's numerator
 *  - denominator: the ratio's denominator
 *  - octaves: the number of octaves by which to transpose the ratio
 */
struct sig_dsp_RatioPhasor {
    /**
     * @brief The signal's base state.
     *
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The phasor's inputs.
     *
     */
    struct sig_dsp_RatioPhasor_Inputs inputs;

    /**
     * @brief The phasor's parameters.
     *
     */
    struct sig_dsp_RatioPhasor_Parameters parameters;

    /**
     * @brief The phasor's output, its phase normalized to 0.0-1.0.
     *
     */
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    /**
     * @brief The phasor's state.
     *
     */
    struct sig_osc_RatioPhasor state;

    /**
     * @brief The glide for which the phasor's coefficient was calculated,
     * in seconds.
     *
     */
    float previousGlide;

    /**
     * @brief The maximum frequency that the phasor's state was last set
     * to, in Hz.
     *
     */
    float previousMaxFrequency;
};

/**
 * @brief Initialises a ratio phasor node.
 *
 * @param self the node to initialise
 * @param context the signal context
 */
void sig_dsp_RatioPhasor_init(struct sig_dsp_RatioPhasor* self,
    struct sig_SignalContext* context);

/**
 * @brief Creates a new ratio phasor node.
 *
 * @param allocator the allocator to use
 * @param context the signal context
 * @return a new ratio phasor node
 */
struct sig_dsp_RatioPhasor* sig_dsp_RatioPhasor_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);

/**
 * @brief Generates a block of samples.
 *
 * @param signal the signal to generate, cast to a struct sig_dsp_RatioPhasor*
 */
void sig_dsp_RatioPhasor_generate(void* signal);

/**
 * @brief Frees a ratio phasor node.
 *
 * @param allocator the allocator that created the node
 * @param self the node to free
 */
void sig_dsp_RatioPhasor_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_RatioPhasor* self);

/**
 * @brief The inputs of a wavetable reader.
 */
struct sig_dsp_WavetableReader_Inputs {
    /**
     * @brief The phase at which to read the table, normalized to 0.0-1.0.
     *
     */
    float_array_ptr phase;

    /**
     * @brief The offset added to the phase, in cycles.
     *
     */
    float_array_ptr phaseOffset;

    /**
     * @brief The gain applied to the output.
     *
     */
    float_array_ptr mul;

    /**
     * @brief The offset added to the output.
     *
     */
    float_array_ptr add;
};

/**
 * @brief Reads a wavetable at a phase from its inputs.
 *
 * Unlike sig_dsp_WavetableOscillator, the reader does not accumulate a
 * phase of its own, so it can follow a shared phase such as the output of a
 * sig_dsp_Phasor or sig_dsp_RatioPhasor.
 *
 * Inputs:
 *  - phase: the phase at which to read, normalized to 0.0-1.0
 *  - phaseOffset: the offset added to the phase, in cycles
 *  - mul: the gain applied to the output
 *  - add: the offset added to the output
 */
struct sig_dsp_WavetableReader {
    /**
     * @brief The signal's base state.
     *
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The reader's inputs.
     *
     */
    struct sig_dsp_WavetableReader_Inputs inputs;

    /**
     * @brief The reader's output.
     *
     */
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    /**
     * @brief The wavetable to read.
     *
     * Must be assigned before the reader is first generated.
     */
    struct sig_Buffer* wavetable;
};

/**
 * @brief Initialises a wavetable reader node.
 *
 * @param self the node to initialise
 * @param context the signal context
 */
void sig_dsp_WavetableReader_init(struct sig_dsp_WavetableReader* self,
    struct sig_SignalContext* context);

/**
 * @brief Creates a new wavetable reader node.
 *
 * @param allocator the allocator to use
 * @param context the signal context
 * @return a new wavetable reader node
 */
struct sig_dsp_WavetableReader* sig_dsp_WavetableReader_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);

/**
 * @brief Generates a block of samples.
 *
 * @param signal the signal to generate, cast to a
 * struct sig_dsp_WavetableReader*
 */
void sig_dsp_WavetableReader_generate(void* signal);

/**
 * @brief Frees a wavetable reader node.
 *
 * @param allocator the allocator that created the node
 * @param self the node to free
 */
void sig_dsp_WavetableReader_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_WavetableReader* self);

/**
 * @brief The inputs of a wavetable bank reader.
 */
struct sig_dsp_WavetableBankReader_Inputs {
    /**
     * @brief The phase at which to read the bank, normalized to 0.0-1.0.
     *
     */
    float_array_ptr phase;

    /**
     * @brief The offset added to the phase, in cycles.
     *
     */
    float_array_ptr phaseOffset;

    /**
     * @brief The table to read, in cycles of the bank.
     *
     * See sig_osc_WavetableBank_tableIndex.
     */
    float_array_ptr tableIndex;

    /**
     * @brief The gain applied to the output.
     *
     */
    float_array_ptr mul;

    /**
     * @brief The offset added to the output.
     *
     */
    float_array_ptr add;
};

/**
 * @brief Reads a bank of wavetables at a phase from its inputs.
 *
 * Unlike sig_dsp_WavetableBankOscillator, the reader does not accumulate a
 * phase of its own, so it can follow a shared phase such as the output of a
 * sig_dsp_ClockPhasor or sig_dsp_RatioPhasor.
 *
 * Inputs:
 *  - phase: the phase at which to read, normalized to 0.0-1.0
 *  - phaseOffset: the offset added to the phase, in cycles
 *  - tableIndex: the table to read, in cycles of the bank
 *  - mul: the gain applied to the output
 *  - add: the offset added to the output
 */
struct sig_dsp_WavetableBankReader {
    /**
     * @brief The signal's base state.
     *
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The reader's inputs.
     *
     */
    struct sig_dsp_WavetableBankReader_Inputs inputs;

    /**
     * @brief The reader's output.
     *
     */
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    /**
     * @brief The bank of wavetables to read.
     *
     * Must be assigned before the reader is first generated.
     */
    struct sig_WavetableBank* wavetables;
};

/**
 * @brief Initialises a wavetable bank reader node.
 *
 * @param self the node to initialise
 * @param context the signal context
 */
void sig_dsp_WavetableBankReader_init(struct sig_dsp_WavetableBankReader* self,
    struct sig_SignalContext* context);

/**
 * @brief Creates a new wavetable bank reader node.
 *
 * @param allocator the allocator to use
 * @param context the signal context
 * @return a new wavetable bank reader node
 */
struct sig_dsp_WavetableBankReader* sig_dsp_WavetableBankReader_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);

/**
 * @brief Generates a block of samples.
 *
 * @param signal the signal to generate, cast to a
 * struct sig_dsp_WavetableBankReader*
 */
void sig_dsp_WavetableBankReader_generate(void* signal);

/**
 * @brief Frees a wavetable bank reader node.
 *
 * @param allocator the allocator that created the node
 * @param self the node to free
 */
void sig_dsp_WavetableBankReader_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_WavetableBankReader* self);

/**
 * @brief The inputs of an additive wavetable bank reader.
 */
struct sig_dsp_AdditiveWavetableBankReader_Inputs {
    /**
     * @brief The phase at which to read the bank, normalized to 0.0-1.0.
     *
     */
    float_array_ptr phase;

    /**
     * @brief The offset added to the phase, in cycles.
     *
     */
    float_array_ptr phaseOffset;

    /**
     * @brief The frequency that selects the bank's octave, in Hz.
     *
     */
    float_array_ptr frequency;

    /**
     * @brief The shape to read, in cycles of the bank.
     *
     */
    float_array_ptr tableIndex;

    /**
     * @brief The gain applied to the output.
     *
     */
    float_array_ptr mul;

    /**
     * @brief The offset added to the output.
     *
     */
    float_array_ptr add;
};

/**
 * @brief The parameters of an additive wavetable bank reader.
 */
struct sig_dsp_AdditiveWavetableBankReader_Parameters {
    /**
     * @brief The time with which the peak read rate falls, in seconds.
     *
     * When greater than 0.0, the reader selects its octave from the peak
     * rate at which it reads, and ignores its frequency input. The default
     * of 0.0 selects the octave from the frequency input.
     */
    float peakRelease;
};

/**
 * @brief Reads an additive wavetable bank at a phase from its inputs.
 *
 * Unlike sig_dsp_AdditiveWavetableBankOscillator, the reader does not
 * accumulate a phase of its own, so it can follow a shared phase such as
 * the output of a sig_dsp_Phasor or sig_dsp_RatioPhasor. Its frequency input
 * selects the octave to read, which should be the frequency at which the
 * phase advances.
 *
 * When the phase or the phase offset is modulated, the phase can advance
 * much faster than its frequency, and the octave's harmonics alias. With a
 * peak release time, the reader instead selects the octave from the peak
 * rate at which it actually reads, using a sig_osc_PeakFrequency.
 *
 * Inputs:
 *  - phase: the phase at which to read, normalized to 0.0-1.0
 *  - phaseOffset: the offset added to the phase, in cycles
 *  - frequency: the frequency that selects the octave, in Hz
 *  - tableIndex: the shape, in cycles of the bank
 *  - mul: the gain applied to the output
 *  - add: the offset added to the output
 */
struct sig_dsp_AdditiveWavetableBankReader {
    /**
     * @brief The signal's base state.
     *
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The reader's inputs.
     *
     */
    struct sig_dsp_AdditiveWavetableBankReader_Inputs inputs;

    /**
     * @brief The reader's parameters.
     *
     */
    struct sig_dsp_AdditiveWavetableBankReader_Parameters parameters;

    /**
     * @brief The reader's output.
     *
     */
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    /**
     * @brief The state that selects and caches the octave to read.
     *
     * The reader does not use the state's phase accumulator.
     */
    struct sig_osc_AdditiveWavetableBank state;

    /**
     * @brief The state that measures the peak rate at which the reader
     * reads.
     *
     */
    struct sig_osc_PeakFrequency peakFrequency;

    /**
     * @brief The peak release time for which the peak frequency's
     * coefficient was calculated, in seconds.
     *
     */
    float previousPeakRelease;

    /**
     * @brief The frequency from which the octave was last selected,
     * in Hz.
     *
     * Selecting an octave needs a logarithm, so the octave is only
     * selected again when the peak frequency moves by more than 1%.
     */
    float levelFrequency;

    /**
     * @brief The additive wavetable bank to read.
     *
     * Must be assigned before the reader is first generated.
     */
    struct sig_AdditiveWavetableBank* wavetables;
};

/**
 * @brief Initialises an additive wavetable bank reader node.
 *
 * @param self the node to initialise
 * @param context the signal context
 */
void sig_dsp_AdditiveWavetableBankReader_init(
    struct sig_dsp_AdditiveWavetableBankReader* self,
    struct sig_SignalContext* context);

/**
 * @brief Creates a new additive wavetable bank reader node.
 *
 * @param allocator the allocator to use
 * @param context the signal context
 * @return a new additive wavetable bank reader node
 */
struct sig_dsp_AdditiveWavetableBankReader*
    sig_dsp_AdditiveWavetableBankReader_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context);

/**
 * @brief Generates a block of samples.
 *
 * @param signal the signal to generate, cast to a
 * struct sig_dsp_AdditiveWavetableBankReader*
 */
void sig_dsp_AdditiveWavetableBankReader_generate(void* signal);

/**
 * @brief Frees an additive wavetable bank reader node.
 *
 * @param allocator the allocator that created the node
 * @param self the node to free
 */
void sig_dsp_AdditiveWavetableBankReader_destroy(
    struct sig_Allocator* allocator,
    struct sig_dsp_AdditiveWavetableBankReader* self);


struct sig_dsp_Smooth_Inputs {
    float_array_ptr source;
};

struct sig_dsp_Smooth_Parameters {
    float time;
};

struct sig_dsp_Smooth {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Smooth_Inputs inputs;
    struct sig_dsp_Smooth_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    float a1;
    float previousTime;
    float previousSample;
};

void sig_dsp_Smooth_init(struct sig_dsp_Smooth* self,
    struct sig_SignalContext* context);
struct sig_dsp_Smooth* sig_dsp_Smooth_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Smooth_generate(void* signal);
void sig_dsp_Smooth_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Smooth* self);



struct sig_dsp_DCBlock_Inputs {
    float_array_ptr source;
};

struct sig_dsp_DCBlock_Parameters {
    float frequency;
};

struct sig_dsp_DCBlock {
    struct sig_dsp_Signal signal;
    struct sig_dsp_DCBlock_Inputs inputs;
    struct sig_dsp_DCBlock_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    struct sig_filter_DCBlock state;

    float previousFrequency;
};

void sig_dsp_DCBlock_init(struct sig_dsp_DCBlock* self,
    struct sig_SignalContext* context);
struct sig_dsp_DCBlock* sig_dsp_DCBlock_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_DCBlock_generate(void* signal);
void sig_dsp_DCBlock_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_DCBlock* self);



struct sig_dsp_EMA_Inputs {
    float_array_ptr source;
};

struct sig_dsp_EMA_Parameters {
    float alpha;
};

struct sig_dsp_EMA {
    struct sig_dsp_Signal signal;
    struct sig_dsp_EMA_Inputs inputs;
    struct sig_dsp_EMA_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    float previousSample;
};

void sig_dsp_EMA_init(struct sig_dsp_EMA* self,
    struct sig_SignalContext* context);
struct sig_dsp_EMA* sig_dsp_EMA_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_EMA_generate(void* signal);
void sig_dsp_EMA_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_EMA* self);



enum sig_dsp_OnePole_Mode {
    sig_dsp_OnePole_Mode_HIGH_PASS,
    sig_dsp_OnePole_Mode_LOW_PASS,
    sig_dsp_OnePole_Mode_NOT_SPECIFIED
};

struct sig_dsp_OnePole_Parameters {
    enum sig_dsp_OnePole_Mode mode;
};

struct sig_dsp_OnePole_Inputs {
    float_array_ptr source;
    float_array_ptr frequency;
};

struct sig_dsp_OnePole {
    struct sig_dsp_Signal signal;
    struct sig_dsp_OnePole_Inputs inputs;
    struct sig_dsp_OnePole_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    float b0;
    float a1;
    enum sig_dsp_OnePole_Mode previousMode;
    float previousFrequency;
    float previousSample;
};

void sig_dsp_OnePole_init(struct sig_dsp_OnePole* self,
    struct sig_SignalContext* context);
struct sig_dsp_OnePole* sig_dsp_OnePole_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context);
void sig_dsp_OnePole_recalculateCoefficients(struct sig_dsp_OnePole* self,
    float frequency);
void sig_dsp_OnePole_generate(void* signal);
void sig_dsp_OnePole_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_OnePole* self);


struct sig_dsp_Tanh_Inputs {
    float_array_ptr source;
};

struct sig_dsp_Tanh {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Tanh_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

void sig_dsp_Tanh_init(struct sig_dsp_Tanh* self,
    struct sig_SignalContext* context);
struct sig_dsp_Tanh* sig_dsp_Tanh_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Tanh_generate(void* signal);
void sig_dsp_Tanh_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Tanh* self);


struct sig_dsp_SoftClip_Parameters {
    /**
     * @brief The input magnitude above which compression begins.
     *
     * In the range 0.0 to 0.99. A value of 0.0 compresses from the start.
     */
    float threshold;
};

struct sig_dsp_SoftClip_Inputs {
    float_array_ptr source;
};

/**
 * @brief Limits a signal to the range -1.0 to 1.0 with a soft knee.
 *
 * Inputs:
 *  - source: the signal to limit
 */
struct sig_dsp_SoftClip {
    struct sig_dsp_Signal signal;
    struct sig_dsp_SoftClip_Inputs inputs;
    struct sig_dsp_SoftClip_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

void sig_dsp_SoftClip_init(struct sig_dsp_SoftClip* self,
    struct sig_SignalContext* context);
struct sig_dsp_SoftClip* sig_dsp_SoftClip_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_SoftClip_generate(void* signal);
void sig_dsp_SoftClip_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SoftClip* self);


struct sig_dsp_Dust_Parameters {
    float bipolar;
};

struct sig_dsp_Dust_Inputs {
    float_array_ptr density;
};

/**
 * Generates random impulses.
 *
 * If the bipolar parameter is set to 0 (the default),
 * the output will be in the range of 0 to 1.
 * If bipolar >0, impulses between -1 and 1 will be generated.
 *
 * Inputs:
 * - density: the number of impulses per second
 *
 * Parameters:
 * - bipolar: the output will be unipolar if <=0, bipolar if >0
 */
struct sig_dsp_Dust {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Dust_Inputs inputs;
    struct sig_dsp_Dust_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    // TODO: Should this be part of AudioSettings?
    float sampleDuration;

    float previousDensity;
    float threshold;
    float scale;
};

void sig_dsp_Dust_init(struct sig_dsp_Dust* self,
    struct sig_SignalContext* context);
struct sig_dsp_Dust* sig_dsp_Dust_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Dust_generate(void* signal);
void sig_dsp_Dust_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Dust* self);



struct sig_dsp_TimedGate_Parameters {
    float resetOnTrigger;
    float bipolar;
};

struct sig_dsp_TimedGate_Inputs {
    float_array_ptr trigger;
    float_array_ptr duration;
};

/**
 * A triggerable timed gate.
 *
 * When triggered, this signal will output a gate that matches the
 * amplitude and polarity of the trigger, for the specified duration.
 *
 * Inputs:
 *  - duration: the duration (in seconds) to remain open
 *  - trigger: a rising edge that will cause the gate to open
 *
 * Parameters:
 * - resetOnTrigger: when >0, the gate will close and then reopen
 *                   if a trigger is received while the gate is open
 * - bipolar: when >0, the gate will open when the trigger is
 *            either positive or negative. Otherwise only positive
 *            zero-crossings will cause the gate to open.
 *
 * Output: the gate's amplitude will reflect the value and
 *         polarity of the trigger
 */
struct sig_dsp_TimedGate {
    struct sig_dsp_Signal signal;
    struct sig_dsp_TimedGate_Inputs inputs;
    struct sig_dsp_TimedGate_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    float previousTrigger;
    float gateValue;
    float previousDuration;
    long durationSamps;
    long samplesRemaining;
};

void sig_dsp_TimedGate_init(struct sig_dsp_TimedGate* self,
    struct sig_SignalContext* context);
struct sig_dsp_TimedGate* sig_dsp_TimedGate_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_TimedGate_generate(void* signal);
void sig_dsp_TimedGate_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_TimedGate* self);


struct sig_dsp_DustGate_Inputs {
    float_array_ptr density;
    float_array_ptr durationPercentage;
};

struct sig_dsp_DustGate_Parameters {
    float bipolar;
};

struct sig_dsp_DustGate {
    struct sig_dsp_Signal signal;
    struct sig_dsp_DustGate_Inputs inputs;
    struct sig_dsp_DustGate_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
    struct sig_dsp_Dust* dust;
    struct sig_dsp_BinaryOp* reciprocalDensity;
    struct sig_dsp_BinaryOp* densityDurationMultiplier;
    struct sig_dsp_TimedGate* gate;
};

struct sig_dsp_DustGate* sig_dsp_DustGate_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context);
void sig_dsp_DustGate_init(struct sig_dsp_DustGate* self,
    struct sig_SignalContext* context);
void sig_dsp_DustGate_generate(void* signal);
void sig_dsp_DustGate_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_DustGate* self);


struct sig_dsp_Clock_Parameters {
    /**
     * @brief The minimum value that a trigger must reach to be
     * recognized as a clock pulse.
     *
     */
    float threshold;
};

/**
 * @brief Inputs for a ClockSource.
 */
struct sig_dsp_ClockSource_Inputs {
    float_array_ptr pulse;
    float_array_ptr tap;
};

/**
 * Provides a unified clock from two inputs:
 * 1. A consistent pulsed clock signal
 * 2. A tapped tempo
 *
 * When two pulses are received at the tap tempo input,
 * this signal will output a latched occuring at the detected frequency.
 * Howerver, whenever  a clock pulse is received,
 * it will always take precedence over the tapped tempo.
 *
 * Inputs:
 *  - pulses a clock signal
 *  - taps a manually tapped signal
 */
struct sig_dsp_ClockSource {
    struct sig_dsp_Signal signal;
    struct sig_dsp_ClockSource_Inputs inputs;
    struct sig_dsp_Clock_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    float previousTapValue;
    uint8_t tapCount;
    uint32_t tempoDurationSamples;
    uint32_t numHighSamples;
    uint32_t samplesSinceLastTap;
    bool isLatching;
    uint32_t latchedTapSamplesRemaining;
};

void sig_dsp_ClockSource_init(
    struct sig_dsp_ClockSource* self,
    struct sig_SignalContext* context);
struct sig_dsp_ClockSource* sig_dsp_ClockSource_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_ClockSource_generate(void* signal);
void sig_dsp_ClockSource_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ClockSource* self);



/**
 * @brief Inputs for a ClockDetector.
 */
struct sig_dsp_ClockDetector_Inputs {
    /**
     * @brief The incoming clock signal.
     */
    float_array_ptr source;
};

struct sig_dsp_ClockDetector_Outputs {
    /**
     * @brief The detected clock frequency in pulses per second (i.e. Hz).
     */
    float_array_ptr main;

    /**
     * @brief The detected clock frequency in beats per minute.
     */
    float_array_ptr bpm;
};

void sig_dsp_ClockDetector_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_ClockDetector_Outputs* outputs);

void sig_dsp_ClockDetector_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_ClockDetector_Outputs* outputs);


/**
 * Detects the number of pulses per second of an incoming clock signal
 * (i.e. any signal that outputs a rising edge to denote clock pulses)
 * and outputs the clock's tempo in pulses per second.
 *
 * Inputs:
 *  - source the incoming clock signal
 */
struct sig_dsp_ClockDetector {
    struct sig_dsp_Signal signal;
    struct sig_dsp_ClockDetector_Inputs inputs;
    struct sig_dsp_Clock_Parameters parameters;
    struct sig_dsp_ClockDetector_Outputs outputs;

    float previousTrigger;
    bool isRisingEdge;
    uint8_t numPulsesDetected;
    uint32_t samplesSinceLastPulse;
    float clockFreq;
    uint32_t pulseDurSamples;
};

void sig_dsp_ClockDetector_init(
    struct sig_dsp_ClockDetector* self,
    struct sig_SignalContext* context);
struct sig_dsp_ClockDetector* sig_dsp_ClockDetector_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_ClockDetector_generate(void* signal);
void sig_dsp_ClockDetector_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ClockDetector* self);

/**
 * @brief The inputs of a clock phasor.
 */
struct sig_dsp_ClockPhasor_Inputs {
    /**
     * @brief The clock signal, whose rising edges are clock pulses.
     *
     */
    float_array_ptr pulse;
};

/**
 * @brief A phasor that follows a train of clock pulses.
 *
 * The phasor's output can be the reference phase of a sig_dsp_RatioPhasor,
 * which multiplies or divides the clock. See sig_osc_ClockPhasor for how
 * the phasor follows the clock.
 *
 * Inputs:
 *  - pulse: the clock signal, whose rising edges are clock pulses
 */
struct sig_dsp_ClockPhasor {
    /**
     * @brief The signal's base state.
     *
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The phasor's inputs.
     *
     */
    struct sig_dsp_ClockPhasor_Inputs inputs;

    /**
     * @brief The phasor's parameters.
     *
     */
    struct sig_dsp_Clock_Parameters parameters;

    /**
     * @brief The phasor's output, its phase normalized to 0.0-1.0.
     *
     */
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    /**
     * @brief The phasor's state.
     *
     */
    struct sig_osc_ClockPhasor state;
};

/**
 * @brief Initialises a clock phasor node.
 *
 * @param self the node to initialise
 * @param context the signal context
 */
void sig_dsp_ClockPhasor_init(struct sig_dsp_ClockPhasor* self,
    struct sig_SignalContext* context);

/**
 * @brief Creates a new clock phasor node.
 *
 * @param allocator the allocator to use
 * @param context the signal context
 * @return a new clock phasor node
 */
struct sig_dsp_ClockPhasor* sig_dsp_ClockPhasor_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);

/**
 * @brief Generates a block of samples.
 *
 * @param signal the signal to generate, cast to a struct sig_dsp_ClockPhasor*
 */
void sig_dsp_ClockPhasor_generate(void* signal);

/**
 * @brief Frees a clock phasor node.
 *
 * @param allocator the allocator that created the node
 * @param self the node to free
 */
void sig_dsp_ClockPhasor_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_ClockPhasor* self);

/**
 * @brief The slowest tempo that a tempo detector supports, in beats
 * per minute.
 */
#define sig_dsp_TempoDetector_LOWEST_TEMPO 30.0f

/**
 * @brief The fastest tempo that a tempo detector supports, in beats
 * per minute.
 */
#define sig_dsp_TempoDetector_HIGHEST_TEMPO 200.0f

/**
 * @brief The inputs of a tempo detector.
 */
struct sig_dsp_TempoDetector_Inputs {
    /**
     * @brief The audio to analyse, where 1.0 is full scale.
     */
    float_array_ptr source;

    /**
     * @brief The sync signal, whose rises to 0.5 or above tap beats.
     */
    float_array_ptr sync;
};

/**
 * @brief The parameters of a tempo detector.
 */
struct sig_dsp_TempoDetector_Parameters {
    /**
     * @brief The level below which the onset detector ignores sound.
     *
     * In dB relative to a full-scale sine, from -90 to -30. Defaults
     * to -70.
     */
    float threshold;

    /**
     * @brief The slowest tempo that the detector allows.
     *
     * In beats per minute, from 30 to 200. Defaults to 30.
     */
    float minTempo;

    /**
     * @brief The fastest tempo that the detector allows.
     *
     * In beats per minute, from 30 to 200. Defaults to 200.
     */
    float maxTempo;

    /**
     * @brief How strongly the follower corrects its phase and tempo.
     *
     * From 0 to 1. Defaults to 0.6.
     */
    float responsiveness;

    /**
     * @brief How far ahead of each predicted beat the clock fires.
     *
     * In seconds, from 0 to 0.25. Defaults to 0.
     */
    float lead;

    /**
     * @brief The length of each clock trigger.
     *
     * In seconds, from 0.001 to 0.1. Defaults to 0.01.
     */
    float triggerDuration;

    /**
     * @brief Whether the clock mutes, 0, or free-runs, 1, while the
     * follower is silent.
     *
     * Defaults to 0.
     */
    float silenceBehaviour;

    /**
     * @brief How many beats without a strong beat make the follower
     * silent.
     *
     * From 1 to 16. Defaults to 4.
     */
    float silenceTimeout;
};

/**
 * @brief The outputs of a tempo detector.
 */
struct sig_dsp_TempoDetector_Outputs {
    /**
     * @brief The clock, which is 1 during each trigger and 0 otherwise.
     */
    float_array_ptr clock;

    /**
     * @brief The beat phase, shifted earlier by the lead, from 0 to 1.
     *
     * Zero while the clock is muted.
     */
    float_array_ptr phase;

    /**
     * @brief The detected tempo, in beats per minute.
     *
     * Zero before the first tempo estimate.
     */
    float_array_ptr bpm;

    /**
     * @brief The follower's confidence, from 0 to 1.
     */
    float_array_ptr confidence;
};

/**
 * @brief Allocates a tempo detector's output blocks.
 *
 * @param allocator the allocator to use
 * @param audioSettings the audio settings that determine the block size
 * @param outputs the outputs to allocate
 */
void sig_dsp_TempoDetector_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_TempoDetector_Outputs* outputs);

/**
 * @brief Frees a tempo detector's output blocks.
 *
 * @param allocator the allocator that allocated the blocks
 * @param outputs the outputs to free
 */
void sig_dsp_TempoDetector_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_TempoDetector_Outputs* outputs);

/**
 * @brief A tempo detector that listens to live music and outputs a clock
 * that follows its beat.
 *
 * The detector runs four stages. A sig_tempo_OnsetDetector converts the
 * audio into an onset strength envelope, a sig_tempo_TempoEstimator
 * estimates its tempo, a sig_tempo_BeatTracker finds its beats and
 * follows them with a beat phase, and a sig_tempo_ClockGenerator
 * converts the phase into triggers. The detector chooses the onset
 * detector's window length, the power of two nearest to 46.4 ms of
 * samples, and its hop length, the sample rate divided by 86.13, from
 * the sample rate. It supports tempos from 30 to 200 BPM. It reads its
 * parameters at the start of each block. When the sync input rises to
 * 0.5 or above, the detector taps the beat tracker on that sample.
 *
 * Inputs:
 *  - source: the audio to analyse
 *  - sync: taps that set the beat's phase and tempo
 */
struct sig_dsp_TempoDetector {
    /**
     * @brief The signal's base state.
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The detector's inputs.
     */
    struct sig_dsp_TempoDetector_Inputs inputs;

    /**
     * @brief The detector's parameters.
     */
    struct sig_dsp_TempoDetector_Parameters parameters;

    /**
     * @brief The detector's outputs.
     */
    struct sig_dsp_TempoDetector_Outputs outputs;

    /**
     * @brief The onset detector's window length, in samples.
     */
    size_t windowLength;

    /**
     * @brief The onset detector's hop length, in samples.
     */
    size_t hopLength;

    /**
     * @brief The onset detector's latency, which the beat tracker
     * subtracts from beat times.
     *
     * In samples, (windowLength - hopLength) / 2.
     */
    float latency;

    /**
     * @brief The onset detector.
     */
    struct sig_tempo_OnsetDetector* onsetDetector;

    /**
     * @brief The tempo estimator.
     */
    struct sig_tempo_TempoEstimator* tempoEstimator;

    /**
     * @brief The beat tracker.
     */
    struct sig_tempo_BeatTracker* beatTracker;

    /**
     * @brief The clock generator.
     */
    struct sig_tempo_ClockGenerator clockGenerator;

    /**
     * @brief The tempo estimator's most recent estimate, in beats per
     * minute.
     */
    float tempoEstimate;

    /**
     * @brief The sync input's value on the previous sample.
     */
    float previousSync;
};

/**
 * @brief Calculates a tempo detector's window length, the power of two
 * nearest to 46.4 ms of samples.
 *
 * @param sampleRate the sample rate in Hz
 * @return the window length in samples
 */
size_t sig_dsp_TempoDetector_calcWindowLength(float sampleRate);

/**
 * @brief Calculates a tempo detector's hop length, the sample rate
 * divided by 86.13, rounded to the nearest sample.
 *
 * @param sampleRate the sample rate in Hz
 * @return the hop length in samples
 */
size_t sig_dsp_TempoDetector_calcHopLength(float sampleRate);

/**
 * @brief Initialises a tempo detector's parameters, clock generator,
 * and inputs.
 *
 * The detector's onset detector, tempo estimator, and beat tracker must
 * already be allocated and initialised, as sig_dsp_TempoDetector_new
 * does.
 *
 * @param self the detector to initialise
 * @param context the signal context
 */
void sig_dsp_TempoDetector_init(struct sig_dsp_TempoDetector* self,
    struct sig_SignalContext* context);

/**
 * @brief Allocates and initialises a tempo detector and its stages.
 *
 * @param allocator the allocator to use
 * @param context the signal context, whose sample rate determines the
 * window and hop lengths
 * @return the new detector
 */
struct sig_dsp_TempoDetector* sig_dsp_TempoDetector_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);

/**
 * @brief Generates a block of the detector's outputs.
 *
 * @param signal the signal to generate, cast to a struct
 * sig_dsp_TempoDetector*
 */
void sig_dsp_TempoDetector_generate(void* signal);

/**
 * @brief Frees a tempo detector and its stages.
 *
 * @param allocator the allocator that created the detector
 * @param self the detector to free
 */
void sig_dsp_TempoDetector_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_TempoDetector* self);




struct sig_dsp_LinearToFreq_Inputs {
    float_array_ptr source;
};

struct sig_dsp_LinearToFreq_Parameters {
    float middleFreq;
};

struct sig_dsp_LinearToFreq {
    struct sig_dsp_Signal signal;
    struct sig_dsp_LinearToFreq_Inputs inputs;
    struct sig_dsp_LinearToFreq_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_LinearToFreq* sig_dsp_LinearToFreq_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_LinearToFreq_init(struct sig_dsp_LinearToFreq* self,
    struct sig_SignalContext* context);
void sig_dsp_LinearToFreq_generate(void* signal);
void sig_dsp_LinearToFreq_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_LinearToFreq* self);


struct sig_dsp_Branch_Inputs {
    float_array_ptr off;
    float_array_ptr on;
    float_array_ptr condition;
};

struct sig_dsp_Branch {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Branch_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_Branch* sig_dsp_Branch_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Branch_init(struct sig_dsp_Branch* self,
    struct sig_SignalContext* context);
void sig_dsp_Branch_generate(void* signal);
void sig_dsp_Branch_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Branch* self);


struct sig_dsp_List_Inputs {
    float_array_ptr index;
};

struct sig_dsp_List_Outputs {
    float_array_ptr main;
    float_array_ptr index;
    float_array_ptr length;
};

void sig_dsp_List_Outputs_newAudioBlocks(struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_List_Outputs* outputs);

void sig_dsp_List_Outputs_destroyAudioBlocks(struct sig_Allocator* allocator,
    struct sig_dsp_List_Outputs* outputs);


struct sig_dsp_List_Parameters {
    float wrap;
    float normalizeIndex;
    float interpolate;
};

struct sig_dsp_List {
    struct sig_dsp_Signal signal;
    struct sig_dsp_List_Inputs inputs;
    struct sig_dsp_List_Parameters parameters;
    struct sig_dsp_List_Outputs outputs;
    struct sig_Buffer* list;
};

struct sig_dsp_List* sig_dsp_List_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_List_init(struct sig_dsp_List* self,
    struct sig_SignalContext* context);
float sig_dsp_List_constrain(bool shouldWrap, float index,
    float lastIndex, float listLength);
void sig_dsp_List_generate(void* signal);
void sig_dsp_List_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_List* self);


struct sig_dsp_Sequencer_Inputs {
    float_array_ptr mul;
    float_array_ptr add;
};

struct sig_dsp_Sequencer_Outputs {
    /**
     * @brief The sequencer's output.
     */
    float_array_ptr main;

    /**
     * @brief The current stage index.
     */
    float_array_ptr index;
};

void sig_dsp_Sequencer_Outputs_newAudioBlocks(struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_Sequencer_Outputs* outputs);

void sig_dsp_Sequencer_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_Sequencer_Outputs* outputs);

struct sig_dsp_Sequencer_Parameters {
    float loop;
    float resetOnNext;
    float holdLastValue;
};

/**
 * Sequencer outputs a sequence of values
 * for the specified durations.
 *
 * When the resetOnNext parameter is set,
 * the sequencer will reset its value to 0.0 for one sample
 * prior to moving to the next duration.
 * This is useful for sequencing envelope gates, for example.
 *
 * Members:
 *     durations: an array of durations (in seconds) to hold each value
 *     values: an array of values to output
 *
 * Parameters:
 *     loop: if > 0, the sequencer will loop back
 *       to the first stage when it reaches the end;
 *       defaults to 0.
 *     resetOnNext: when high, output 0.0 prior to moving
 *       to the next sequencer stage
 *     holdLastValue: when high, the last value of the
 *       will continue to be output after the
 *       sequence has completed.
 */
struct sig_dsp_Sequencer {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Sequencer_Inputs inputs;
    struct sig_dsp_Sequencer_Parameters parameters;
    struct sig_dsp_Sequencer_Outputs outputs;
    struct sig_Buffer* durations;
    struct sig_Buffer* values;
    size_t samplesRemaining;
    int32_t stepIndex;
    float lastValue;
};

struct sig_dsp_Sequencer* sig_dsp_Sequencer_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Sequencer_init(struct sig_dsp_Sequencer* self,
    struct sig_SignalContext* context);
float sig_dsp_Sequencer_startStage(struct sig_dsp_Sequencer* self);
void sig_dsp_Sequencer_generate(void* signal);
void sig_dsp_Sequencer_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Sequencer* self);



struct sig_dsp_LinearMap_Inputs {
    float_array_ptr source;
};

struct sig_dsp_LinearMap_Parameters {
    float fromMin;
    float fromMax;
    float toMin;
    float toMax;
};

struct sig_dsp_LinearMap {
    struct sig_dsp_Signal signal;
    struct sig_dsp_LinearMap_Inputs inputs;
    struct sig_dsp_LinearMap_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_LinearMap* sig_dsp_LinearMap_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_LinearMap_init(struct sig_dsp_LinearMap* self,
    struct sig_SignalContext* context);
void sig_dsp_LinearMap_generate(void* signal);
void sig_dsp_LinearMap_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_LinearMap* self);


struct sig_dsp_TwoOpFM_Inputs {
    float_array_ptr frequency;
    float_array_ptr index;
    float_array_ptr ratio;
    float_array_ptr phaseOffset;
    float_array_ptr modulatorPhaseOffset;
    float_array_ptr feedbackGain;
};

struct sig_dsp_TwoOpFM_Outputs {
    float_array_ptr main;
    float_array_ptr carrierEOC;
    float_array_ptr modulator;
    float_array_ptr modulatorEOC;
};

void sig_dsp_TwoOpFM_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_TwoOpFM_Outputs* outputs);

void sig_dsp_TwoOpFM_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_TwoOpFM_Outputs* outputs);


struct sig_dsp_TwoOpFM {
    struct sig_dsp_Signal signal;
    struct sig_dsp_TwoOpFM_Inputs inputs;
    struct sig_dsp_TwoOpFM_Outputs outputs;
    struct sig_osc_Wavetable carrierState;
    struct sig_osc_Wavetable modulatorState;
    struct sig_DelayLine feedbackDelay;

    struct sig_Buffer* sineTable;
};

struct sig_dsp_TwoOpFM* sig_dsp_TwoOpFM_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context);
void sig_dsp_TwoOpFM_init(struct sig_dsp_TwoOpFM* self,
    struct sig_SignalContext* context);
void sig_dsp_TwoOpFM_generate(void* signal);
void sig_dsp_TwoOpFM_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_TwoOpFM* self);


struct sig_dsp_FourPoleFilter_Inputs {
    float_array_ptr source;
    float_array_ptr frequency;
    float_array_ptr resonance;
    float_array_ptr inputGain;
    float_array_ptr pole1Gain;
    float_array_ptr pole2Gain;
    float_array_ptr pole3Gain;
    float_array_ptr pole4Gain;
};


struct sig_dsp_FourPoleFilter_Outputs {
    /**
     * @brief A mix of all filter stages based on the stage mix inputs.
     */
    float_array_ptr main;

    /**
     * @brief The full two pole 12 dB output of the filter.
     */
    float_array_ptr twoPole;

    /**
     * @brief The full four pole 24 dB output from the filter.
     */
    float_array_ptr fourPole;
};

void sig_dsp_FourPoleFilter_Outputs_newAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_FourPoleFilter_Outputs* outputs);

void sig_dsp_FourPoleFilter_Outputs_destroyAudioBlocks(
    struct sig_Allocator* allocator,
    struct sig_dsp_FourPoleFilter_Outputs* outputs);


struct sig_dsp_Ladder_Parameters {
    /**
     * @brief The passband gain of the filter (best between 0.0-0.5),
     * higher values will amplify the lower frequencies when resonance is high.
     */
    float passbandGain;
};

/**
 * @brief Valimaki and Huovilainen's Moog-style Ladder Filter
 * Ported from Infrasonic Audio LLC's adaptation of
 * Richard Van Hoesel's implementation from the Teensy Audio Library.
 * https://gist.github.com/ndonald2/534831b639b8c78d40279b5007e06e5b
 *
 * Inputs:
 *  - source: the input to filter
 *  - frequency: the cutoff frequency in Hz
 *  - resonance: the resonance (stable values in the range 0 - 1.8)
 */
struct sig_dsp_Ladder {
    /**
     * @brief The signal's base state.
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The filter's inputs.
     */
    struct sig_dsp_FourPoleFilter_Inputs inputs;

    /**
     * @brief The filter's parameters.
     */
    struct sig_dsp_Ladder_Parameters parameters;

    /**
     * @brief The filter's outputs.
     */
    struct sig_dsp_FourPoleFilter_Outputs outputs;

    /**
     * @brief The filter's state.
     */
    struct sig_filter_Ladder state;

    /**
     * @brief The frequency from the previous sample.
     */
    float previousFrequency;
};

struct sig_dsp_Ladder* sig_dsp_Ladder_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Ladder_init(
    struct sig_dsp_Ladder* self,
    struct sig_SignalContext* context);
void sig_dsp_Ladder_generate(void* signal);
void sig_dsp_Ladder_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Ladder* self);



struct sig_dsp_TiltEQ_Inputs {
    float_array_ptr source;
    float_array_ptr frequency; // 20-20000 Hz
    float_array_ptr gain; // -6 to +6 dB, normalized to -1.0 to 1.0
};

/**
 * @brief A Tilt Equalizer.
 * Boosts a range of the spectrum above or below the centre frequency,
 * while attunuating the opposite range. More extreme settings will
 * turn the filter into first order low-pass or high-pass.
 * This is achieved with greater gain factors
 * (ex: +6db for high pass, or -6db for low pass)
 *
 * This implementation is written by moc.liamg@321tiloen
 * from the musicdsp.org archive. It is an emulation
 * of the "Niveau" filter from the "Elysia mPressor" compressor plugin.
 *
 * https://www.musicdsp.org/en/latest/Filters/267-simple-tilt-equalizer.html
 *
 *  Inputs:
 *  - source: the input to filter
 *  - frequency: the centre frequency in Hz (20-20000 Hz)
 *  - gain: the EQ gain, in dB (-6.0 to +6.0 dB)
 */
struct sig_dsp_TiltEQ {
    struct sig_dsp_Signal signal;
    struct sig_dsp_TiltEQ_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    float sr3;
    float lpOut;
};

struct sig_dsp_TiltEQ* sig_dsp_TiltEQ_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_TiltEQ_init(struct sig_dsp_TiltEQ* self,
    struct sig_SignalContext* context);
void sig_dsp_TiltEQ_generate(void* signal);
void sig_dsp_TiltEQ_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_TiltEQ* self);



/**
 * @brief The state variable filter's inputs.
 */
struct sig_dsp_SVF_Inputs {
    float_array_ptr source;
    float_array_ptr frequency;
    float_array_ptr resonance;
};

/**
 * @brief The state variable filter's outputs, one for each response shape.
 */
struct sig_dsp_SVF_Outputs {
    /**
     * @brief The low pass output.
     */
    float_array_ptr low;

    /**
     * @brief The band pass output.
     */
    float_array_ptr band;

    /**
     * @brief The high pass output.
     */
    float_array_ptr high;

    /**
     * @brief The notch output, the sum of the low and high pass outputs.
     */
    float_array_ptr notch;

    /**
     * @brief The peak output, the difference of the high and low pass outputs.
     */
    float_array_ptr peak;

    /**
     * @brief The all pass output.
     */
    float_array_ptr all;
};

/**
 * @brief Allocates a sample block for each of the filter's outputs.
 *
 * @param allocator the allocator to use
 * @param audioSettings the audio settings that set each block's size
 * @param outputs the outputs to allocate the blocks for
 */
void sig_dsp_SVF_Outputs_newAudioBlocks(struct sig_Allocator* allocator,
    struct sig_AudioSettings* audioSettings,
    struct sig_dsp_SVF_Outputs* outputs);

/**
 * @brief Frees the filter's output blocks.
 *
 * @param allocator the allocator that allocated the blocks
 * @param outputs the outputs to free the blocks from
 */
void sig_dsp_SVF_Outputs_destroyAudioBlocks(struct sig_Allocator* allocator,
    struct sig_dsp_SVF_Outputs* outputs);

/**
 * @brief The Cytomic State Variable Filter.
 *
 * This filter implements the optimised trapezoidal state variable filter from
 * Andrew Simper's "Solving the continuous SVF equations using
 * trapezoidal integration and equivalent currents" (2016).
 * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
 *
 * It is a stable second order filter with a 12 dB per octave cutoff slope.
 * The filter provides separate simultaneous outputs for its low pass, band pass,
 * high pass, notch, peak, and all pass responses.
 * Audio rate modulation of the filter's inputs is stable.
 *
 * Inputs:
 *  - source: the signal to filter
 *  - frequency: the cutoff frequency in Hz, from 0 to 0.49 times the sample rate
 *  - resonance: the resonance, normalized to 0.0 to 1.0. A value of 0.0 gives
 *    a heavily damped filter with a Q of 0.5, and a value of 1.0 gives the
 *    maximum resonance, with a Q of 100
 */
struct sig_dsp_SVF {
    /**
     * @brief The signal's base state.
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The filter's inputs.
     */
    struct sig_dsp_SVF_Inputs inputs;

    /**
     * @brief The filter's outputs, one for each response shape.
     */
    struct sig_dsp_SVF_Outputs outputs;

    /**
     * @brief The filter's state.
     */
    struct sig_filter_SVF state;

    /**
     * @brief The frequency from the previous sample, in Hz.
     */
    float previousFrequency;

    /**
     * @brief The resonance from the previous sample.
     */
    float previousResonance;
};

/**
 * @brief Initialises a state variable filter node.
 *
 * @param self the node to initialise
 * @param context the signal context
 */
void sig_dsp_SVF_init(struct sig_dsp_SVF* self,
    struct sig_SignalContext* context);

/**
 * @brief Creates a new state variable filter node.
 *
 * @param allocator the allocator to use
 * @param context the signal context
 * @return a new state variable filter node
 */
struct sig_dsp_SVF* sig_dsp_SVF_new(struct sig_Allocator* allocator,
    struct sig_SignalContext* context);

/**
 * @brief Generates a block of samples.
 *
 * @param signal the signal to generate, cast to a struct sig_dsp_SVF*
 */
void sig_dsp_SVF_generate(void* signal);

/**
 * @brief Frees a state variable filter node.
 *
 * @param allocator the allocator that created the node
 * @param self the node to free
 */
void sig_dsp_SVF_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SVF* self);

/**
 * @brief A low pass state variable filter.
 *
 * This filter uses the same core as sig_dsp_SVF, but writes only its low pass output.
 * Use it to reduce memory consumption when only a low pass response is needed.
 *
 * Inputs:
 *  - source: the signal to filter
 *  - frequency: the cutoff frequency in Hz, from 0 to 0.49 times the sample rate
 *  - resonance: the resonance, normalized to 0.0 to 1.0
 */
struct sig_dsp_SVFLowPass {
    /**
     * @brief The signal's base state.
     */
    struct sig_dsp_Signal signal;

    /**
     * @brief The filter's inputs.
     */
    struct sig_dsp_SVF_Inputs inputs;

    /**
     * @brief The filter's main output.
     */
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    /**
     * @brief The filter's state.
     */
    struct sig_filter_SVF state;

    /**
     * @brief The frequency from the previous sample, in Hz.
     */
    float previousFrequency;

    /**
     * @brief The resonance from the previous sample.
     */
    float previousResonance;
};

/**
 * @brief Initialises a low pass state variable filter node.
 *
 * @param self the node to initialise
 * @param context the signal context
 */
void sig_dsp_SVFLowPass_init(struct sig_dsp_SVFLowPass* self,
    struct sig_SignalContext* context);

/**
 * @brief Creates a new low pass state variable filter node.
 *
 * @param allocator the allocator to use
 * @param context the signal context
 * @return a new low pass state variable filter node
 */
struct sig_dsp_SVFLowPass* sig_dsp_SVFLowPass_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);

/**
 * @brief Generates a block of samples.
 *
 * @param signal the struct sig_dsp_SVFLowPass signal to generate
 */
void sig_dsp_SVFLowPass_generate(void* signal);

/**
 * @brief Frees a low pass state variable filter node.
 *
 * @param allocator the allocator that created the node
 * @param self the node to free
 */
void sig_dsp_SVFLowPass_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SVFLowPass* self);



struct sig_dsp_Delay_Inputs {
    float_array_ptr source;
    float_array_ptr delayTime;
};

struct sig_dsp_Delay {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Delay_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    struct sig_DelayLine* delayLine;
};

struct sig_dsp_Delay* sig_dsp_Delay_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Delay_init(struct sig_dsp_Delay* self,
    struct sig_SignalContext* context);
void sig_dsp_Delay_read(struct sig_dsp_Delay* self, float source,
    size_t i);
void sig_dsp_Delay_generate(void* signal);
void sig_dsp_Delay_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Delay* self);

struct sig_dsp_Delay* sig_dsp_DelayTap_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_DelayTap_init(struct sig_dsp_Delay* self,
    struct sig_SignalContext* context);
void sig_dsp_DelayTap_generate(void* signal);
void sig_dsp_DelayTap_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Delay* self);

struct sig_dsp_DelayWrite_Inputs {
    float_array_ptr source;
};

struct sig_dsp_DelayWrite {
    struct sig_dsp_Signal signal;
    struct sig_dsp_DelayWrite_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    struct sig_DelayLine* delayLine;
};

struct sig_dsp_DelayWrite* sig_dsp_DelayWrite_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_DelayWrite_init(struct sig_dsp_DelayWrite* self,
    struct sig_SignalContext* context);
void sig_dsp_DelayWrite_generate(void* signal);
void sig_dsp_DelayWrite_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_DelayWrite* self);


struct sig_dsp_Comb_Inputs {
    float_array_ptr source;
    float_array_ptr delayTime;
    float_array_ptr feedbackGain;
    float_array_ptr lpfCoefficient;
};

struct sig_dsp_Comb {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Comb_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    struct sig_DelayLine* delayLine;
    float previousSample;
};

struct sig_dsp_Comb* sig_dsp_Comb_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Comb_init(struct sig_dsp_Comb* self,
    struct sig_SignalContext* context);
void sig_dsp_Comb_generate(void* signal);
void sig_dsp_Comb_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Comb* self);



struct sig_dsp_Allpass_Inputs {
    float_array_ptr source;
    float_array_ptr delayTime;
    float_array_ptr g;
};

struct sig_dsp_Allpass {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Allpass_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    struct sig_DelayLine* delayLine;
};

struct sig_dsp_Allpass* sig_dsp_Allpass_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Allpass_init(struct sig_dsp_Allpass* self,
    struct sig_SignalContext* context);
void sig_dsp_Allpass_generate(void* signal);
void sig_dsp_Allpass_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Allpass* self);



struct sig_dsp_Chorus_Inputs {
    float_array_ptr source;
    float_array_ptr delayTime;
    float_array_ptr speed;
    float_array_ptr width;
    float_array_ptr feedbackGain;
    float_array_ptr feedforwardGain;
    float_array_ptr blend;
};

struct sig_dsp_Chorus_Outputs {
    float_array_ptr main;
    float_array_ptr modulator;
};

struct sig_dsp_Chorus {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Chorus_Inputs inputs;
    struct sig_dsp_Chorus_Outputs outputs;

    struct sig_DelayLine* delayLine;
    struct sig_osc_FastLFSine modulator;
    float previousFixedOutput;
    float previousModulatedOutput;
};

struct sig_dsp_Chorus* sig_dsp_Chorus_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Chorus_init(struct sig_dsp_Chorus* self,
    struct sig_SignalContext* context);
void sig_dsp_Chorus_generate(void* signal);
void sig_dsp_Chorus_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Chorus* self);



struct sig_dsp_LinearXFade_Inputs {
    float_array_ptr left;
    float_array_ptr right;
    float_array_ptr mix;
};

struct sig_dsp_LinearXFade {
    struct sig_dsp_Signal signal;
    struct sig_dsp_LinearXFade_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_LinearXFade* sig_dsp_LinearXFade_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_LinearXFade_init(struct sig_dsp_LinearXFade* self,
    struct sig_SignalContext* context);
void sig_dsp_LinearXFade_generate(void* signal);
void sig_dsp_LinearXFade_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_LinearXFade* self);



// TODO: Don't hardcode these.
#define sig_dsp_Calibrator_NUM_STAGES 6
#define sig_dsp_Calibrator_TARGET_VALUES {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 4.75f}

struct sig_dsp_Calibrator_Inputs {
    float_array_ptr source;
    float_array_ptr gate;
};

struct sig_dsp_Calibrator_Node {
    float target;
    size_t numSamplesRecorded;
    float min;
    float max;
    float sum;
    float avg;
    float diff;
};

void sig_dsp_Calibrator_Node_init(struct sig_dsp_Calibrator_Node* nodes,
    float_array_ptr targetValues, size_t numNodes);

size_t sig_dsp_Calibrator_locateIntervalForValue(float x,
    struct sig_dsp_Calibrator_Node* nodes, size_t numNodes);

float sig_dsp_Calibrator_fitValueToCalibrationData(float x,
    struct sig_dsp_Calibrator_Node* nodes, size_t numNodes);

struct sig_dsp_Calibrator {
    struct sig_dsp_Signal signal;
    struct sig_dsp_Calibrator_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;

    struct sig_dsp_Calibrator_Node nodes[sig_dsp_Calibrator_NUM_STAGES];
    float previousGate;
    size_t stage;
};

struct sig_dsp_Calibrator* sig_dsp_Calibrator_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_Calibrator_init(struct sig_dsp_Calibrator* self,
    struct sig_SignalContext* context);
void sig_dsp_Calibrator_generate(void* signal);
void sig_dsp_Calibrator_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_Calibrator* self);


struct sig_dsp_SineWavefolder_Inputs {
    float_array_ptr source;
    float_array_ptr gain;
    float_array_ptr factor;
};

struct sig_dsp_SineWavefolder {
    struct sig_dsp_Signal signal;
    struct sig_dsp_SineWavefolder_Inputs inputs;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_SineWavefolder* sig_dsp_SineWavefolder_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_SineWavefolder_init(struct sig_dsp_SineWavefolder* self,
    struct sig_SignalContext* context);
void sig_dsp_SineWavefolder_generate(void* signal);
void sig_dsp_SineWavefolder_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_SineWavefolder* self);


struct sig_dsp_NoiseGate_Inputs {
    float_array_ptr source;
    float_array_ptr threshold;
};

struct sig_dsp_NoiseGate {
    struct sig_dsp_Signal signal;
    struct sig_dsp_NoiseGate_Inputs inputs;
    struct sig_dsp_ScaleOffset_Parameters parameters;
    struct sig_dsp_Signal_SingleMonoOutput outputs;
};

struct sig_dsp_NoiseGate* sig_dsp_NoiseGate_new(
    struct sig_Allocator* allocator, struct sig_SignalContext* context);
void sig_dsp_NoiseGate_init(struct sig_dsp_NoiseGate* self,
    struct sig_SignalContext* context);
void sig_dsp_NoiseGate_generate(void* signal);
void sig_dsp_NoiseGate_destroy(struct sig_Allocator* allocator,
    struct sig_dsp_NoiseGate* self);


/*******************************
 * Inline function definitions *
 *******************************/

// Every function above is declared before the function definitions below,
// so that the functions here can call the declared function above.

#define sig_DelayLine_readAtTime_IMPL(self, source, tapTime,\
    sampleRate, readFn)\
    float sample;\
    if (tapTime <= 0.0f) {\
        sample = source;\
    } else {\
        float readPos = tapTime * sampleRate;\
        float maxDelayLength = (float) self->buffer->length;\
        if (readPos >= maxDelayLength) {\
            readPos = maxDelayLength - 1;\
        }\
        sample = readFn(self, readPos);\
    }\
    return sample

#define sig_DelayLine_readAtTimes_IMPL(self, source, tapTimes,\
    tapGains, numTaps, sampleRate, timeScale, readFn)\
    const float* SIG_RESTRICT restrictTapTimes = FLOAT_ARRAY(tapTimes);\
    const float* SIG_RESTRICT restrictTapGains = FLOAT_ARRAY(tapGains);\
    float tapSum = 0;\
    for (size_t i = 0; i < numTaps; i++) {\
        float tapTime = restrictTapTimes[i];\
        float scaledTapTime = tapTime * timeScale;\
        float gain = restrictTapGains[i];\
        float sample;\
        if (tapTime <= 0.0f) {\
            sample = source;\
        } else {\
            float readPos = scaledTapTime * sampleRate;\
            float maxDelayLength = (float) self->buffer->length;\
            if (readPos >= maxDelayLength) {\
                readPos = maxDelayLength - 1;\
            }\
            sample = readFn(self, readPos);\
        }\
        tapSum += sample * gain;\
    }\
    return tapSum

#define sig_DelayLine_comb_IMPL(self, sample, readPos, g, readFn) \
    float read = readFn(self, readPos); \
    float toWrite = sig_DelayLine_feedback(sample, read, g); \
    sig_DelayLine_write(self, toWrite); \
    return read

#define sig_DelayLine_allpass_IMPL(self, sample, readPos, g,\
    readFn) \
    float read = readFn(self, readPos); \
    float toWrite = sample + (g * read); \
    sig_DelayLine_write(self, toWrite); \
    return read - (g * toWrite) \


inline float sig_fminf(float a, float b) {
    float r;
#ifdef __arm__
    asm("vminnm.f32 %[d], %[n], %[m]" : [d] "=t"(r) : [n] "t"(a), [m] "t"(b) :);
#else
    r = (a < b) ? a : b;
#endif // __arm__
    return r;
}

inline float sig_fmaxf(float a, float b) {
    float r;
#ifdef __arm__
    asm("vmaxnm.f32 %[d], %[n], %[m]" : [d] "=t"(r) : [n] "t"(a), [m] "t"(b) :);
#else
    r = (a > b) ? a : b;
#endif // __arm__
    return r;
}

inline float sig_clamp(float value, float min, float max) {
    return sig_fminf(sig_fmaxf(value, min), max);
}

inline float sig_snapToInteger(float value, float tolerance) {
    float nearest = roundf(value);

    return fabsf(value - nearest) <= tolerance ? nearest : value;
}

inline float sig_fastMod1f(float x) {
    int32_t ix = (int32_t) x;
    float r = x - (float) ix;
    return r + (r < 0.0f);
}

inline float sig_flooredfmodf(float numer, float denom) {
    float remain = fmodf(numer, denom);
    if ((remain > 0.0f && denom < 0.0f) ||
        (remain < 0.0f && denom > 0.0f)) {
        remain = remain + denom;
    }

    return remain;
}

inline float sig_fastTanhf(float x) {
    // From https://gist.github.com/ndonald2/534831b639b8c78d40279b5007e06e5b
    if (x > 3.0f) {
        return 1.0f;
    }

    if (x < -3.0f) {
        return -1.0f;
    }

    float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

inline float sig_linearMap(float value,
    float fromMin, float fromMax, float toMin, float toMax) {
    if (fromMin == fromMax) {
        return toMin;
    }

    float clamped = sig_clamp(value, fromMin, fromMax);
    float mapped = (clamped - fromMin) * (toMax - toMin) /
        (fromMax - fromMin) + toMin;

    return mapped;
}

inline uint16_t sig_unipolarToUint12(float sample) {
    return (uint16_t) (sample * 4095.0f);
}

inline uint16_t sig_bipolarToUint12(float sample) {
    float normalized = sample * 0.5 + 0.5;
    return (uint16_t) (normalized * 4095.0f);
}

inline uint16_t sig_bipolarToInvUint12(float sample) {
    return sig_bipolarToUint12(-sample);
}

inline float sig_uint16ToBipolar(uint16_t sample) {
    float normalized = (float) (sample / 65535.0f);
    float scaled = normalized * 2.0f - 1.0f;

    return scaled;
}

inline float sig_uint16ToUnipolar(uint16_t sample) {
    float normalized = (float) (sample / 65535.0f);

    return normalized;
}

inline float sig_invUint16ToBipolar(uint16_t sample) {
    return -sig_uint16ToBipolar(sample);
}

// TODO: Unit tests.
inline float sig_sum(float_array_ptr values,
    size_t length) {
    float sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += FLOAT_ARRAY(values)[i];
    }

    return sum;
}

// TODO: Unit tests.
inline size_t sig_indexOfMin(float_array_ptr values,
    size_t length) {
    size_t indexOfMin = 0;

    if (length < 1) {
        return indexOfMin;
    }

    float minValue = FLOAT_ARRAY(values)[0];
    for (size_t i = 1; i < length; i++) {
        float currentValue = FLOAT_ARRAY(values)[i];
        if (currentValue < minValue) {
            indexOfMin = i;
            minValue = currentValue;
        }
    }

    return indexOfMin;
}

// TODO: Unit tests.
inline size_t sig_indexOfMax(float_array_ptr values,
    size_t length) {
    size_t indexOfMax = 0;

    if (length < 1) {
        return indexOfMax;
    }

    float maxValue = FLOAT_ARRAY(values)[0];
    for (size_t i = 1; i < length; i++) {
        float currentValue = FLOAT_ARRAY(values)[i];
        if (currentValue > maxValue) {
            indexOfMax = i;
            maxValue = currentValue;
        }
    }

    return indexOfMax;
}

// TODO: Unit tests.
inline float sig_filter_mean(float_array_ptr values,
    size_t length) {
    float sum = sig_sum(values, length);
    return sum / (float) length;
}

// TODO: Unit tests.
inline float sig_filter_meanExcludeMinMax(
    float_array_ptr values, size_t length) {
    if (length < 1) {
        return 0.0f;
    } else if (length < 3) {
        return sig_filter_mean(values, length);
    }

    float min = FLOAT_ARRAY(values)[sig_indexOfMin(values, length)];
    float max = FLOAT_ARRAY(values)[sig_indexOfMax(values, length)];
    float exclude = min + max;
    float sum = sig_sum(values, length) - exclude;
    return sum / (float) (length - 2);
}

// TODO: Unit tests.
inline float sig_filter_ema(float current, float previous,
    float a) {
    return (a * current) + (1 - a) * previous;
}

inline float sig_filter_onepole(float current,
    float previous, float b0, float a1) {
    return b0 * current - a1 * previous;
}

inline float sig_filter_onepole_HPF_calculateA1(
    float frequency, float sampleRate) {
    float fc = frequency / sampleRate;
    return -expf(-2.0f * sig_PI * (0.5f - fc));
}

inline float sig_filter_onepole_HPF_calculateB0(float a1) {
    return 1.0f + a1;
}

inline float sig_filter_onepole_LPF_calculateA1(
    float frequency, float sampleRate) {
    float fc = frequency / sampleRate;
    return expf(-2.0f * sig_PI * fc);
}

inline float sig_filter_onepole_LPF_calculateB0(float a1) {
    return 1.0f - a1;
}

inline float sig_filter_smooth(float current,
    float previous, float coeff) {
    return current + coeff * (previous - current);
}

inline float sig_filter_smooth_calculateCoefficient(
    float time, float sampleRate) {
    return expf(sig_LOG0_001 / (time * sampleRate));
}

inline float sig_filter_dcBlock(float current,
    float previousInput, float previousOutput, float r) {
    return current - previousInput + r * previousOutput;
}

inline float sig_filter_dcBlock_calculateCoefficient(
    float frequency, float sampleRate) {
    return 1.0f - (sig_TWOPI * frequency / sampleRate);
}

inline float sig_filter_svf_highPass(float input, float band,
    float low, float k) {
    return input - k * band - low;
}

inline float sig_filter_svf_notch(float input, float band,
    float low, float k) {
    return input - k * band;
}

inline float sig_filter_svf_peak(float input, float band,
    float low, float k) {
    return input - k * band - 2.0f * low;
}

inline float sig_filter_svf_allPass(float input, float band,
    float low, float k) {
    return input - 2.0f * k * band;
}

inline float sig_filter_Smooth_generate(
    struct sig_filter_Smooth* self, float value) {
    float smoothed = sig_filter_smooth(value, self->previous, self->coeff);
    self->previous = smoothed;

    return smoothed;
}

inline float sig_filter_DCBlock_generate(
    struct sig_filter_DCBlock* self, float value) {
    float output = sig_filter_dcBlock(value,
        self->previousInput, self->previousOutput, self->r);
    self->previousInput = value;
    self->previousOutput = output;

    return output;
}

inline void sig_filter_SVF_calculateCoefficients(
    struct sig_filter_SVF* self, float frequency, float resonance,
    float sampleRate) {
    // Clamp the cutoff below the frequency at which floating point rounding of
    // pi pushes the tangent past its pole, which flips the coefficients' sign.
    frequency = sig_clamp(frequency, 0.0f, sampleRate * 0.49f);
    resonance = sig_clamp(resonance, 0.0f, 1.0f);

    float g = tanf(sig_PI * frequency / sampleRate);
    // A minimum damping keeps the filter bounded when a resonance control
    // reaches full scale.
    float k = sig_fmaxf(2.0f - 2.0f * resonance, 0.01f);

    self->k = k;
    self->a1 = 1.0f / (1.0f + g * (g + k));
    self->a2 = g * self->a1;
    self->a3 = g * self->a2;
}

inline void sig_filter_SVF_generate(struct sig_filter_SVF* self,
    float value, struct sig_filter_SVF_Taps* taps) {
    float v3 = value - self->ic2eq;
    float band = self->a1 * self->ic1eq + self->a2 * v3;
    float low = self->ic2eq + self->a2 * self->ic1eq + self->a3 * v3;
    self->ic1eq = 2.0f * band - self->ic1eq;
    self->ic2eq = 2.0f * low - self->ic2eq;

    taps->input = value;
    taps->band = band;
    taps->low = low;
}

inline void sig_filter_Ladder_calculateCoefficients(
    struct sig_filter_Ladder* self, float frequency, float sampleRate) {
    frequency = sig_clamp(frequency, 5.0f, sampleRate * 0.425f);
    float wc = frequency * (float) (sig_TWOPI /
        ((float) self->interpolation * sampleRate));
    float wc2 = wc * wc;
    self->alpha = 0.9892f * wc - 0.4324f *
        wc2 + 0.1381f * wc * wc2 - 0.0202f * wc2 * wc2;
    self->qAdjust = 1.006f + 0.0536f * wc - 0.095f * wc2 - 0.05f * wc2 * wc2;
}

inline float sig_filter_Ladder_calcStage(
    struct sig_filter_Ladder* self, float s, uint8_t i) {
    float ft = s * (1.0f/1.3f) + (0.3f/1.3f) * self->z0[i] - self->z1[i];
    ft = ft * self->alpha + self->z1[i];
    self->z1[i] = ft;
    self->z0[i] = s;
    return ft;
}

inline float sig_filter_Ladder_calcStageInput(
    struct sig_filter_Ladder* self, float value, float interp,
    float resonance, float passbandGain) {
    // Interpolate from the previous input sample toward the current one over
    // the sub-steps, so the input stays continuous across sample boundaries.
    float inInterp = (1.0f - interp) * self->previousInput + interp * value;
    float k = 4.0f * resonance;

    return inInterp - (self->z1[3] - passbandGain * inInterp) *
        k * self->qAdjust;
}

inline void sig_filter_Ladder_calcStages(
    struct sig_filter_Ladder* self, float u,
    struct sig_filter_Ladder_Taps* taps) {
    float stage1 = sig_filter_Ladder_calcStage(self, u, 0);
    float stage2 = sig_filter_Ladder_calcStage(self, stage1, 1);
    float stage3 = sig_filter_Ladder_calcStage(self, stage2, 2);
    float stage4 = sig_filter_Ladder_calcStage(self, stage3, 3);

    taps->input = u;
    taps->pole1 = stage1;
    taps->pole2 = stage2;
    taps->pole3 = stage3;
    taps->pole4 = stage4;
}

inline void sig_filter_Ladder_generate(struct sig_filter_Ladder* self,
    float value, float resonance, float passbandGain,
    struct sig_filter_Ladder_Taps* taps) {
    const uint8_t interpolation = self->interpolation;
    const float interpolationRecip = self->interpolationRecip;
    float totals[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float interp = 0.0f;

    for (size_t os = 0; os < interpolation; os++) {
        float u = sig_filter_Ladder_calcStageInput(self, value, interp,
            resonance, passbandGain);
        u = sig_fastTanhf(u);

        struct sig_filter_Ladder_Taps step;
        sig_filter_Ladder_calcStages(self, u, &step);
        totals[0] = step.input;
        totals[1] += step.pole1 * interpolationRecip;
        totals[2] += step.pole2 * interpolationRecip;
        totals[3] += step.pole3 * interpolationRecip;
        totals[4] += step.pole4 * interpolationRecip;
        interp += interpolationRecip;
    }
    self->previousInput = value;

    taps->input = totals[0];
    taps->pole1 = totals[1];
    taps->pole2 = totals[2];
    taps->pole3 = totals[3];
    taps->pole4 = totals[4];
}

inline float sig_osc_Oscillator_eoc(float phase) {
    return phase < 0.0f || phase > 1.0f ? 1.0f : 0.0f;
}

inline float sig_osc_Oscillator_wrapPhase(float phase) {
    return sig_fastMod1f(phase);
}

inline void sig_osc_Oscillator_accumulatePhase(
    float* phaseAccumulator, float frequency, float sampleRate) {
    // Division is slow, so the frequency is multiplied by the sample
    // period. When this is called in a loop, the compiler calculates the
    // sample period once, outside the loop.
    float phaseStep = frequency * (1.0f / sampleRate);
    float phase = *phaseAccumulator + phaseStep;

    // The phase usually stays within 0.0-1.0, where wrapping would return
    // it unchanged, so it is only wrapped when it leaves that range.
    if (phase >= 1.0f || phase < 0.0f) {
        phase = sig_osc_Oscillator_wrapPhase(phase);
    }

    *phaseAccumulator = phase;
}

inline float sig_osc_Wavetable_generate(
    struct sig_osc_Wavetable* self, float frequency,
    float phaseOffset, float sampleRate, float* eocOut) {
    float modulatedPhase = self->phaseAccumulator + phaseOffset;
    *eocOut = sig_osc_Oscillator_eoc(modulatedPhase);
    modulatedPhase = sig_osc_Oscillator_wrapPhase(modulatedPhase);
    float sample = sig_Buffer_readLinearAtPhase(self->wavetable,
        modulatedPhase);
    sig_osc_Oscillator_accumulatePhase(&self->phaseAccumulator,
        frequency, sampleRate);

    return sample;
}

inline float sig_osc_WavetableBank_generate(
    struct sig_osc_WavetableBank* self,
    float frequency, float phaseOffset, float tableIndex,
    float sampleRate, float* eocOut) {
        // TODO: Precompute this.
        float numTables = (float) self->wavetables->length;
        // TODO: Reduce duplication with sig_osc_Wavetable
        float modulatedPhase = self->phaseAccumulator + phaseOffset;
        *eocOut = sig_osc_Oscillator_eoc(modulatedPhase);
        modulatedPhase = sig_fastMod1f(modulatedPhase);
        float scaledTableIdx = tableIndex * numTables;
        float sample = sig_WavetableBank_readLinearAtPhase(
            self->wavetables, scaledTableIdx, modulatedPhase);
        sig_osc_Oscillator_accumulatePhase(&self->phaseAccumulator,
            frequency, sampleRate);

        return sample;
}

inline float sig_osc_AdditiveWavetableBank_readAtPhase(
    struct sig_osc_AdditiveWavetableBank* self, float frequency, float phase,
    float tableIndex) {
    float waveIdx = tableIndex * (float) self->wavetables->numShapes;

    // Logarithms are expensive, so the level is only
    // recalculated when the frequency changes.
    if (frequency != self->previousFrequency) {
        self->level = sig_AdditiveWavetableBank_levelForFrequency(
            self->wavetables, frequency);
        self->previousFrequency = frequency;
    }

    return sig_AdditiveWavetableBank_readLinearAtPhase(self->wavetables,
        self->level, waveIdx, phase);
}

inline float sig_osc_AdditiveWavetableBank_generate(
    struct sig_osc_AdditiveWavetableBank* self, float frequency,
    float phaseOffset, float tableIndex, float sampleRate, float* eocOut) {
    float modulatedPhase = self->phaseAccumulator + phaseOffset;
    *eocOut = sig_osc_Oscillator_eoc(modulatedPhase);
    float sample = sig_osc_AdditiveWavetableBank_readAtPhase(self, frequency,
        sig_osc_Oscillator_wrapPhase(modulatedPhase), tableIndex);
    sig_osc_Oscillator_accumulatePhase(&self->phaseAccumulator,
        frequency, sampleRate);

    return sample;
}

inline float sig_osc_RatioPhasor_generate(struct sig_osc_RatioPhasor* self,
    float reference, float numerator, float denominator, float octaves) {
    if (denominator <= 0.0f) {
        denominator = 1.0f;
    }

    // Transposing by whole octaves multiplies the numerator when going up
    // and the denominator when going down, so a ratio of whole numbers
    // stays a ratio of whole numbers.
    if (octaves == roundf(octaves)) {
        if (octaves > 0.0f) {
            numerator = ldexpf(numerator, (int) octaves);
        } else if (octaves < 0.0f) {
            denominator = ldexpf(denominator, (int) -octaves);
        }
    } else {
        numerator *= exp2f(octaves);
    }

    // A reference that moves by close to a whole cycle has wrapped, so
    // removing the whole cycles leaves its actual increment.
    float delta = reference - self->previousReference;
    float wraps = roundf(delta);
    delta -= wraps;
    self->previousReference = reference;

    // Only whole denominators lock, and the cycle count needs at least one
    // cycle per period, so other denominators count a period of one cycle.
    // The largest float below 2^32 keeps the conversion in range.
    const uint32_t periodCycles = denominator >= 1.0f &&
        denominator <= 4294967040.0f ? (uint32_t) denominator : 1;
    uint32_t cycle = self->cycle;
    // Division is slow, so the cycle is only wrapped when the denominator
    // has decreased below it.
    if (cycle >= periodCycles) {
        cycle %= periodCycles;
    }
    if (wraps < 0.0f) {
        cycle = (cycle + 1) % periodCycles;
    } else if (wraps > 0.0f) {
        cycle = (cycle + periodCycles - 1) % periodCycles;
    }
    self->cycle = cycle;

    float ratio = numerator / denominator;
    float increment = delta * ratio;
    bool isLimited = fabsf(increment) > self->maxIncrement;
    if (isLimited) {
        increment = copysignf(self->maxIncrement, increment);
    }
    float phase = self->phase + increment;

    if (!isLimited && numerator == roundf(numerator) &&
        denominator == roundf(denominator)) {
        float locked = ((float) cycle + reference) * ratio;
        float error = locked - phase;
        error -= roundf(error);
        phase += error * self->correction;
    }

    // The phase usually stays within 0.0-1.0, where wrapping would return
    // it unchanged, so it is only wrapped when it leaves that range.
    if (phase >= 1.0f || phase < 0.0f) {
        phase = sig_osc_Oscillator_wrapPhase(phase);
    }
    self->phase = phase;

    return phase;
}

inline float sig_osc_PeakFrequency_generate(
    struct sig_osc_PeakFrequency* self, float phase, float sampleRate) {
    // A phase that moves by close to a whole cycle has wrapped, so removing
    // the whole cycles leaves its actual movement.
    float delta = phase - self->previousPhase;
    delta -= roundf(delta);
    float frequency = self->hasPreviousPhase ?
        fabsf(delta) * sampleRate : 0.0f;
    self->previousPhase = phase;
    self->hasPreviousPhase = true;

    float releasedPeak = self->peak * self->release;
    self->peak = frequency > releasedPeak ? frequency : releasedPeak;

    return self->peak;
}

inline float sig_osc_ClockPhasor_generate(struct sig_osc_ClockPhasor* self,
    float pulse, float threshold) {
    self->samplesSinceLastPulse++;

    if (pulse >= threshold && self->previousPulse < threshold) {
        if (self->hasPulsed) {
            float period = (float) self->samplesSinceLastPulse;
            float error = roundf(self->phase) - self->phase;
            self->increment = 1.0f / period;
            self->correction = error / period;
            self->correctionSamples = self->samplesSinceLastPulse;
        }

        self->hasPulsed = true;
        self->samplesSinceLastPulse = 0;
    }
    self->previousPulse = pulse;

    float phase = self->phase;
    float increment = self->increment;
    if (self->correctionSamples > 0) {
        increment += self->correction;
        self->correctionSamples--;
    }
    self->phase = sig_osc_Oscillator_wrapPhase(phase + increment);

    return phase;
}

inline void sig_osc_FastLFSine_setFrequency(
    struct sig_osc_FastLFSine* self, float frequency) {
    self->f = 2.0f * sinf(sig_PI * frequency / self->sampleRate);
}

inline void sig_osc_FastLFSine_setFrequencyFast(
    struct sig_osc_FastLFSine* self, float frequency) {
    self->f = sig_TWOPI * frequency / self->sampleRate;
}

inline void sig_osc_FastLFSine_generate(struct sig_osc_FastLFSine* self) {
    // TODO: Oversample for stability at higher frequencies.
    // Depending on the Q setting, but stability is roughly 1/6
    // of the sample rate.
    // A simple way to do this is to run the filter twice with the same input sample, and discard one output sample.
    self->sinZ = self->sinZ + self->f * self->cosZ;
    self->cosZ = self->cosZ - self->f * self->sinZ;
}

inline void sig_Status_reportResult(
    struct sig_Status* status, enum sig_Result result) {
    if (status != NULL) {
        status->result = result;
    }
}

inline float sig_Buffer_readLinearAtPhase(
    struct sig_Buffer* self,
    float phase) {
    size_t tableLength = self->length;
    float tableIdx = phase * tableLength;

    return sig_interpolate_linear(tableIdx, self->samples, tableLength);
}

inline float sig_WavetableBank_readLinearAtPhase(
    struct sig_WavetableBank* self,
    float tableIdx, float phase) {
        const size_t length = self->length;
        const float lengthF = (float) length;

        // Wrapping requires a division, so it is skipped for the
        // usual case where the index is already within the bank.
        if (tableIdx < 0.0f || tableIdx >= lengthF) {
            tableIdx = sig_flooredfmodf(tableIdx, lengthF);
        }

        size_t tableIdxIntegral = (size_t) tableIdx;
        float tableIdxFractional = tableIdx - (float) tableIdxIntegral;

        // Tiny negative indices can wrap to exactly the length,
        // which is the start of the bank.
        size_t aTableIndex = tableIdxIntegral % length;
        struct sig_Buffer* aTable = self->waves[aTableIndex];
        size_t bTableIndex = (aTableIndex + 1) % length;
        struct sig_Buffer* bTable = self->waves[bTableIndex];

        float a = sig_Buffer_readLinearAtPhase(aTable, phase);
        float b = sig_Buffer_readLinearAtPhase(bTable, phase);
        float sample = a + (b - a) * tableIdxFractional;

        return sample;
}

inline float sig_AdditiveWavetableBank_levelForFrequency(
    struct sig_AdditiveWavetableBank* self, float frequency) {
    float highestLevel = (float) (self->numLevels - 1);
    float level = log2f(fabsf(frequency) / self->lowestFrequency);

    return sig_clamp(level, 0.0f, highestLevel);
}

inline float sig_AdditiveWavetableBank_readLinearAtPhase(
    struct sig_AdditiveWavetableBank* self, float level, float tableIdx,
    float phase) {
    size_t lowerLevel = (size_t) level;
    size_t upperLevel = lowerLevel + 1 < self->numLevels ?
        lowerLevel + 1 : lowerLevel;
    float levelMix = level - (float) lowerLevel;

    float lower = sig_WavetableBank_readLinearAtPhase(
        self->levels[lowerLevel], tableIdx, phase);
    float upper = sig_WavetableBank_readLinearAtPhase(
        self->levels[upperLevel], tableIdx, phase);

    return lower + (upper - lower) * levelMix;
}

inline float sig_DelayLine_readAt(
    struct sig_DelayLine* self, size_t readPos) {
    size_t idx = (self->writeIdx + readPos) % self->buffer->length;
    return FLOAT_ARRAY(self->buffer->samples)[idx];
}

inline float sig_DelayLine_linearReadAt(
    struct sig_DelayLine* self, float readPos) {
    size_t maxDelayLength = self->buffer->length;
    float* delayLineSamples = FLOAT_ARRAY(self->buffer->samples);
    int32_t integ = (int32_t) readPos;
    float frac = readPos - (float) integ;
    float a = delayLineSamples[(self->writeIdx + integ) % maxDelayLength];
    float b = delayLineSamples[(self->writeIdx + integ + 1) % maxDelayLength];

    return a + (b - a) * frac;
}

inline float sig_DelayLine_cubicReadAt(
    struct sig_DelayLine* self, float readPos) {
    size_t maxDelayLength = self->buffer->length;
    float* delayLineSamples = FLOAT_ARRAY(self->buffer->samples);

    int32_t integ = (int32_t) readPos;
    float frac = readPos - (float) integ;
    int32_t t = (int32_t) (self->writeIdx + integ + maxDelayLength);
    float xm1 = delayLineSamples[(t - 1) % maxDelayLength];
    float x0 = delayLineSamples[t % maxDelayLength];
    float x1 = delayLineSamples[(t + 1) % maxDelayLength];
    float x2 = delayLineSamples[(t + 2) % maxDelayLength];
    float c = (x1 - xm1) * 0.5f;
    float v = x0 - x1;
    float w = c + v;
    float a = w + v + (x2 - x0) * 0.5f;
    float bNeg = w + a;

    return (((a * frac) - bNeg) * frac + c) * frac + x0;
}

inline float sig_DelayLine_allpassReadAt(
    struct sig_DelayLine* self, float readPos,
    float previousSample) {
    size_t maxDelayLength = self->buffer->length;
    float* delayLineSamples = FLOAT_ARRAY(self->buffer->samples);
    int32_t integ = (int32_t) readPos;
    float frac = readPos - (float) integ;
    float invFrac = 1.0f - frac;
    float a = delayLineSamples[(self->writeIdx + integ) % maxDelayLength];
    float b = delayLineSamples[(self->writeIdx + integ + 1) % maxDelayLength];

    return b + invFrac * a - invFrac * previousSample;
}

inline float sig_DelayLine_readAtTime(
    struct sig_DelayLine* self, float source,
    float tapTime, float sampleRate) {
    sig_DelayLine_readAtTime_IMPL(self, source, tapTime, sampleRate,
        sig_DelayLine_readAt);
}

inline float sig_DelayLine_linearReadAtTime(
    struct sig_DelayLine* self, float source, float tapTime,
    float sampleRate) {
    sig_DelayLine_readAtTime_IMPL(self, source, tapTime, sampleRate,
        sig_DelayLine_linearReadAt);
}

inline float sig_DelayLine_cubicReadAtTime(
    struct sig_DelayLine* self, float source, float tapTime,
    float sampleRate) {
    sig_DelayLine_readAtTime_IMPL(self, source, tapTime, sampleRate,
        sig_DelayLine_cubicReadAt);
}

inline float sig_DelayLine_allpassReadAtTime(
    struct sig_DelayLine* self, float source, float tapTime,
    float sampleRate, float previousSample) {
    // TODO: Cut and pasted from the sig_DelayLine_readAtTime_IMPL macro above.
    float sample;
    if (tapTime <= 0.0f) {
        sample = source;
    } else {
        float readPos = tapTime * sampleRate;
        float maxDelayLength = (float) self->buffer->length;
        if (readPos >= maxDelayLength) {
            readPos = maxDelayLength - 1;
        }
        sample = sig_DelayLine_allpassReadAt(self, readPos, previousSample);
    }

    return sample;
}

inline float sig_DelayLine_readAtTimes(
    struct sig_DelayLine* self, float source,
    float_array_ptr SIG_RESTRICT tapTimes,
    float_array_ptr SIG_RESTRICT tapGains,
    size_t numTaps, float sampleRate, float timeScale) {
    sig_DelayLine_readAtTimes_IMPL(self, source, tapTimes,
        tapGains, numTaps, sampleRate, timeScale, sig_DelayLine_readAt);
}

inline float sig_DelayLine_linearReadAtTimes(
    struct sig_DelayLine* self, float source,
    float_array_ptr SIG_RESTRICT tapTimes,
    float_array_ptr SIG_RESTRICT tapGains,
    size_t numTaps, float sampleRate, float timeScale) {
    sig_DelayLine_readAtTimes_IMPL(self, source, tapTimes,
        tapGains, numTaps, sampleRate, timeScale, sig_DelayLine_linearReadAt);
}

inline float sig_DelayLine_cubicReadAtTimes(
    struct sig_DelayLine* self, float source,
    float_array_ptr SIG_RESTRICT tapTimes,
    float_array_ptr SIG_RESTRICT tapGains,
    size_t numTaps, float sampleRate, float timeScale) {
    sig_DelayLine_readAtTimes_IMPL(self, source, tapTimes,
        tapGains, numTaps, sampleRate, timeScale, sig_DelayLine_cubicReadAt);
}

inline void sig_DelayLine_write(struct sig_DelayLine* self,
    float sample) {
    size_t maxDelayLength = self->buffer->length;
    FLOAT_ARRAY(self->buffer->samples)[self->writeIdx] = sample;
    self->writeIdx = (self->writeIdx - 1 + maxDelayLength) % maxDelayLength;
}

inline float sig_DelayLine_calcFeedbackGain(float delayTime,
    float decayTime) {
    // Convert 60dB time in secs to feedback gain (g) coefficient
    // (also why is the equation in Dodge and Jerse wrong?)
    if (delayTime <= 0.0f || decayTime <= 0.0f) {
        return 0.0f;
    }

    return expf(sig_LOG0_001 * delayTime / decayTime);
}

inline float sig_DelayLine_feedback(float sample,
    float read, float g) {
    return sample + (g * read);
}

inline float sig_DelayLine_comb(struct sig_DelayLine* self,
    float sample, size_t readPos, float g) {
    sig_DelayLine_comb_IMPL(self, sample, readPos, g, sig_DelayLine_readAt);
}

inline float sig_DelayLine_linearComb(
    struct sig_DelayLine* self, float sample, float readPos,
    float g) {
    sig_DelayLine_comb_IMPL(self, sample, readPos, g,
        sig_DelayLine_linearReadAt);
}

inline float sig_DelayLine_cubicComb(
    struct sig_DelayLine* self, float sample, float readPos,
    float g) {
    sig_DelayLine_comb_IMPL(self, sample, readPos, g,
        sig_DelayLine_cubicReadAt);
}

inline float sig_DelayLine_allpass(
    struct sig_DelayLine* self, float sample, size_t readPos,
    float g) {
    sig_DelayLine_allpass_IMPL(self, sample, readPos, g, sig_DelayLine_readAt);
}

inline float sig_DelayLine_linearAllpass(
    struct sig_DelayLine* self, float sample, float readPos,
    float g) {
    sig_DelayLine_allpass_IMPL(self, sample, readPos, g,
        sig_DelayLine_linearReadAt);
}

inline float sig_DelayLine_cubicAllpass(
    struct sig_DelayLine* self, float sample, float readPos,
    float g) {
    sig_DelayLine_allpass_IMPL(self, sample, readPos, g,
        sig_DelayLine_cubicReadAt);
}

inline float sig_linearXFade(float left, float right,
    float mix) {
    float clipped = sig_clamp(mix, -1.0f, 1.0f);
    // At -1.0f, left gain should be 1.0 and right gain should be 0.0;
    // at 0.0, left and right gains should be 0.5;
    // At 1.0 left gain should be 0.0f and right gain should be 1.0.
    float gain = clipped * 0.5f + 0.5f;
    float sample = left + gain * (right - left);

    return sample;
}

inline float sig_sineWavefolder(float x, float gain,
    float factor) {
    float sample = x + gain * sinf(factor * x);
    return sample;
}

inline float sig_softClip(float x, float threshold) {
    threshold = sig_clamp(threshold, 0.0f, 0.99f);

    float magnitude = fabsf(x);
    if (magnitude <= threshold) {
        return x;
    }

    float headroom = 1.0f - threshold;
    float shaped = threshold +
        headroom * sig_fastTanhf((magnitude - threshold) / headroom);

    return (x < 0.0f) ? -shaped : shaped;
}

inline bool sig_tempo_OnsetDetector_hasNewValue(
    struct sig_tempo_OnsetDetector* self) {
    return self->hasNewValue;
}

inline void sig_tempo_OnsetDetector_setThreshold(
    struct sig_tempo_OnsetDetector* self, float threshold) {
    if (threshold == self->threshold) {
        return;
    }

    self->threshold = threshold;
    self->thresholdMagnitude = powf(10.0f, threshold / 20.0f);
}

inline void sig_tempo_TempoEstimator_setTempoRange(
    struct sig_tempo_TempoEstimator* self, float minTempo, float maxTempo) {
    if (minTempo > maxTempo) {
        float swap = minTempo;
        minTempo = maxTempo;
        maxTempo = swap;
    }

    self->minTempo = sig_clamp(minTempo, self->lowestTempo,
        self->highestTempo);
    self->maxTempo = sig_clamp(maxTempo, self->lowestTempo,
        self->highestTempo);
}

inline float sig_tempo_BeatTracker_follow(struct sig_tempo_BeatTracker* self) {
    self->sampleClock += 1.0;
    if (self->state == sig_tempo_BeatTracker_State_WAITING) {
        return 0.0f;
    }

    // The output chases the grid's beat position along the shorter way
    // around the beat, at a limited rate.
    double gridPosition = (self->sampleClock - self->anchor) / self->period;
    double gap = gridPosition - self->position;
    gap -= floor(gap + 0.5);
    double rate = 1.0 + sig_tempo_BeatTracker_CHASE_GAIN * gap;
    if (rate < sig_tempo_BeatTracker_MIN_RATE) {
        rate = sig_tempo_BeatTracker_MIN_RATE;
    } else if (rate > sig_tempo_BeatTracker_MAX_RATE) {
        rate = sig_tempo_BeatTracker_MAX_RATE;
    }
    self->position += rate / self->period;

    float phase = (float) (self->position - floor(self->position));

    return phase < 1.0f ? phase : 0.0f;
}

inline float sig_tempo_BeatTracker_tempo(struct sig_tempo_BeatTracker* self) {
    if (self->state == sig_tempo_BeatTracker_State_WAITING) {
        return 0.0f;
    }

    return (float) (60.0 * (double) self->sampleRate / self->period);
}

inline float sig_tempo_BeatTracker_confidence(
    struct sig_tempo_BeatTracker* self) {
    return self->state == sig_tempo_BeatTracker_State_TRACKING ?
        self->confidence : 0.0f;
}

inline enum sig_tempo_BeatTracker_State sig_tempo_BeatTracker_state(
    struct sig_tempo_BeatTracker* self) {
    return self->state;
}

inline void sig_tempo_BeatTracker_setResponsiveness(
    struct sig_tempo_BeatTracker* self, float responsiveness) {
    self->responsiveness = sig_clamp(responsiveness, 0.0f, 1.0f);
}

inline void sig_tempo_BeatTracker_setTempoRange(
    struct sig_tempo_BeatTracker* self, float minTempo, float maxTempo) {
    if (minTempo > maxTempo) {
        float swap = minTempo;
        minTempo = maxTempo;
        maxTempo = swap;
    }

    self->minTempo = sig_clamp(minTempo, self->lowestTempo,
        self->highestTempo);
    self->maxTempo = sig_clamp(maxTempo, self->lowestTempo,
        self->highestTempo);
}

inline void sig_tempo_BeatTracker_setSilenceTimeout(
    struct sig_tempo_BeatTracker* self, float silenceTimeout) {
    self->silenceTimeout = sig_clamp(silenceTimeout, 1.0f, 16.0f);
}

inline void sig_tempo_ClockGenerator_setLead(
    struct sig_tempo_ClockGenerator* self, float lead) {
    self->lead = sig_clamp(lead, 0.0f, 0.25f);
}

inline void sig_tempo_ClockGenerator_setTriggerDuration(
    struct sig_tempo_ClockGenerator* self, float triggerDuration) {
    self->triggerDuration = sig_clamp(triggerDuration, 0.001f, 0.1f);
    float samples = roundf(self->triggerDuration * self->sampleRate);
    self->triggerSamples = samples < 1.0f ? 1 : (uint32_t) samples;
}

inline void sig_tempo_ClockGenerator_setSilenceBehaviour(
    struct sig_tempo_ClockGenerator* self, float silenceBehaviour) {
    self->silenceBehaviour = silenceBehaviour;
}

inline void sig_tempo_ClockGenerator_generate(
    struct sig_tempo_ClockGenerator* self, float phase, float tempo,
    float confidence, enum sig_tempo_BeatTracker_State state,
    struct sig_tempo_ClockGenerator_Outputs* outputs) {
    bool isActive = state == sig_tempo_BeatTracker_State_TRACKING ||
        (state == sig_tempo_BeatTracker_State_SILENT &&
            self->silenceBehaviour >= 0.5f);
    float shiftedPhase = 0.0f;

    if (isActive) {
        shiftedPhase = phase + self->lead * tempo / 60.0f;
        shiftedPhase -= floorf(shiftedPhase);
        if (shiftedPhase >= 1.0f) {
            shiftedPhase = 0.0f;
        }

        // No trigger has fired for the current beat when the clock
        // becomes active.
        if (!self->wasActive ||
            (shiftedPhase >= sig_tempo_ClockGenerator_ARM_START &&
                shiftedPhase < sig_tempo_ClockGenerator_ARM_END)) {
            self->isArmed = true;
        }

        if (self->isArmed && self->previousPhase - shiftedPhase > 0.5f) {
            self->isArmed = false;
            self->triggerSamplesRemaining = self->triggerSamples;
        }
    } else {
        self->triggerSamplesRemaining = 0;
    }

    outputs->clock = self->triggerSamplesRemaining > 0 ? 1.0f : 0.0f;
    if (self->triggerSamplesRemaining > 0) {
        self->triggerSamplesRemaining--;
    }
    outputs->phase = shiftedPhase;
    outputs->bpm = tempo;
    outputs->confidence = confidence;

    self->previousPhase = shiftedPhase;
    self->wasActive = isActive;
}

#ifdef __cplusplus
}
#endif

#endif /* LIBSIGNALETIC_H */
