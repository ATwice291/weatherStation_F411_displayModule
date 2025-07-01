#pragma once

#if defined(STM32F4)
#include "sysClock_f4.hpp"
#elif defined(STM32F1)

#else
#error "Unsupported MCU family"
#endif