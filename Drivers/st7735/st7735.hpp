#pragma once

#include <cstdint>
#include <functional>
#include "../Periph/spi.hpp"
#include "../Periph/gpio.hpp"

enum struct St7735DisplayRotation
{
  VERTICAL = 0,
  HORIZONTAL = 1,
  VERTICAL_INVERSE = 2,
  HORIZONTAL_INVERSE = 3
};

class ST7735
{
private:
  using Callback = void(*)();
  inline static Callback displayReadyCallback = nullptr;
  enum Commands
  {
    ST7735_NOP = 0x00,
    ST7735_SWRESET = 0x01,
    ST7735_RDDID = 0x04,
    ST7735_RDDST = 0x09,
    ST7735_SLPIN = 0x10,
    ST7735_SLPOUT = 0x11,
    ST7735_PTLON = 0x12,
    ST7735_NORON = 0x13,
    ST7735_INVOFF = 0x20,
    ST7735_INVON = 0x21,
    ST7735_DISPOFF = 0x28,
    ST7735_DISPON = 0x29,
    ST7735_CASET = 0x2A,
    ST7735_RASET = 0x2B,
    ST7735_RAMWR = 0x2C,
    ST7735_RAMRD = 0x2E,
    ST7735_PTLAR = 0x30,
    ST7735_COLMOD = 0x3A,
    ST7735_MADCTL = 0x36,
    ST7735_FRMCTR1 = 0xB1,
    ST7735_FRMCTR2 = 0xB2,
    ST7735_FRMCTR3 = 0xB3,
    ST7735_INVCTR = 0xB4,
    ST7735_DISSET5 = 0xB6,
    ST7735_PWCTR1 = 0xC0,
    ST7735_PWCTR2 = 0xC1,
    ST7735_PWCTR3 = 0xC2,
    ST7735_PWCTR4 = 0xC3,
    ST7735_PWCTR5 = 0xC4,
    ST7735_VMCTR1 = 0xC5,
    ST7735_RDID1 = 0xDA,
    ST7735_RDID2 = 0xDB,
    ST7735_RDID3 = 0xDC,
    ST7735_RDID4 = 0xDD,
    ST7735_PWCTR6 = 0xFC,
    ST7735_GMCTRP1 = 0xE0,
    ST7735_GMCTRN1 = 0xE1
  };

  template <typename Func>
  void withCs(Func &&f)
  {
    csLow();
    f();
    while (_spi.isBusy())
    {
    }
    csHigh();
  }

public:
  ST7735(SpiInterface spi, GpioInterface cs, GpioInterface dc, GpioInterface reset, bool txDma)
      : _spi(spi), _cs(cs), _dc(dc), _reset(reset), _txDma(txDma),
        _xstart(0), _ystart(0), _width(WIDTH), _height(HEIGHT), _isTransmittingBlock(0) {}

  template <typename TimerMs>
  void init(St7735DisplayRotation rotation)
  {
    _getMs = [&]
    { return TimerMs::getMs(); };
    withCs([&]
           { reset(); });
    withCs([&]
           { magicSequenceInit(); });
    withCs([&]
           { setRotation(rotation); });
  }
  void drawPixel(uint16_t x, uint16_t y, uint16_t color)
  {
    if ((x >= _width) || (y >= _height))
    {
      return;
    }
    withCs([&]
           { setAddressWindow(x, y, x+1, y+1);
            uint8_t data[] = { static_cast<uint8_t>(color >> 8), static_cast<uint8_t>(color & 0xFF) };
            sendData(data, sizeof(data)); });
  }
  void drawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
  {
    if ((x >= _width) || (y >= _height))
    {
      return;
    }
    if ((x + w - 1) >= _width)
    {
      return;
    }
    if ((y + h - 1) >= _height)
    {
      return;
    }

    _isTransmittingBlock = 1;
    csLow();
    setAddressWindow(x, y, x + w - 1, y + h - 1);
    
    if (_txDma)
    {
      sendDataDma((uint8_t *)data, sizeof(uint16_t) * w * h);
    }
    else
    {
      sendData((uint8_t *)data, sizeof(uint16_t) * w * h);
      dmaCallback();
    }
  }
  void dmaCallback()
  {
    csHigh();
    _isTransmittingBlock = 0;
    if(displayReadyCallback) {
      displayReadyCallback();
    }
  }
  static void setDisplayReadyCallback(Callback cb) {
      displayReadyCallback = cb;
  }
  int16_t getWidth() {
    return _width;
  }
  int16_t getHeight() {
    return _height;
  }

private:
  SpiInterface _spi;
  GpioInterface _cs;
  GpioInterface _dc;
  GpioInterface _reset;
  bool _txDma;
  uint8_t _xstart;
  uint8_t _ystart;
  int16_t _width;
  int16_t _height;
  bool _isTransmittingBlock;
  std::function<uint32_t()> _getMs;

  void csLow() { _cs.reset(); }
  void csHigh() { _cs.set(); }
  void delay(uint32_t ms)
  {
    uint32_t start = _getMs();
    while ((start + ms) > _getMs())
    {
    }
  }
  void reset()
  {
    _reset.reset();
    delay(5);
    _reset.set();
  }
  void sendCmd(uint8_t cmd)
  {
    _dc.reset();
    _spi.transmit(&cmd, 1);
    while (_spi.isBusy())
    {
    }
  }
  void sendData(uint8_t *buf, uint16_t size)
  {
    _dc.set();
    _spi.transmit(buf, size);
    while (_spi.isBusy())
    {
    }
  }
  void sendDataDma(uint8_t *buf, uint16_t size)
  {
    _dc.set();
    _spi.transmitDma(buf, size);
  }
  void magicSequenceInit()
  {
    const uint8_t *addr = init_sequence;
    uint8_t numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;
    while (numCommands--)
    {
      uint8_t cmd = *addr++;
      sendCmd(cmd);

      numArgs = *addr++;
      // If high bit set, delay follows args
      ms = numArgs & DELAY;
      numArgs &= ~DELAY;
      if (numArgs)
      {
        sendData((uint8_t *)addr, numArgs);
        addr += numArgs;
      }
      if (ms)
      {
        ms = *addr++;
        if (ms == 255)
          { ms = 500; }
        delay(ms);
      }
    }
  }
  void setRotation(St7735DisplayRotation rotation)
  {
    uint8_t u8rotation = static_cast<uint8_t>(rotation);
    static constexpr uint8_t madctlValue[] = {
        ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB,
        ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB,
        ST7735_MADCTL_RGB,
        ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB};
    if (u8rotation & 0x01) {
      _width = HEIGHT;
      _height = WIDTH;
    } else {
      _width = WIDTH;
      _height = HEIGHT;
    }
    uint8_t madctl = madctlValue[u8rotation];
    sendCmd(ST7735_MADCTL);
    sendData(&madctl, 1);
  }
  void setAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
  {
    sendCmd(ST7735_CASET);
    uint8_t data[] = {0x00, (uint8_t)(x0 + _xstart), 0x00, (uint8_t)(x1 + _xstart)};
    sendData(data, sizeof(data));

    sendCmd(ST7735_RASET);
    data[1] = y0 + _ystart;
    data[3] = y1 + _ystart;
    sendData(data, sizeof(data));

    sendCmd(ST7735_RAMWR);
  }
  static constexpr uint8_t DELAY = 0x80;
  static constexpr uint8_t ST7735_MADCTL_MY = 0x80;
  static constexpr uint8_t ST7735_MADCTL_MX = 0x40;
  static constexpr uint8_t ST7735_MADCTL_MV = 0x20;
  static constexpr uint8_t ST7735_MADCTL_ML = 0x10;
  static constexpr uint8_t ST7735_MADCTL_RGB = 0x00;
  static constexpr uint8_t ST7735_MADCTL_BGR = 0x08;
  static constexpr uint8_t ST7735_MADCTL_MH = 0x04;
  static constexpr uint16_t WIDTH = 128;
  static constexpr uint16_t HEIGHT = 160;
  static constexpr uint8_t init_sequence[] = {
      21,                    // 15 commands in list:
      ST7735_SWRESET, DELAY, //  1: Software reset, 0 args, w/delay
      150,                   //     150 ms delay
      ST7735_SLPOUT, DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                   //     500 ms delay
      ST7735_FRMCTR1, 3,     //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,      //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
      ST7735_FRMCTR2, 3,     //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,      //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
      ST7735_FRMCTR3, 6,     //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,      //     Dot inversion mode
      0x01, 0x2C, 0x2D,      //     Line inversion mode
      ST7735_INVCTR, 1,      //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                  //     No inversion
      ST7735_PWCTR1, 3,      //  7: Power control, 3 args, no delay:
      0xA2, 0x02, 0x84,      //     -4.6V, AUTO mode
      ST7735_PWCTR2, 1,      //  8: Power control, 1 arg, no delay:
      0xC5,                  //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
      ST7735_PWCTR3, 2,      //  9: Power control, 2 args, no delay:
      0x0A,                  //     Opamp current small
      0x00,                  //     Boost frequency
      ST7735_PWCTR4, 2,      // 10: Power control, 2 args, no delay:
      0x8A,                  //     BCLK/2, Opamp current small & Medium low
      0x2A,
      ST7735_PWCTR5, 2, // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
      ST7735_VMCTR1, 1, // 12: Power control, 1 arg, no delay:
      0x0E,
      ST7735_INVOFF, 0, // 13: Don't invert display, no args, no delay
      ST7735_COLMOD, 1, // 15: set color mode, 1 arg, no delay:
      0x05,
      ST7735_CASET, 4,    //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,         //     XSTART = 0
      0x00, 0x7F,         //     XEND = 127
      ST7735_RASET, 4,    //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,         //     XSTART = 0
      0x00, 0x7F,         //  4 commands in list:
      ST7735_GMCTRP1, 16, //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
      ST7735_GMCTRN1, 16, //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
      ST7735_NORON, DELAY,  //  3: Normal display on, no args, w/delay
      10,                   //     10 ms delay
      ST7735_DISPON, DELAY, //  4: Main screen turn on, no args w/delay
      100};
};