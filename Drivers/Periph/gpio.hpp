#pragma once

struct GpioInterface {
    bool (*read)();
    void (*set)();
    void (*reset)();
};

#if defined(STM32F4)
#include "gpio_f4.hpp"

#elif defined(STM32F1)
#include "gpio_f1.hpp"
template<uint32_t PortBase, uint32_t Pin>
using Gpio = GpioF1<PortBase, Pin>;

#else
#error "Unsupported MCU family"
#endif