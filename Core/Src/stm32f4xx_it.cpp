#include "main.h"
#include "stm32f4xx_it.h"
#include "cmsis_os.h"

extern MCP2515 ExtCan;
extern osMessageQueueId_t canMessagesQueue;

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

extern "C" void EXTI3_IRQHandler(void) {
  if (EXTI->PR & EXTI_PR_PR3) {
    EXTI->PR = EXTI_PR_PR3;
    uint8_t canStatus;
    CanMessage msg;
    ExtCan.readStatus(canStatus);
    if (canStatus & 0x03) {
      uint8_t fifoNum = (canStatus & 0x01) ? 0 : 1;
      ExtCan.receiveMessage(fifoNum, msg);
      osMessageQueuePut(canMessagesQueue, &msg, 0U, 0U);
    }
  }
}
