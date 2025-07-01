#pragma once

#include <cstdint>
#include "stm32f4xx.h"
#include <cassert>
#include "sysClock_f4.hpp"
#include "dma_f4.hpp"

template<typename params, typename SysClock>
class I2c {
    //using Callback = void(*)();
public:
    enum ErrorCodes {
        ERROR_NONE = 0x00,
        ERROR_BERR = 0x00000001U,
        ERROR_ARLO = 0x00000002U,
        ERROR_AF = 0x00000004U,
        ERROR_OVR = 0x00000008U,
        ERROR_DMA = 0x00000010U,
        ERROR_TIMEOUT = 0x00000020U,
        ERROR_SIZE = 0x00000040U,
        ERROR_DMA_PARAM = 0x00000080U,
        WRONG_START = 0x00000200U
    };
private:
    //inline static Callback txCompleteCallback = nullptr;
    //inline static Callback rxCompleteCallback = nullptr;
    //inline static Callback errorCallback = nullptr;

    static constexpr I2C_TypeDef* getInstance() {
        if constexpr(params::getInstance() == I2cInstance::i2c1) return I2C1;
        if constexpr(params::getInstance() == I2cInstance::i2c2) return I2C2;
        if constexpr(params::getInstance() == I2cInstance::i2c3) return I2C3;
    }
    static inline void enableClock() {
        if constexpr(params::getInstance() == I2cInstance::i2c1) {RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;}
        if constexpr(params::getInstance() == I2cInstance::i2c2) {RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;}
        if constexpr(params::getInstance() == I2cInstance::i2c3) {RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;}
    }
    static inline void disableClock() {
        if constexpr(params::getInstance() == I2cInstance::i2c1) {RCC->APB1ENR &= (~RCC_APB1ENR_I2C1EN);}
        if constexpr(params::getInstance() == I2cInstance::i2c2) {RCC->APB1ENR &= (~RCC_APB1ENR_I2C2EN);}
        if constexpr(params::getInstance() == I2cInstance::i2c3) {RCC->APB1ENR &= (~RCC_APB1ENR_I2C3EN);}
    }
    static constexpr uint32_t getCR1Config() {
        uint32_t cr1 = 0;
        return cr1;
    }
    static constexpr uint32_t getCR2Config() {
        return getFrequency();
    }
    static constexpr uint32_t getTRISEConfig() {
        uint32_t trise = 0;
        if constexpr(params::getSpeed() == I2cSpeed::standart) {
            trise = getFrequency() + 1;
        }
        if constexpr(params::getSpeed() == I2cSpeed::fast || params::getSpeed() == I2cSpeed::fastPlus) {
            trise = ((getFrequency() * 300U) / 1000U) + 1U;
        }
        return trise;
    }
    static constexpr uint32_t getCCRConfig() {
        uint32_t ccr = 0;
        if constexpr(params::getSpeed() == I2cSpeed::standart) {
            ccr |= (getFrequency() * 10 / 2);
        }
        if constexpr(params::getSpeed() == I2cSpeed::fast) {
            ccr |= I2C_CCR_FS;
            ccr |= static_cast<uint32_t>(getFrequency() * 2.5 / 3);
        }
        if constexpr(params::getSpeed() == I2cSpeed::fastPlus) {
            ccr |= I2C_CCR_FS;
            ccr |= (getFrequency() / 3);
        }
        return ccr;
    }
    static constexpr uint32_t getFrequency() {
        constexpr uint32_t frequency = SysClock::getAPB1Clock() / 1000000;
        if constexpr(params::getSpeed() == I2cSpeed::standart) {
            static_assert(frequency >= 2 && frequency <= 50, "Incorrect frerquency");
        }
        if constexpr(params::getSpeed() == I2cSpeed::fast || params::getSpeed() == I2cSpeed::fastPlus) {
            static_assert(frequency >= 4 && frequency <= 50, "Incorrect frerquency");
        }
        return frequency;
    }
    static void enable() {
        getInstance()->CR1 |= I2C_CR1_PE;
    }
    static void disable() {
        getInstance()->CR1 &= (~I2C_CR1_PE);
    }
    static void masterPrepare() {
        waitForSR2FlagReset(I2C_SR2_BUSY);
        if(errorCode == ERROR_TIMEOUT) {
            return;
        }
        if((getInstance()->CR1 & I2C_CR1_PE) != I2C_CR1_PE) {
            enable();
        }
        getInstance()->CR1 &= (~I2C_CR1_POS);
        getInstance()->CR1 &= (~I2C_CR1_STOP);
    }
    static void masterRequestWrite(uint8_t address) {
        generateStart();
        uint32_t timeout = 0;
        while((getInstance()->SR1 & I2C_SR1_SB) == 0) {
            timeout ++;
            if(timeout > 4321) {
                errorCode |= ERROR_TIMEOUT;
                return;
            }
        }
        getInstance()->DR = static_cast<uint8_t>(address & (~I2C_OAR1_ADD0));
        timeout = 0;
        while((getInstance()->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR) {
            timeout ++;
            if(timeout > 4321) {
                errorCode |= ERROR_TIMEOUT;
                return;
            }
            if(isAcknowledgeFailed()) {
                generateStop();
                return;
            }
        }
    }
    static void masterRequestRead(uint8_t address) {
        getInstance()->CR1 |= I2C_CR1_ACK;
        generateStart();
        uint32_t timeout = 0;
        while((getInstance()->SR1 & I2C_SR1_SB) == 0) {
            timeout ++;
            if(timeout > 4321) {
                errorCode |= ERROR_TIMEOUT;
                return;
            }
        }
        getInstance()->DR = static_cast<uint8_t>(address | I2C_OAR1_ADD0);
        timeout = 0;
        while((getInstance()->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR) {
            timeout ++;
            if(timeout > 4321) {
                errorCode |= ERROR_TIMEOUT;
                return;
            }
            if(isAcknowledgeFailed()) {
                generateStop();
                return;
            }
        }
    }
    static void masterSendBytes(const uint8_t *data, uint16_t size) {
        while(size > 0) {
            waitForSR1FlagSet(I2C_SR1_TXE);
            getInstance()->DR = *data;
            data++;
            size--;
            if(((getInstance()->SR1 & I2C_SR1_BTF) == I2C_SR1_BTF) && (size > 0)) {
                getInstance()->DR = *data;
                data++;
                size--;
            }
            waitForSR1FlagSet(I2C_SR1_BTF);
        }
    }
    static void masterReceiveBytes(uint8_t *data, uint16_t size) {
        if(size == 0) {
            clearAddressFlag();
            generateStop(); 
        } else if (size == 1) {
            getInstance()->CR1 &= (~I2C_CR1_ACK);  
            clearAddressFlag(); 
            generateStop(); 
        } else if (size == 2) {
            getInstance()->CR1 &= (~I2C_CR1_ACK);  
            getInstance()->CR1 |= I2C_CR1_POS;
            clearAddressFlag(); 
        } else {
            getInstance()->CR1 |= I2C_CR1_ACK;
            clearAddressFlag(); 
        }
        while(size > 0) {
            if(size <= 3) {
                if(size == 1) {
                    waitOnRXNE();
                    if(errorCode == ERROR_TIMEOUT) {
                        return;
                    }
                    *data = getInstance()->DR;
                    data++;
                    size--;
                } else if(size == 2) {
                    waitForSR1FlagSet(I2C_SR1_BTF);
                    if(errorCode == ERROR_TIMEOUT) {
                        return;
                    }
                    generateStop(); 
                    *data = getInstance()->DR;
                    data++;
                    size--;
                    *data = getInstance()->DR;
                    data++;
                    size--;
                } else {
                    waitForSR1FlagSet(I2C_SR1_BTF);
                    if(errorCode == ERROR_TIMEOUT) {
                        return;
                    }
                    getInstance()->CR1 &= (~I2C_CR1_ACK);  
                    *data = getInstance()->DR;
                    data++;
                    size--;
                }
            } else {
                waitOnRXNE();
                if(errorCode == ERROR_TIMEOUT) {
                    return;
                }
                *data = getInstance()->DR;
                data++;
                size--;
                if(((getInstance()->SR1 & I2C_SR1_BTF) == I2C_SR1_BTF)) {
                    *data = getInstance()->DR;
                    data++;
                    size--;
                }
            }
        }
    }
    static void clearAddressFlag() {
        volatile uint32_t tmp = getInstance()->SR1;
        tmp = getInstance()->SR2;
        (void)tmp;
    }
    static bool isAcknowledgeFailed() {
        if((getInstance()->SR1 & I2C_SR1_AF) == I2C_SR1_AF) {
            getInstance()->SR1 &= (~I2C_SR1_AF);
            errorCode |= ERROR_AF;
            return true;
        }
        return false;
    }
    static void waitForSR1FlagSet(uint32_t flag) {
        uint32_t timeout = 0;
        while((getInstance()->SR1 & flag) != flag) {
            timeout ++;
            if(timeout > 4321) {
                errorCode |= ERROR_TIMEOUT;
                return;
            }
            if(isAcknowledgeFailed()) {
                generateStop();
            }
        }
    }
    static void waitForSR2FlagReset(uint32_t flag) {
        uint32_t timeout = 0;
        while((getInstance()->SR2 & flag) == flag) {
            timeout ++;
            if(timeout > 4321) {
                errorCode |= ERROR_TIMEOUT;
                return;
            }
            if(isAcknowledgeFailed()) {
                generateStop();
            }
        }
    }
    static void waitOnRXNE() {
        uint32_t timeout = 0;
        while((getInstance()->SR1 & I2C_SR1_RXNE) != I2C_SR1_RXNE) {
            timeout ++;
            if(timeout > 4321) {
                errorCode |= ERROR_TIMEOUT;
                return;
            }
            if((getInstance()->SR1 & I2C_SR1_STOPF) == I2C_SR1_STOPF) {
                getInstance()->SR1 &= (~I2C_SR1_STOPF);
                return;
            }
        }
    }
    static void generateStart() {
        getInstance()->CR1 |= I2C_CR1_START;
    }
    static void generateStop() {
        getInstance()->CR1 |= I2C_CR1_STOP;
    }
public:
    static inline uint32_t errorCode;
    static void init() {
        enableClock();
        getInstance()->CR1 |= I2C_CR1_SWRST;
        getInstance()->CR1 &= (~I2C_CR1_SWRST);
        getInstance()->CR2 = getCR2Config();
        getInstance()->TRISE = getTRISEConfig();
        getInstance()->CCR = getCCRConfig();
        enable();
    }
    static bool acknowledgePolling(uint8_t devAddress, uint32_t attempts) {
        while(attempts--) {
            errorCode = ERROR_NONE;
            masterPrepare();
            masterRequestRead(devAddress);
            if(errorCode == ERROR_NONE) {
                generateStop();
                return true;
            }
        }
        return false;
    }
    static void transmit(uint8_t devAddress, const uint8_t *data, uint16_t size) {
        errorCode = ERROR_NONE;
        masterPrepare();
        masterRequestWrite(devAddress);
        if(errorCode != ERROR_NONE) {
            return;
        }
        clearAddressFlag();
        masterSendBytes(data, size);
        generateStop();   
    }
    static void receive(uint8_t devAddress, uint8_t *data, uint16_t size) {
        errorCode = ERROR_NONE;
        masterPrepare();
        masterRequestRead(devAddress);
        if(errorCode != ERROR_NONE) {
            return;
        }
        masterReceiveBytes(data, size);
    }
    static void memoryWrite(uint8_t devAddress, uint16_t memAddress, I2cMemAddrSize addressSize, const uint8_t *data, uint16_t size) {
        errorCode = ERROR_NONE;
        masterPrepare();
        masterRequestWrite(devAddress);
        if(errorCode != ERROR_NONE) {
            return;
        }
        clearAddressFlag();
        uint8_t memAddr[2];
        uint16_t addrSize = 0;
        if(addressSize == I2cMemAddrSize::oneByte) {
            memAddr[addrSize++] = memAddress & 0xFF;
        } else {
            memAddr[addrSize++] = (memAddress>>8) & 0xFF;
            memAddr[addrSize++] = memAddress & 0xFF;
        }
        masterSendBytes(memAddr, addrSize);
        masterSendBytes(data, size);
        generateStop();  
    }
    static void memoryRead(uint8_t devAddress, uint16_t memAddress, I2cMemAddrSize addressSize, uint8_t *data, uint16_t size) {
        errorCode = ERROR_NONE;
        masterPrepare();
        masterRequestWrite(devAddress);
        if(errorCode != ERROR_NONE) {
            return;
        }
        clearAddressFlag();
        uint8_t memAddr[2];
        uint16_t addrSize = 0;
        if(addressSize == I2cMemAddrSize::oneByte) {
            memAddr[addrSize++] = memAddress & 0xFF;
        } else {
            memAddr[addrSize++] = (memAddress>>8) & 0xFF;
            memAddr[addrSize++] = memAddress & 0xFF;
        }
        masterSendBytes(memAddr, addrSize);
        masterRequestRead(devAddress);
        if(errorCode != ERROR_NONE) {
            return;
        }
        masterReceiveBytes(data, size);
    }
    static I2CInterface getInterface() {
        return {acknowledgePolling,
                transmit,
                receive,
                memoryWrite,
                memoryRead};
    }
};