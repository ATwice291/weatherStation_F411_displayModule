#pragma once

#include <cstdint>
#include "stm32f1xx.h"

template<uint32_t PortBase, uint32_t Pin>
class GpioF1 {
public:
    static void init() {
        if constexpr (PortBase == GPIOA_BASE) RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

        GPIO_TypeDef* port = reinterpret_cast<GPIO_TypeDef*>(PortBase);
        if constexpr (Pin < 8) {
            port->CRL &= ~(0b1111 << (Pin * 4));
            port->CRL |= (0b0011 << (Pin * 4)); // Output push-pull
        } else {
            port->CRH &= ~(0b1111 << ((Pin - 8) * 4));
            port->CRH |= (0b0011 << ((Pin - 8) * 4));
        }
    }

    static void set() {
        port->BSRR = (1U << Pin);
    }

    static void clear() {
        port->BRR = (1U << Pin);
    }
};