#pragma once

#include <cstdint>
#include "stm32f4xx.h"
#include <cassert>
#include "dma_f4.hpp"

template<typename params, typename SysClock>
class Spi {
    using Callback = void(*)();
    using dmaTx = typename params::dmaTx;
    using dmaRx = typename params::dmaRx;
private:
    inline static Callback txCompleteCallback = nullptr;
    inline static Callback rxCompleteCallback = nullptr;
    inline static Callback errorCallback = nullptr;

    static constexpr SPI_TypeDef* getInstance() {
        if constexpr(params::getInstance() == SpiInstance::spi1) return SPI1;
        if constexpr(params::getInstance() == SpiInstance::spi2) return SPI2;
        if constexpr(params::getInstance() == SpiInstance::spi3) return SPI3;
        if constexpr(params::getInstance() == SpiInstance::spi4) return SPI4;
        //#if defined SPI5
        if constexpr(params::getInstance() == SpiInstance::spi5) return SPI5;
        //#endif
        //static_assert(false, "Invalid SPI instance");
    }
    static inline void enableClock() {
        if constexpr(params::getInstance() == SpiInstance::spi1) {RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;}
        if constexpr(params::getInstance() == SpiInstance::spi2) {RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;}
        if constexpr(params::getInstance() == SpiInstance::spi3) {RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;}
        if constexpr(params::getInstance() == SpiInstance::spi4) {RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;}
        if constexpr(params::getInstance() == SpiInstance::spi5) {RCC->APB2ENR |= RCC_APB2ENR_SPI5EN;}
    }
    static inline void disableClock() {
        if constexpr(params::getInstance() == SpiInstance::spi1) {RCC->APB2ENR &= (~RCC_APB2ENR_SPI1EN);}
        if constexpr(params::getInstance() == SpiInstance::spi2) {RCC->APB1ENR &= (~RCC_APB1ENR_SPI2EN);}
        if constexpr(params::getInstance() == SpiInstance::spi3) {RCC->APB1ENR &= (~RCC_APB1ENR_SPI3EN);}
        if constexpr(params::getInstance() == SpiInstance::spi4) {RCC->APB2ENR &= (~RCC_APB2ENR_SPI4EN);}
        if constexpr(params::getInstance() == SpiInstance::spi5) {RCC->APB2ENR &= (~RCC_APB2ENR_SPI5EN);}
    }
    static uint32_t getCR1Config() {
        uint32_t cr1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM |
                       (static_cast<uint32_t>(params::getPrescaler()) << SPI_CR1_BR_Pos) |
                       (static_cast<uint32_t>(params::getMode()));
        if constexpr(params::getDataSize() == SpiDataSize::twoBytes) {
            cr1 |= SPI_CR1_DFF;
        }
        if constexpr(params::getBitOrder() == SpiBitOrder::lsbFirst) {
            cr1 |= SPI_CR1_LSBFIRST;
        }
        return cr1;
    }
    static inline bool isOverrun() {
        return (getInstance()->SR & SPI_SR_OVR);
    }
public:
    static constexpr uint32_t getBaudRate() {
        if constexpr(params::getInstance() == SpiInstance::spi1 ||
                     params::getInstance() == SpiInstance::spi4 ||
                     params::getInstance() == SpiInstance::spi5) {
            return SysClock::getAPB2Clock() >> (static_cast<uint32_t>(params::getPrescaler())+1);
        }
        if constexpr(params::getInstance() == SpiInstance::spi2 ||
                     params::getInstance() == SpiInstance::spi3) {
            return SysClock::getAPB1Clock() >> (static_cast<uint32_t>(params::getPrescaler())+1);
        }
        return 0;
    }
    static inline void init() {
        enableClock();
        getInstance()->CR1 = getCR1Config();
    }
    static void enable() {
        getInstance()->CR1 |= SPI_CR1_SPE;
    }
    static void disable() {
        while (!(getInstance()->SR & SPI_SR_RXNE)) {}
        while (!(getInstance()->SR & SPI_SR_TXE)) {}
        while ((getInstance()->SR & SPI_SR_BSY)) {}
        getInstance()->CR1 &= (~SPI_CR1_SPE);
    }
    static inline bool isBusy() {
        return (getInstance()->SR & SPI_SR_BSY);
    }
    static void transmitReceive(uint8_t* txData, uint8_t* rxData, uint16_t size) {
        uint16_t txIndex = 0, rxIndex = 0;
        while (isBusy()) {}
        if((getInstance()->SR & SPI_SR_RXNE)) {
            (void)getInstance()->DR;
        }
        bool txAllowed = true;
        while (rxIndex < size) {
            if ((getInstance()->SR & SPI_SR_TXE) && txAllowed) {
                if(txIndex < size) {
                    getInstance()->DR = txData[txIndex++];
                    txAllowed = false;
                }
            }
            if((getInstance()->SR & SPI_SR_RXNE)) {
                if(rxIndex < size) {
                    rxData[rxIndex++] = getInstance()->DR;
                    txAllowed = true;
                }
            }
        }
    }
    static void transmit(uint8_t* txData, uint16_t txSize) {
        uint16_t txIndex = 0;
        while (isBusy()) {}
        if((getInstance()->SR & SPI_SR_RXNE)) {
            (void *)getInstance()->DR;
        }
        while (txIndex < txSize) {
            if ((getInstance()->SR & SPI_SR_TXE)) {
                getInstance()->DR = txData[txIndex++];
            }
        }
    }
    static void receive(uint8_t* rxData, uint16_t rxSize) {
        transmitReceive(rxData, rxData, rxSize);
    }
    static void transmitReceiveDma(uint8_t* txData, uint8_t* rxData, uint16_t size) {
        dmaRx::stop();
        dmaRx::configure(reinterpret_cast<uint32_t>(&(getInstance()->DR)), reinterpret_cast<uint32_t>(rxData), size);
        dmaRx::enableInterrupts();
        dmaRx::start();
        dmaRxEnable();
        dmaTx::stop();
        dmaTx::configure(reinterpret_cast<uint32_t>(txData), reinterpret_cast<uint32_t>(&(getInstance()->DR)), size);
        dmaTx::enableInterrupts();
        dmaTx::start();
        dmaTxEnable();
    }
    static void transmitDma(uint8_t* txData, uint16_t txSize) {
        dmaTx::stop();
        dmaTx::configure(reinterpret_cast<uint32_t>(txData), reinterpret_cast<uint32_t>(&(getInstance()->DR)), txSize);
        dmaTx::enableInterrupts();
        dmaTx::start();
        dmaTxEnable();
    }
    static void receiveDma(uint8_t* rxData, uint16_t rxSize) {
        transmitReceiveDma(rxData, rxData, rxSize);
    }
    static void setTxCompleteCallback(Callback cb) {
        txCompleteCallback = cb;
    }
    static void setRxCompleteCallback(Callback cb) {
        rxCompleteCallback = cb;
    }
    static void setErrorCallback(Callback cb) {
        errorCallback = cb;
    }
    static inline void dmaTxEnable() {
        getInstance()->CR2 |= SPI_CR2_TXDMAEN; 
    }
    static inline void dmaTxDisable() {
        getInstance()->CR2 &= (~SPI_CR2_TXDMAEN); 
    }
    static inline void dmaRxEnable() {
        getInstance()->CR2 |= SPI_CR2_RXDMAEN; 
    }
    static inline void dmaRxDisable() {
        getInstance()->CR2 &= (~SPI_CR2_RXDMAEN); 
    }
    static inline void rxneIrqEnable() {
        getInstance()->CR2 |= SPI_CR2_RXNEIE; 
    }
    static inline void rxneIrqDisable() {
        getInstance()->CR2 &= (~SPI_CR2_RXNEIE); 
    }
    static inline void txeIrqEnable() {
        getInstance()->CR2 |= SPI_CR2_TXEIE; 
    }
    static inline void txeIrqDisable() {
        getInstance()->CR2 &= (~SPI_CR2_TXEIE); 
    }
    static inline void errIrqEnable() {
        getInstance()->CR2 |= SPI_CR2_ERRIE; 
    }
    static inline void errIrqDisable() {
        getInstance()->CR2 &= (~SPI_CR2_ERRIE); 
    }
    static void handleInterrupt() {
        if(((getInstance()->SR & SPI_SR_OVR) == 0) &&
            ((getInstance()->SR & SPI_SR_RXNE) == SPI_SR_RXNE) &&
            ((getInstance()->CR2 & SPI_CR2_RXNEIE) == SPI_CR2_RXNEIE)) {
            if (rxCompleteCallback) {
                rxCompleteCallback();
            }
        }
        if(((getInstance()->SR & SPI_SR_TXE) == SPI_SR_TXE) &&
            ((getInstance()->CR2 & SPI_CR2_TXEIE) == SPI_CR2_TXEIE)) {
            if (txCompleteCallback) {
                txCompleteCallback();
            }
        }
        if((((getInstance()->SR & SPI_SR_MODF) == SPI_SR_MODF) ||
            ((getInstance()->SR & SPI_SR_OVR) == SPI_SR_OVR) ||
            ((getInstance()->SR & SPI_SR_FRE) == SPI_SR_FRE)) &&
            ((getInstance()->CR2 & SPI_CR2_ERRIE) == SPI_CR2_ERRIE)) {
            if (errorCallback) {
                errorCallback();
            }
        }
    }
    static SpiInterface getInterface() {
        return {transmitReceive,
                transmit,
                receive,
                transmitReceiveDma,
                transmitDma,
                receiveDma,
                isBusy};
    }
};