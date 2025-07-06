#pragma once

#include <cstdint>
#include "stm32f4xx.h"
#include "gpio_f4.hpp"


template<GpioPort Port, GpioPin Pin>
class Exti{
    using Callback = void(*)();
private:
    inline static Callback interruptCallback = nullptr;
    static constexpr uint8_t pin = static_cast<uint8_t>(Pin);
    static constexpr uint32_t pinMask = (1 << pin);
    static constexpr uint8_t exti_idx = pin / 4;
    static constexpr uint8_t exti_shift = (pin % 4) * 4;
    static constexpr uint8_t port =
        (Port == GpioPort::A) ? 0 :
        (Port == GpioPort::B) ? 1 :
        (Port == GpioPort::C) ? 2 :
        (Port == GpioPort::D) ? 3 :
        (Port == GpioPort::E) ? 4 :
        (Port == GpioPort::H) ? 7 : 0;

public:
    static void init(bool rising, bool falling) {
        RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
        SYSCFG->EXTICR[exti_idx] &= ~(0xF << exti_shift);
        SYSCFG->EXTICR[exti_idx] |= (port << exti_shift);
        
        if (rising) {
            EXTI->RTSR |=  pinMask; 
        } else {
            EXTI->RTSR &= ~pinMask;
        }
        if (falling) {
            EXTI->FTSR |=  pinMask; 
        } else {
            EXTI->FTSR &= ~pinMask;
        }
        EXTI->IMR |= pinMask;
    }
    static void setInterruptCallback(Callback cb) {
        interruptCallback = cb;
    }
    static void interruptEnable() {
        EXTI->IMR |= pinMask;
    }
    static void interruptDisable() {
        EXTI->IMR &= ~pinMask;
    }
    static void handleInterrupt() {
        if(interruptCallback) {
            interruptCallback();
        }
    }

};