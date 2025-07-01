#pragma once
#include "dma.hpp"

enum struct SpiInstance {spi1, spi2, spi3, spi4, spi5, spi6, spi7, spi8};
enum struct SpiPrescaler {psc2 = 0, psc4, psc8, psc16, psc32, psc64, psc128, psc256};
enum struct SpiMode {mode0 = 0, 
                     mode1 = SPI_CR1_CPHA, 
                     mode2 = SPI_CR1_CPOL, 
                     mode3 = SPI_CR1_CPHA | SPI_CR1_CPOL};
enum struct SpiBitOrder {msbFirst, lsbFirst};
enum struct SpiDataSize {oneByte, twoBytes};

using dmaTxParamsDefault = DmaParams<DmaController::Dma2,
                                  DmaStream::S7,
                                  DmaChannel::CH7>;
using DmaTxDefault = Dma<dmaTxParamsDefault>;
using DmaRxDefault = Dma<dmaTxParamsDefault>; //TODO

template <SpiInstance instance = SpiInstance::spi1, 
          SpiPrescaler prescaler = SpiPrescaler::psc2, 
          SpiMode mode = SpiMode::mode3, 
          SpiBitOrder bitOrder = SpiBitOrder::msbFirst, 
          SpiDataSize dataSize = SpiDataSize::oneByte,
          typename DmaTxTypename = DmaTxDefault,
          typename DmaRxTypename = DmaRxDefault>
struct SpiParams {
    using dmaTx = DmaTxTypename;
    using dmaRx = DmaRxTypename;
    static constexpr SpiInstance getInstance() {return instance;}
    static constexpr SpiPrescaler getPrescaler() {return prescaler;}
    static constexpr SpiMode getMode() {return mode;}
    static constexpr SpiBitOrder getBitOrder() {return bitOrder;}
    static constexpr SpiDataSize getDataSize() {return dataSize;}
};

struct SpiInterface {
    void (*transmitReceive)(uint8_t* txData, uint8_t* rxData, uint16_t size);
    void (*transmit)(uint8_t* txData, uint16_t txSize);
    void (*receive)(uint8_t* rxData, uint16_t rxSize);
    void (*transmitReceiveDma)(uint8_t* txData, uint8_t* rxData, uint16_t size);
    void (*transmitDma)(uint8_t* txData, uint16_t txSize);
    void (*receiveDma)(uint8_t* rxData, uint16_t rxSize);
    bool (*isBusy)();
};

#if defined(STM32F4)
#include "spi_f4.hpp"

#elif defined(STM32F1)
#include "spi_f1.hpp"

#else
#error "Unsupported MCU family"
#endif