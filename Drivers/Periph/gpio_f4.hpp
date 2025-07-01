#pragma once

#include <cstdint>
#include "stm32f4xx.h"

enum class GpioPort { 
    A = GPIOA_BASE, 
    B = GPIOB_BASE,
    C = GPIOC_BASE,
    D = GPIOD_BASE,
    E = GPIOE_BASE,
    H = GPIOH_BASE};
enum class GpioPin { P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15 };
enum class GpioMode { In=(0|0), Out_PP=(0|1), Alt_PP=(0|2), Analog=(0|3), Out_OD=(4|1), Alt_OD=(4|2)};
enum class GpioSpeed { Low, Med, High, VeryHigh };
enum class GpioPull { No, Up, Down };
enum class GpioAf { AF0, AF1, AF2, AF3, AF4, AF5, AF6, AF7, AF8, AF9, AF10, AF11, AF12, AF13, AF14, AF15 };

template<GpioPort Port, GpioPin Pin, GpioMode Mode, GpioSpeed Speed, GpioPull Pull, GpioAf Af = GpioAf::AF0>
class Gpio{
private:
    static constexpr uint8_t pin = static_cast<uint8_t>(Pin);
    static constexpr uint32_t mode = static_cast<uint32_t>(Mode);
    static constexpr uint32_t speed = static_cast<uint32_t>(Speed);
    static constexpr uint32_t pull = static_cast<uint32_t>(Pull);
    static constexpr uint32_t af = static_cast<uint32_t>(Af);
    
    static constexpr GPIO_TypeDef* getInstance() {
        if constexpr (Port == GpioPort::A) return GPIOA;
        if constexpr (Port == GpioPort::B) return GPIOB;
        if constexpr (Port == GpioPort::C) return GPIOC;
        if constexpr (Port == GpioPort::D) return GPIOD;
        if constexpr (Port == GpioPort::E) return GPIOE;
        if constexpr (Port == GpioPort::H) return GPIOH;
    }
public:
    static void init() {
        // Включаем тактирование порта
        if constexpr (Port == GpioPort::A) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
        if constexpr (Port == GpioPort::B) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
        if constexpr (Port == GpioPort::C) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
        if constexpr (Port == GpioPort::D) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
        if constexpr (Port == GpioPort::E) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
        if constexpr (Port == GpioPort::H) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;

        getInstance()->MODER &= (~(GPIO_MODER_MODER0 << (pin * 2)));
        getInstance()->MODER |= (((mode)&0x03) << (pin * 2));

        getInstance()->OTYPER &= (~(1U << pin));
        getInstance()->OTYPER |= (((mode)>>2) << pin);

        getInstance()->OSPEEDR &= (~(GPIO_OSPEEDR_OSPEED0 << (pin * 2)));
        getInstance()->OSPEEDR |= ((speed) << (pin * 2));

        getInstance()->PUPDR &= (~(GPIO_PUPDR_PUPD0 << (pin * 2)));
        getInstance()->PUPDR |= ((pull) << (pin * 2));

        getInstance()->AFR[pin / 8] &= (~(GPIO_AFRL_AFSEL0 << ((pin % 8) * 4)));
        if constexpr (Mode == GpioMode::Alt_OD || Mode == GpioMode::Alt_PP) {
            getInstance()->AFR[pin / 8] |= ((af) << ((pin % 8) * 4));
        }
    }
    static bool read() {
        return (getInstance()->IDR & (1U << pin)) != 0;
    }
    static void set() {
        getInstance()->BSRR = (1U << pin);
    }
    static void reset() {
       getInstance()->BSRR = (1U << (pin + 16));
    }
    static GpioInterface getInterface() {
        return {read,
                set,
                reset};
    }
};