#pragma once

#if defined(STM32F4)
#include "exti_f4.hpp"

#elif defined(STM32F1)
#include "exti_f1.hpp"

#else
#error "Unsupported MCU family"
#endif