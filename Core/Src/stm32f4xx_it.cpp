#include "main.h"
#include "stm32f4xx_it.h"

void NMI_Handler(void) {
   while (1) {
   }
}

void HardFault_Handler(void) {
  while (1) {
  }
}

void MemManage_Handler(void) {
  while (1) {
  }
}

void BusFault_Handler(void) {
  while (1) {
  }
}

void UsageFault_Handler(void) {
  while (1) {
  }
}

void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}

void DebugMon_Handler(void) {
  
}

void DMA1_Stream1_IRQHandler(void) {
  
}

extern "C" void DMA1_Stream3_IRQHandler(void) {
  
}

extern "C" void DMA1_Stream4_IRQHandler(void) {
  
}

extern "C" void DMA2_Stream2_IRQHandler(void) {
  DmaTxSpi1::handleInterrupt();
}

void TIM1_TRG_COM_TIM11_IRQHandler(void) {
  
}

void I2C1_EV_IRQHandler(void) {
  
}

extern "C" void SPI2_IRQHandler(void) {
  Spi2::handleInterrupt();
}
