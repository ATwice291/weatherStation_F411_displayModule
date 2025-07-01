#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx.h"
#include "sysClock.hpp"
#include "gpio.hpp"
#include "spi.hpp"
#include "mcp2515.hpp"
#include "st7735.hpp"

//constexpr Prescalers Psc = {1, 2, 1}; 
using psc = Prescalers<1,2,1>;
using pll = PllConfig<16, 100, 2, 2>;
using paramsHSI = ClockParams<SysClockSource::HSI, 25000000, pll, psc>;
using SystemClockHSI = SysClock<paramsHSI>;

using paramsHSE = ClockParams<SysClockSource::HSE, 25000000, pll, psc>;
using SystemClockHSE = SysClock<paramsHSE>;

using paramsPLLHSI = ClockParams<SysClockSource::PLL_HSI, 25000000, pll, psc>;
using SystemClockPLLHSI = SysClock<paramsPLLHSI>;

using pllHSE = PllConfig<25, 200, 2, 2>;
using paramsPLLHSE = ClockParams<SysClockSource::PLL_HSE, 25000000, pllHSE, psc>;
using SystemClock = SysClock<paramsPLLHSE>;

//PC13=LED1
//PA2=LED2
//PA3=LED3
using Led1 = Gpio<GpioPort::C, GpioPin::P13, GpioMode::Out_PP, GpioSpeed::Med, GpioPull::No>;
using Led2 = Gpio<GpioPort::A,  GpioPin::P2, GpioMode::Out_PP, GpioSpeed::Med, GpioPull::No>;
using Led3 = Gpio<GpioPort::A,  GpioPin::P3, GpioMode::Out_PP, GpioSpeed::Med, GpioPull::No>;

//PB12=SPI2_CS
//PB13=SPI2_SCK
//PB14=SPI2_MISO
//PB15=SPI2_MOSI

//PA15 - MCP2515 CS
//PB3 - MCP2515 INT 
using Mcp2515_CS = Gpio<GpioPort::A, GpioPin::P15, GpioMode::Out_PP, GpioSpeed::VeryHigh, GpioPull::Up>;

using Spi2_SCK = Gpio<GpioPort::B, GpioPin::P13, GpioMode::Alt_PP, GpioSpeed::VeryHigh, GpioPull::No, GpioAf::AF5>;
using Spi2_MISO = Gpio<GpioPort::B, GpioPin::P14, GpioMode::Alt_PP, GpioSpeed::VeryHigh, GpioPull::No, GpioAf::AF5>;
using Spi2_MOSI = Gpio<GpioPort::B, GpioPin::P15, GpioMode::Alt_PP, GpioSpeed::VeryHigh, GpioPull::No, GpioAf::AF5>;

using spi2Params = SpiParams<SpiInstance::spi2, 
                             SpiPrescaler::psc8,
                             SpiMode::mode3,
                             SpiBitOrder::msbFirst,
                             SpiDataSize::oneByte>;
using Spi2 = Spi<spi2Params, SystemClock>;

using MyCanParams = Mcp2515CanParams<2,
                        Mcp2515PropSeg::T1,
                        Mcp2515Phase1::T3,
                        Mcp2515Phase2::T3,
                        Mcp2515SynchroJumpWidth::T1>;


using Display_CS = Gpio<GpioPort::B, GpioPin::P0, GpioMode::Out_PP, GpioSpeed::VeryHigh, GpioPull::Up>;
using Spi1_SCK = Gpio<GpioPort::A, GpioPin::P5, GpioMode::Alt_PP, GpioSpeed::VeryHigh, GpioPull::No, GpioAf::AF5>;
using Spi1_MOSI = Gpio<GpioPort::A, GpioPin::P7, GpioMode::Alt_PP, GpioSpeed::VeryHigh, GpioPull::No, GpioAf::AF5>;
using Display_DC = Gpio<GpioPort::A, GpioPin::P4, GpioMode::Out_PP, GpioSpeed::VeryHigh, GpioPull::Up>;
using Display_RESET = Gpio<GpioPort::A, GpioPin::P6, GpioMode::Out_PP, GpioSpeed::VeryHigh, GpioPull::Up>;

using dmaTxSpi1Params = DmaParams<DmaController::Dma2,
                                  DmaStream::S2,
                                  DmaChannel::CH2,
                                  DmaDirection::MemToPerif,
                                  false,
                                  false,
                                  true>;
using DmaTxSpi1 = Dma<dmaTxSpi1Params>;

using spi1Params = SpiParams<SpiInstance::spi1, 
                             SpiPrescaler::psc8,
                             SpiMode::mode3,
                             SpiBitOrder::msbFirst,
                             SpiDataSize::oneByte,
                             DmaTxSpi1>;
using Spi1 = Spi<spi1Params, SystemClock>;


//using ExtClock = DS3231<I2c3, (0x68<<1)>;
//using OledDisplay = SSD1306<I2c1, (0x3C<<1)>;

using ModeButton = Gpio<GpioPort::C, GpioPin::P15, GpioMode::In, GpioSpeed::Low, GpioPull::Up, GpioAf::AF0>;
using PlusButton = Gpio<GpioPort::C, GpioPin::P14, GpioMode::In, GpioSpeed::Low, GpioPull::Up, GpioAf::AF0>;
using MinusButton = Gpio<GpioPort::A, GpioPin::P0, GpioMode::In, GpioSpeed::Low, GpioPull::Up, GpioAf::AF0>;

#endif /* __MAIN_H */
