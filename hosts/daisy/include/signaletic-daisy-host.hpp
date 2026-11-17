#pragma once

#include <libsignaletic.h>
#include "daisy.h"
#include "stm32h7xx_hal.h"
// hal_map.h declares C functions without C linkage guards.
extern "C" {
#include "util/hal_map.h"
}
#include "signaletic-host.h"

namespace sig {
namespace libdaisy {

struct Normalization {
    float scale;
    float offset;
};

static constexpr struct Normalization NO_NORMALIZATION = {
    .scale = 1.0f,
    .offset = 0.0f
};

static constexpr struct Normalization UNI_TO_BIPOLAR = {
    .scale = 2.0f,
    .offset = -1.0f
};

static constexpr struct Normalization BI_TO_UNIPOLAR = {
    .scale = 0.5f,
    .offset = 0.5f
};

static constexpr struct Normalization INVERT = {
    .scale = -1.0f,
    .offset = 0.0f
};

static constexpr struct Normalization INV_UNI_TO_BIPOLAR = {
    .scale = -2.0f,
    .offset = 1.0f
};

static constexpr struct Normalization INV_BI_TO_UNIPOLAR = {
    .scale = -0.5f,
    .offset = 0.5f
};

struct ADCMuxSpec {
    uint8_t numMuxChannels;
    dsy_gpio_pin selA;
    dsy_gpio_pin selB;
    dsy_gpio_pin selC;
};

static constexpr struct ADCMuxSpec NO_MUX = {
    .numMuxChannels = 0,
    .selA = {DSY_GPIOX, 0},
    .selB = {DSY_GPIOX, 0},
    .selC = {DSY_GPIOX, 0}
};

struct ADCChannelSpec {
    dsy_gpio_pin pin;
    struct Normalization normalization;
    struct ADCMuxSpec mux;
};

template<typename T, size_t size> class InputBank {
    public:
        T* inputs;
        float values[size] = {0.0};

        inline void Read() {
            for (size_t i = 0; i < size; i++) {
                values[i] = inputs[i].Value();
            }
        }
};

template<typename T, size_t size> class OutputBank {
    public:
        T* outputs;
        float values[size];
        inline void Write() {
            for (size_t i = 0; i < size; i++) {
                outputs[i].Write(values[i]);
            }
        }
};

class BaseAnalogInput {
    public:
        uint16_t* adcPtr;
        float scale = 1.0f;
        float offset = 0.0f;

        void Init(daisy::AdcHandle* adc, ADCChannelSpec spec, int adcChannel) {
            adcPtr = adc->GetPtr(adcChannel);
            InitNormalization(spec);
        }

        void InitMux(daisy::AdcHandle* adc, ADCChannelSpec spec, int adcChannel,
            int muxChannel) {
            adcPtr = adc->GetMuxPtr(adcChannel, muxChannel);
            InitNormalization(spec);
        }

        void InitNormalization(ADCChannelSpec spec) {
            scale = spec.normalization.scale;
            offset = spec.normalization.offset;
        }
};

class AnalogInput : public BaseAnalogInput {
    public:
        /**
         * @brief Returns the value of the ADC channel as a float
         * scaled to a range of -1.0 to 1.0f.
         *
         * @return float the ADC channel's current value
         */
        inline float Value() {
            float converted = sig_uint16ToBipolar(*adcPtr);
            float normalized =  converted * scale + offset;
            return normalized;
        }
};

class UnipolarAnalogInput : public BaseAnalogInput {
    public:
        /**
         * @brief Returns the inverted value of the ADC channel as
         * a float scaled to the range of 0.0 to 1.0f.
         *
         * @return float the ADC channel's current value
         */
        inline float Value() {
            return sig_uint16ToUnipolar(*adcPtr);
        }
};

class InvertedAnalogInput : public BaseAnalogInput {
    public:
        /**
         * @brief Returns the inverted value of the ADC channel as
         * a float scaled to the range of -1.0 to 1.0f.
         *
         * @return float the ADC channel's current value
         */
        inline float Value() {
            float converted = sig_invUint16ToBipolar(*adcPtr);
            return converted;
        }
};

template<typename T, size_t numInputs>
    class ADCController {
    public:
        daisy::AdcHandle* adcHandle;
        T inputs[numInputs];
        InputBank<T, numInputs> channelBank;

        void Init(daisy::AdcHandle* inAdcHandle, ADCChannelSpec* channelSpecs,
            size_t numADCChannels) {
            adcHandle = inAdcHandle;

            daisy::AdcChannelConfig adcConfigs[numADCChannels];
            for (size_t i = 0; i < numADCChannels; i++) {
                ADCChannelSpec spec = channelSpecs[i];
                if (spec.mux.numMuxChannels > 0) {
                    adcConfigs[i].InitMux(spec.pin, spec.mux.numMuxChannels,
                        spec.mux.selA, spec.mux.selB, spec.mux.selC);
                } else {
                    adcConfigs[i].InitSingle(spec.pin);
                }
            }

            adcHandle->Init(adcConfigs, numADCChannels);

            size_t inputIdx = 0;
            for (size_t i = 0; i < numADCChannels; i++) {
                ADCChannelSpec spec = channelSpecs[i];
                if (spec.mux.numMuxChannels > 0) {
                    for (size_t j = 0; j < spec.mux.numMuxChannels; j++) {
                        inputs[inputIdx].InitMux(adcHandle, spec, i, j);
                        inputIdx++;
                    }
                } else {
                    inputs[inputIdx].Init(adcHandle, spec, i);
                    inputIdx++;
                }
            }

            channelBank.inputs = inputs;
        }

        void Start() {
            adcHandle->Start();
        }

        void Stop() {
            adcHandle->Stop();
        }

        inline void Read() {
            channelBank.Read();
        }
};

class BaseAnalogOutput {
    public:
        float scale = 1.0f;
        float offset = 0.0f;
};

class BufferedAnalogOutput : public BaseAnalogOutput{
    public:
        uint16_t* dacOutputs;
        size_t channel;

        void Init(uint16_t* inDACOutputs, size_t inChannel,
            struct Normalization normalization = NO_NORMALIZATION) {
            dacOutputs = inDACOutputs;
            channel = inChannel;
            scale = normalization.scale;
            offset = normalization.offset;
        }

        inline void Write(float value) {
            float normalizedValue = value * scale + offset;
            dacOutputs[channel] = sig_unipolarToUint12(normalizedValue);
        }
};

// TODO: Probably worth parameterizing the write strategy,
// though it will make for quite long templated definitions,
// e.g. OutputBank<AnalogOutput<BipolarInvertedWriter>> dacOutputBank;
class BipolarInvertedBufferedAnalogOutput : public BufferedAnalogOutput{
    public:
        inline void Write(float value) {
            float normalizedValue = value * scale + offset;
            dacOutputs[channel] = sig_bipolarToInvUint12(normalizedValue);
        }
};

class PollingAnalogOutput : public BaseAnalogOutput {
    public:
        daisy::DacHandle* dac;
        size_t channel;

        void Init(daisy::DacHandle* inDAC, size_t inChannel,
            struct Normalization normalization = NO_NORMALIZATION) {
            dac = inDAC;
            channel = inChannel;
            scale = normalization.scale;
            offset = normalization.offset;
        }

        inline void Write(float value) {
            float normalizedValue = value * scale + offset;
            uint16_t convertedValue = sig_unipolarToUint12(normalizedValue);
            dac->WriteValue(static_cast<daisy::DacHandle::Channel>(channel),
                convertedValue);
        }
};

class GPIO {
    public:
        dsy_gpio gpio;

        void Init(dsy_gpio_pin pin, dsy_gpio_mode mode, dsy_gpio_pull pull) {
            gpio.pin = pin;
            gpio.mode = mode;
            gpio.pull = pull;

            dsy_gpio_init(&gpio);
        }
};

class GPIOInput : public GPIO {
    public:
        inline float Value() {
            return (float) dsy_gpio_read(&gpio);
        }
};

class GPIOOutput : public GPIO {
    public:
        inline void Write(float value) {
            dsy_gpio_write(&gpio, (uint8_t) value);
        }
};


class GateInput {
    public:
        GPIOInput gpioSource;

        void Init(dsy_gpio_pin pin) {
            gpioSource.Init(pin, DSY_GPIO_MODE_INPUT, DSY_GPIO_PULLUP);
        }

        inline float Value() {
            return !gpioSource.Value();
        }
};

class EdgeTimedGateInput;

/**
 * @brief The edge-timed gate inputs that handle each pin interrupt line,
 * indexed by pin number.
 *
 */
extern EdgeTimedGateInput* edgeTimedGateInputs[16];

/**
 * @brief A gate input that places each of its edges on the sample at which
 * the edge occurred.
 *
 * A gate input that is read once per block can only detect an edge at the
 * start of the following block, which makes the timing of clock pulses
 * jitter by up to a block. Instead, a pin interrupt records the time of each
 * edge, and Read() places each edge on the matching sample of the next
 * block. The gate is delayed by one block, without jitter.
 *
 * The pin interrupt has to be able to interrupt the audio callback, so
 * Init() lowers the priority of the audio DMA interrupts below it.
 *
 * Slow or noisy edges can switch the pin several times in quick
 * succession. The gate ignores any transition that occurs within the
 * holdoff time of the previous one.
 */
class EdgeTimedGateInput {
    public:
        /**
         * @brief The largest block size, in samples.
         *
         */
        static const size_t MAX_BLOCK_SIZE = 256;

        /**
         * @brief The number of edges that can be recorded between reads.
         *
         */
        static const size_t MAX_EDGES = 64;

        /**
         * @brief The gate's samples for the most recently read block.
         *
         * 1.0 while the gate is high, and 0.0 while it is low.
         */
        float samples[MAX_BLOCK_SIZE] = {0.0f};

        /**
         * @brief The gate's value at the end of the most recently read block.
         *
         */
        float state = 0.0f;

        /**
         * @brief Initialises the gate's pin and its interrupt.
         *
         * @param inPin the gate's pin, which is inverted, as on the
         * Patch SM's gate inputs
         * @param holdoff the time after each transition during which the
         * gate ignores further transitions, in seconds
         */
        void Init(dsy_gpio_pin inPin, float holdoff = 0.0005f) {
            gpio.pin = inPin;
            gpio.mode = DSY_GPIO_MODE_INPUT;
            gpio.pull = DSY_GPIO_PULLUP;
            dsy_gpio_init(&gpio);

            holdoffTicks = (uint32_t) (holdoff *
                (float) daisy::System::GetTickFreq());
            edgeTimedGateInputs[inPin.pin] = this;

            __HAL_RCC_SYSCFG_CLK_ENABLE();
            GPIO_InitTypeDef init = {};
            init.Pin = dsy_hal_map_get_pin(&gpio.pin);
            init.Mode = GPIO_MODE_IT_RISING_FALLING;
            init.Pull = GPIO_PULLUP;
            init.Speed = GPIO_SPEED_FREQ_LOW;
            HAL_GPIO_Init(dsy_hal_map_get_port(&gpio.pin), &init);

            LowerAudioInterruptPriority();
            IRQn_Type irq = InterruptForLine(inPin.pin);
            HAL_NVIC_SetPriority(irq, 0, 0);
            HAL_NVIC_EnableIRQ(irq);
        }

        /**
         * @brief Records the time and level of an edge.
         *
         * Called from the pin's interrupt handler.
         */
        inline void OnEdge() {
            uint32_t tick = daisy::System::GetTick();
            uint32_t index = writeIndex;
            edgeTicks[index % MAX_EDGES] = tick;
            // The gate input is inverted, so a low pin is a high gate.
            edgeIsHigh[index % MAX_EDGES] = !dsy_gpio_read(&gpio);
            writeIndex = index + 1;
        }

        /**
         * @brief Fills the gate's samples for the next block from the edges
         * that occurred during the previous block.
         *
         * Called at the start of each audio callback.
         *
         * @param blockSize the number of samples in the block, up to
         * MAX_BLOCK_SIZE
         */
        void Read(size_t blockSize) {
            if (blockSize > MAX_BLOCK_SIZE) {
                blockSize = MAX_BLOCK_SIZE;
            }

            bool isPinHigh = !dsy_gpio_read(&gpio);
            uint32_t now = daisy::System::GetTick();

            if (!hasPreviousBlockTick) {
                previousBlockTick = now;
                hasPreviousBlockTick = true;
                readIndex = writeIndex;
                SetState(isPinHigh, now);
                FillSamples(0, blockSize);
                return;
            }

            uint32_t blockTicks = now - previousBlockTick;
            uint32_t endIndex = writeIndex;
            if (endIndex - readIndex > MAX_EDGES) {
                readIndex = endIndex - MAX_EDGES;
            }

            size_t position = 0;
            while (readIndex != endIndex) {
                uint32_t tick = edgeTicks[readIndex % MAX_EDGES];
                uint32_t ticksIntoBlock = tick - previousBlockTick;
                // Edges that occurred after the start of this callback
                // belong to the next block.
                if (ticksIntoBlock > blockTicks) {
                    break;
                }

                bool isHigh = edgeIsHigh[readIndex % MAX_EDGES];
                readIndex++;
                if (tick - lastTransitionTick < holdoffTicks ||
                    isHigh == (state > 0.0f)) {
                    continue;
                }

                size_t edgePosition = SampleForTick(ticksIntoBlock,
                    blockTicks, blockSize);
                FillSamples(position, edgePosition);
                position = edgePosition;
                SetState(isHigh, tick);
            }

            FillSamples(position, blockSize);

            // A transition ignored during the holdoff leaves the gate in the
            // wrong state, so it switches at the end of the block instead.
            if (isPinHigh != (state > 0.0f) &&
                now - lastTransitionTick >= holdoffTicks) {
                SetState(isPinHigh, now);
                samples[blockSize - 1] = state;
            }

            previousBlockTick = now;
        }

        /**
         * @brief Lowers the priority of libDaisy's audio DMA interrupts,
         * so that pin interrupts can interrupt the audio callback.
         *
         */
        static void LowerAudioInterruptPriority() {
            HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 1, 0);
            HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 1, 0);
            HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 1, 0);
            HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 1, 0);
        }

        /**
         * @brief Returns the interrupt that handles a pin interrupt line.
         *
         * @param line the pin interrupt line, which is the pin's number
         * @return the interrupt
         */
        static IRQn_Type InterruptForLine(uint8_t line) {
            static const IRQn_Type LOW_LINE_INTERRUPTS[5] = {
                EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn
            };

            if (line < 5) {
                return LOW_LINE_INTERRUPTS[line];
            }

            return line < 10 ? EXTI9_5_IRQn : EXTI15_10_IRQn;
        }

    private:
        /**
         * @brief The gate's pin.
         *
         */
        dsy_gpio gpio;

        /**
         * @brief The holdoff time, in system ticks.
         *
         */
        uint32_t holdoffTicks = 0;

        /**
         * @brief The time of the gate's most recent transition, in system
         * ticks.
         *
         */
        uint32_t lastTransitionTick = 0;

        /**
         * @brief The time at which the previous block was read, in system
         * ticks.
         *
         */
        uint32_t previousBlockTick = 0;

        /**
         * @brief Whether a block has been read yet.
         *
         */
        bool hasPreviousBlockTick = false;

        /**
         * @brief The time of each recorded edge, in system ticks.
         *
         * A ring buffer, written by the interrupt handler.
         */
        volatile uint32_t edgeTicks[MAX_EDGES] = {0};

        /**
         * @brief Whether the gate was high after each recorded edge.
         *
         */
        volatile bool edgeIsHigh[MAX_EDGES] = {false};

        /**
         * @brief The number of edges recorded by the interrupt handler.
         *
         */
        volatile uint32_t writeIndex = 0;

        /**
         * @brief The number of recorded edges that Read() has processed.
         *
         */
        uint32_t readIndex = 0;

        inline void SetState(bool isHigh, uint32_t tick) {
            state = isHigh ? 1.0f : 0.0f;
            lastTransitionTick = tick;
        }

        inline void FillSamples(size_t start, size_t end) {
            for (size_t i = start; i < end; i++) {
                samples[i] = state;
            }
        }

        static inline size_t SampleForTick(uint32_t ticksIntoBlock,
            uint32_t blockTicks, size_t blockSize) {
            if (blockTicks == 0) {
                return 0;
            }

            size_t sample = (size_t) (((uint64_t) ticksIntoBlock *
                (uint64_t) blockSize) / (uint64_t) blockTicks);

            return sample < blockSize ? sample : blockSize - 1;
        }
};

class Toggle {
    public:
        uint32_t previousDebounce;
        uint8_t state;
        dsy_gpio gpio;

        void Init(dsy_gpio_pin pin) {
            previousDebounce = daisy::System::GetNow();
            state = 0x00;
            gpio.pin = pin;
            gpio.mode = DSY_GPIO_MODE_INPUT;
            gpio.pull = DSY_GPIO_PULLUP;
            dsy_gpio_init(&gpio);
        }

        inline float Value() {
            // See https://www.ganssle.com/debouncing-pt2.htm
            // for a good overview of debouncing algorithms.
            uint32_t now = daisy::System::GetNow();
            if (now - previousDebounce >= 1) {
                state = (state << 1) | !dsy_gpio_read(&gpio);
                previousDebounce = now;
            }
            return (float) state == 0xff;
        }
};

class TriSwitch {
    public:
        Toggle switchA;
        Toggle switchB;

        void Init(dsy_gpio_pin pins[2]) {
            switchA.Init(pins[0]);
            switchB.Init(pins[1]);
        }

        void Init(dsy_gpio_pin pinA, dsy_gpio_pin pinB) {
            switchA.Init(pinA);
            switchB.Init(pinB);
        }

        inline float Value() {
            return switchA.Value() + -switchB.Value();
        }
};

class Encoder {
    public:
        uint32_t previousDebounce;
        uint8_t a;
        uint8_t b;
        dsy_gpio gpioA;
        dsy_gpio gpioB;

        void Init(dsy_gpio_pin pinA, dsy_gpio_pin pinB) {
            gpioA.pin = pinA;
            gpioA.mode = DSY_GPIO_MODE_INPUT;
            gpioA.pull = DSY_GPIO_PULLUP;

            gpioB.pin = pinB;
            gpioB.mode = DSY_GPIO_MODE_INPUT;
            gpioB.pull = DSY_GPIO_PULLUP;

            dsy_gpio_init(&gpioA);
            dsy_gpio_init(&gpioB);

            previousDebounce = daisy::System::GetNow();
            a = 255;
            b = 255;
        }

        inline float Value() {
            uint32_t now = daisy::System::GetNow();
            float increment = 0.0f;

            if (now - previousDebounce >= 1) {
                a = (a << 1) | dsy_gpio_read(&gpioA);
                b = (b << 1) | dsy_gpio_read(&gpioB);
                uint8_t aMasked = a & 3;
                uint8_t bMasked = b & 3;

                if (aMasked == 2 && bMasked == 0) {
                    increment = 1.0f;
                } else if (bMasked == 2 && aMasked == 0) {
                    increment = -1.0f;
                }

                previousDebounce = now;
            }

            return increment;
        }
};


void DaisyHostAudioCallback(daisy::AudioHandle::InputBuffer in,
    daisy::AudioHandle::OutputBuffer out, size_t size);

template<typename T> class DaisyHost {
    public:
        T device;
        struct sig_AudioSettings* audioSettings;

        void Init(struct sig_AudioSettings* inAudioSettings,
            struct sig_dsp_SignalEvaluator* evaluator) {
            audioSettings = inAudioSettings;
            device.Init(audioSettings, evaluator);
            sig_host_registerGlobalHardwareInterface(&device.hardware);
        }

        void Start() {
            device.Start(DaisyHostAudioCallback);
        }

        void Stop() {
            device.Stop();
            device.board.audio.Stop();
        }
};
};
};
