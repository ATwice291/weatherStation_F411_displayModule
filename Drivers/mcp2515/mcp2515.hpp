#pragma once

#include <cstdint>
#include "../Periph/spi.hpp"
#include "../Periph/gpio.hpp"
//#include "cmsis_os.h"

struct CanMessage {
    uint16_t id;
    uint8_t len;
    uint8_t data[8];
};

enum struct Mcp2515PropSeg:uint8_t {T1=1, T2, T3, T4, T5, T6, T7, T8};
enum struct Mcp2515Phase1 {T1=1, T2, T3, T4, T5, T6, T7, T8};
enum struct Mcp2515Phase2 {T2=2, T3, T4, T5, T6, T7, T8};
enum struct Mcp2515SynchroJumpWidth {T1=1, T2, T3, T4};

template<uint8_t prescaler,
        Mcp2515PropSeg propSeg,
        Mcp2515Phase1 phase1,
        Mcp2515Phase2 phase2,
        Mcp2515SynchroJumpWidth synchro
>
struct Mcp2515CanParams {
    static_assert(static_cast<uint8_t>(propSeg)+static_cast<uint8_t>(phase1)>=static_cast<uint8_t>(phase2), 
    "PropSeg+PS1 >= PS2");
    static_assert(static_cast<uint8_t>(phase2)>static_cast<uint8_t>(synchro), 
    "PS2 > SJW");
    static_assert(prescaler>=2 && prescaler<=128, "prescaler should be between 2 and 128");
    static constexpr uint8_t getBrp() {return (prescaler/2-1);}
    static constexpr uint8_t getCNF1() {
        return ((static_cast<uint8_t>(synchro)-1)<<6) | 
                getBrp();
    }
    static constexpr uint8_t getCNF2() {
        return (1<<7) | 
                ((static_cast<uint8_t>(phase1)-1)<<3) | 
                ((static_cast<uint8_t>(propSeg)-1)<<0);
    }
    static constexpr uint8_t getCNF3() {
        return ((static_cast<uint8_t>(phase2)-1)<<0);
    }
};

class MCP2515 {
private:
    enum Commands { 
        CMD_RESET = 0xC0,
        CMD_READ = 0x03,
        CMD_READ_RX_BUFFER = 0x90,
        CMD_WRITE = 0x02,
        CMD_LOAD_TX_BUFFER = 0x40,
        CMD_RTS = 0x80,
        CMD_READ_STATUS = 0xA0,
        CMD_RX_STATUS = 0xB0,
        CMD_BIT_MODIFY = 0x05
    };
    enum Registers {
        REG_CANSTAT = 0x0E,
        REG_CANCTRL = 0x0F,
        REG_CNF3 = 0x28,
        REG_CNF2 = 0x29,
        REG_CNF1 = 0x2A,
        REG_TXB0CTRL = 0x30,
        REG_TXB0SIDH = 0x31,
        REG_TXB0SIDL = 0x32,
        REG_TXB0EID8 = 0x33,
        REG_TXB0EID0 = 0x34,
        REG_TXB0DLC = 0x35,
        REG_TXB0D0 = 0x36,
        REG_TXB0D1 = 0x37,
        REG_TXB0D2 = 0x38,
        REG_TXB0D3 = 0x39,
        REG_TXB0D4 = 0x3A,
        REG_TXB0D5 = 0x3B,
        REG_TXB0D6 = 0x3C,
        REG_TXB0D7 = 0x3D,
    };
    enum OpModes {
        OM_NORMAL = 0x00,
        OM_SLEEP = 0x20,
        OM_LOOPBACK = 0x40,
        OM_LISTEN_ONLY = 0x60,
        OM_CONFIG = 0x80
    };
    static constexpr uint8_t OP_MODE_MASK = 0xE0;

    template<typename Func>
    void withCs(Func&& f) {
        csLow();
        f();
        while (_spi.isBusy()) {}
        csHigh();
    }
    
public:
    MCP2515(SpiInterface spi, GpioInterface cs) 
        : _spi(spi), _cs(cs) {}

    void reset() {
        command(CMD_RESET);
    }
    template <typename config>
    void init() {
        csHigh();

        reset();
        switchToMode(OM_CONFIG);

        writeRegister(REG_CNF1, config::getCNF1());
        writeRegister(REG_CNF2, config::getCNF2());
        writeRegister(REG_CNF3, config::getCNF3());
        
        bitModify(REG_CANCTRL, 0x08, 0x08); //One Shot Mode

        switchToMode(OM_NORMAL);
    }
    void sendMessage(uint8_t mailBox, CanMessage& msg) {
        if(mailBox > 2) {return;}
        
        //switchToMode(OM_CONFIG);
        withCs([&] {
            uint8_t txData[14] = {CMD_LOAD_TX_BUFFER | (mailBox<<1u), 
                                (msg.id>>3)&0xFF,
                                (msg.id<<5)&0xE0,
                                0x00, 0x00,
                                msg.len, 
                                msg.data[0], msg.data[1], msg.data[2], msg.data[3],
                                msg.data[4], msg.data[5], msg.data[6], msg.data[7]};
            _spi.transmit(txData, 6+msg.len);
        });
        //switchToMode(OM_NORMAL);
        
        requestToSend(1<<mailBox);
    }
    void receiveMessage(uint8_t fifo, CanMessage& msg) {
        if(fifo > 1) {return;}
        //switchToMode(OM_CONFIG);
        uint8_t data[14] = {static_cast<uint8_t>(CMD_READ_RX_BUFFER | (fifo<<2u)), 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00 };
        /*withCs([&] {
            _spi.transmitReceive(data, data, 14);
        });*/
        
        csLow();
        _spi.transmitReceive(data, data, 14);
        uint32_t i = 0;
        while (_spi.isBusy()) {
            ++i;
            if(i > 1000) {
                while(1){}
            }
        }
        csHigh();
        
        msg.id = (data[1]<<3)|(data[2]>>5);
        msg.len = (data[5])&0x0F;
        for(uint8_t i = 0; i < msg.len; ++i) {
            msg.data[i] = data[6+i];
        }
        //switchToMode(OM_NORMAL);
    }
    void readStatus(uint8_t& value) {
        uint8_t data[3] = {CMD_READ_STATUS, 0x00, 0x00};
        withCs([&] {
            _spi.transmitReceive(data, data, 3);
        });
        value = data[1];
    }
    
private:
    SpiInterface _spi;
    GpioInterface _cs;
    uint8_t _status;

    void csLow() { _cs.reset(); }
    void csHigh() { _cs.set(); }
    
    void command(uint8_t cmd) {
        withCs([&] {
            _spi.transmit(&cmd, 1);
        });
    }

    void readRegister(uint8_t address, uint8_t& value) {
        uint8_t data[3] = {CMD_READ, address, 0x00};
        withCs([&] {
            _spi.transmitReceive(data, data, 3);
        });
        value = data[2];
    }

    void writeRegister(uint8_t address, uint8_t value) {
        uint8_t data[3] = {CMD_WRITE, address, value};
        withCs([&] {
            _spi.transmit(data, 3);
        });

    }

    void requestToSend(uint8_t bufferMask) {
        uint8_t data = CMD_RTS | (bufferMask & 0x07);
        withCs([&] {
            _spi.transmit(&data, 1);
        });
    }

    void bitModify(uint8_t address, uint8_t mask, uint8_t value) {
        uint8_t data[4] = {CMD_BIT_MODIFY, address, mask, value};
        withCs([&] {
            _spi.transmit(data, 4);
        });
    }

    void switchToMode(uint8_t mode) {
        bitModify(REG_CANCTRL, OP_MODE_MASK, mode);
        uint8_t opMode;
        do {
            readRegister(REG_CANSTAT, opMode);
        } while(opMode != mode);
    }
};