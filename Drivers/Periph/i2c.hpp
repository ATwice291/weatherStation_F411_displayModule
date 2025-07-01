#pragma once

enum struct I2cInstance {i2c1, i2c2, i2c3, i2c4, i2c5, i2c6, i2c7, i2c8};
enum struct I2cSpeed {standart, fast, fastPlus};
enum struct I2cMemAddrSize {oneByte, twoBytes};

template <I2cInstance instance = I2cInstance::i2c1, 
          I2cSpeed speed = I2cSpeed::fast>
struct I2cParams {
    static constexpr I2cInstance getInstance() {return instance;}
    static constexpr I2cSpeed getSpeed() {return speed;}
};

struct I2CInterface {
    bool (*acknowledgePolling)(uint8_t devAddress, uint32_t attempts);
    void (*transmit)(uint8_t devAddress, const uint8_t *data, uint16_t size);
    void (*receive)(uint8_t devAddress, uint8_t *data, uint16_t size);
    void (*memoryWrite)(uint8_t devAddress, uint16_t memAddress, I2cMemAddrSize addressSize, const uint8_t *data, uint16_t size);
    void (*memoryRead)(uint8_t devAddress, uint16_t memAddress, I2cMemAddrSize addressSize, uint8_t *data, uint16_t size);
};

#if defined(STM32F4)
#include "i2c_f4.hpp"

#elif defined(STM32F1)
#include "i2c_f1.hpp"

#else
#error "Unsupported MCU family"
#endif