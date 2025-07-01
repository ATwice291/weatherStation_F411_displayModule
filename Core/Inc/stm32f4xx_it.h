#ifndef __IT_H__
#define __IT_H__
extern "C" {

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void Error_Handler(void);

void DebugMon_Handler(void);
void DMA1_Stream1_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);
void DMA1_Stream4_IRQHandler(void);
void DMA2_Stream2_IRQHandler(void);
void TIM1_TRG_COM_TIM11_IRQHandler(void);
void I2C1_EV_IRQHandler(void);
void SPI2_IRQHandler(void);
}

#endif /*__IT_H__*/