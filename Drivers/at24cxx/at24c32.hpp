#pragma once

#include <cstdint>
#include "../Periph/i2c.hpp"

template<typename I2C, uint8_t devAddress>
class AT24C32 {
public:
    static void init() {
    }
    static void read(uint32_t memAddress, uint8_t* data, uint32_t size) {
        I2C::memoryRead(devAddress, memAddress, memAddrSize, data, size);
    }
    static uint32_t write(uint32_t memAddress, uint8_t* data, uint32_t size) {
        uint32_t dataChunkSize = ((memAddress+pageSize)&(~bytesInPageMask))-memAddress;
        if(dataChunkSize > size) dataChunkSize = size;
        I2C::memoryWrite(devAddress, memAddress, memAddrSize, data, dataChunkSize);
        if(I2C::errorCode != I2C::ERROR_NONE) {
            return 0;
        }
        return dataChunkSize;
    }
private:
    static constexpr I2cMemAddrSize memAddrSize = I2cMemAddrSize::twoBytes;
    static constexpr uint32_t pageSize = 32;
    static constexpr uint32_t bytesInPageMask = 0x001F;
};