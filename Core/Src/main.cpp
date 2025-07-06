#include "main.h"
#include <stdio.h>

#include "cmsis_os.h"

MCP2515 ExtCan(Spi2::getInterface(), Mcp2515_CS::getInterface());
ST7735 display(Spi1::getInterface(), Display_CS::getInterface(), Display_DC::getInterface(), Display_RESET::getInterface(), true);

int __io_putchar(int ch) {
  ITM_SendChar((uint32_t)ch);
  return 0;
}

extern uint32_t SystemCoreClock;
extern void MX_FREERTOS_Init(void);

volatile unsigned long taskTime;

void spi1_init();
void spi2_init();
void dmaTxComplete();

void spi2_txComplete();
void spi2_rxComplete();
void dmaSpi2_txComplete();
void dmaSpi2_rxComplete();

int main(void) {
  SystemClock::init();
  SystemCoreClock = SystemClock::getSysClock();
  //constexpr uint32_t apb1 = SystemClockPLLHSE::getAPB1Clock();
  Led1::init();
  Led2::init();
  Led3::init();
  
  Mcp2515_CS::init();
  Mcp2515_CS::set();

  Display_CS::init();
  Display_CS::set();
  Display_DC::init();
  Display_DC::set();
  Display_RESET::init();
  Display_RESET::set();
  spi1_init();
  spi2_init();

  //ExtCan.init<MyCanParams>();

  __enable_irq();

  osKernelInitialize();
  MX_FREERTOS_Init();
  osKernelStart();
  while (1) {
  }
}
void spi1_init() {
  Spi1_SCK::init();
  Spi1_MOSI::init();

  Spi1::init();
  Spi1::enable();
  
  NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  NVIC_SetPriority(DMA2_Stream2_IRQn, 2);

  DmaTxSpi1::init();
  DmaTxSpi1::setTransferCompleteCallback(dmaTxComplete);
  DmaTxSpi1::enableInterrupts();
}

void spi2_init() {
  Spi2_SCK::init();
  Spi2_MISO::init();
  Spi2_MOSI::init();

  Spi2::init();
  Spi2::enable();
  
  MCP2515_INT::init();
  MCP2515_INT_EXTI::init(false, true);
  MCP2515_INT_EXTI::interruptEnable();
  NVIC_EnableIRQ(EXTI3_IRQn);
  NVIC_SetPriority(EXTI3_IRQn, 6);
}

void dmaTxComplete() {
  display.dmaCallback();
}