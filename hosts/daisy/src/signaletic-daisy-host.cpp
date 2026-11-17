#include "../include/signaletic-daisy-host.hpp"

sig::libdaisy::EdgeTimedGateInput* sig::libdaisy::edgeTimedGateInputs[16] = {
    NULL
};

// Each pin interrupt handler serves a range of interrupt lines, so it
// checks which of its lines are pending.
static void sig_libdaisy_dispatchPinInterrupts(uint8_t firstLine,
    uint8_t lastLine) {
    for (uint8_t line = firstLine; line <= lastLine; line++) {
        uint32_t lineMask = 1u << line;
        if (__HAL_GPIO_EXTI_GET_IT(lineMask)) {
            __HAL_GPIO_EXTI_CLEAR_IT(lineMask);
            sig::libdaisy::EdgeTimedGateInput* input =
                sig::libdaisy::edgeTimedGateInputs[line];
            if (input != NULL) {
                input->OnEdge();
            }
        }
    }
}

extern "C" void EXTI0_IRQHandler(void) {
    sig_libdaisy_dispatchPinInterrupts(0, 0);
}

extern "C" void EXTI1_IRQHandler(void) {
    sig_libdaisy_dispatchPinInterrupts(1, 1);
}

extern "C" void EXTI2_IRQHandler(void) {
    sig_libdaisy_dispatchPinInterrupts(2, 2);
}

extern "C" void EXTI3_IRQHandler(void) {
    sig_libdaisy_dispatchPinInterrupts(3, 3);
}

extern "C" void EXTI4_IRQHandler(void) {
    sig_libdaisy_dispatchPinInterrupts(4, 4);
}

extern "C" void EXTI9_5_IRQHandler(void) {
    sig_libdaisy_dispatchPinInterrupts(5, 9);
}

extern "C" void EXTI15_10_IRQHandler(void) {
    sig_libdaisy_dispatchPinInterrupts(10, 15);
}

void sig::libdaisy::DaisyHostAudioCallback(daisy::AudioHandle::InputBuffer in,
    daisy::AudioHandle::OutputBuffer out, size_t size) {
    struct sig_host_HardwareInterface* hardware =
        sig_host_getGlobalHardwareInterface();
    hardware->audioInputChannels = (float**) in;
    hardware->audioOutputChannels = (float**) out;

    // Invoke callback before evaluating the signal graph.
    hardware->onEvaluateSignals(size, hardware);

    // Evaluate the signal graph.
    struct sig_dsp_SignalEvaluator* evaluator = hardware->evaluator;
    evaluator->evaluate(evaluator);

    // Invoke the after callback.
    hardware->afterEvaluateSignals(size, hardware);
}
