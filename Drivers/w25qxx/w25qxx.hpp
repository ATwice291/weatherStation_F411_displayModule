#pragma once

#include <cstdint>
#include "../Periph/spi.hpp"

template<typename SPI, typename CS>
class W25Qxx {
    public:
    static void init() {
        csHigh();
        if (readID() != 0xEF4018) {
           
        }
    }

    static uint32_t readID() {
        uint8_t data[4] = {0};
        data[0] = CMD_JEDEC_ID;

        csLow();
        SPI::transmitReceive(data, data, 4);
        while(SPI::isBusy()){}
        csHigh();

        return (data[1] << 16) | (data[2] << 8) | data[3];
    }

    static void readData(uint32_t address, uint8_t* buffer, uint32_t length) {
        uint8_t cmd[4] = {
            CMD_READ_DATA,
            static_cast<uint8_t>(address >> 16),
            static_cast<uint8_t>(address >> 8),
            static_cast<uint8_t>(address)
        };

        csLow();
        SPI::transmit(cmd, 4);
        while(SPI::isBusy()){}
        SPI::receive(buffer, length);
        while(SPI::isBusy()){}
        csHigh();
    }

    static void pageProgram(uint32_t address, uint8_t* data, uint32_t length) {
        if (length > 256) length = 256; 

        writeEnable();

        uint8_t cmd[4] = {
            CMD_PAGE_PROGRAM,
            static_cast<uint8_t>(address >> 16),
            static_cast<uint8_t>(address >> 8),
            static_cast<uint8_t>(address)
        };

        csLow();
        SPI::transmit(cmd, 4);
        while(SPI::isBusy()){}
        SPI::transmit(data, length);
        while(SPI::isBusy()){}
        csHigh();

        waitUntilReady();
    }

    static void eraseSector(uint32_t address) {
        writeEnable();

        uint8_t cmd[4] = {
            CMD_SECTOR_ERASE,
            static_cast<uint8_t>(address >> 16),
            static_cast<uint8_t>(address >> 8),
            static_cast<uint8_t>(address)
        };

        csLow();
        SPI::transmit(cmd, 4);
        while(SPI::isBusy()){}
        csHigh();

        waitUntilReady();
    }
    private:
    enum Commands {
        CMD_WRITE_ENABLE  = 0x06,
        CMD_READ_STATUS   = 0x05,
        CMD_JEDEC_ID      = 0x9F,
        CMD_READ_DATA     = 0x03,
        CMD_PAGE_PROGRAM  = 0x02,
        CMD_SECTOR_ERASE  = 0x20
    };
    static inline void csLow() { CS::reset(); }
    static inline void csHigh() { CS::set(); }
    static void writeEnable() {
        uint8_t cmd = CMD_WRITE_ENABLE;
        csLow();
        SPI::transmit(&cmd, 1);
        while(SPI::isBusy()){}
        csHigh();
    }
    static bool isBusy() {
        uint8_t data[2];
        data[0] = CMD_READ_STATUS;
        csLow();
        SPI::transmitReceive(data, data, 2);
        while(SPI::isBusy()){}
        csHigh();
        return (data[1] & 0x01);
    }
    static inline void waitUntilReady() {
        while (isBusy());
    }
};