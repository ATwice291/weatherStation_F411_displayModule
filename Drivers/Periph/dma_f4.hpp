#pragma once

#include <cstdint>
#include "stm32f4xx.h"
#include <cassert>
#include <array>

template<typename params>
class Dma {
    using Callback = void(*)();
private:
    inline static Callback transferCompleteCallback = nullptr;
    inline static Callback transferErrorCallback = nullptr;

    static constexpr DMA_TypeDef* getControllerInstance() {
        if constexpr(params::getController() == DmaController::Dma1) {return DMA1;}
        if constexpr(params::getController() == DmaController::Dma2) {return DMA2;}
    }
    static inline void enableClock() {
        if constexpr(params::getController() == DmaController::Dma1) {RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;}
        if constexpr(params::getController() == DmaController::Dma2) {RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;}
    }
    static inline void disableClock() {
        if constexpr(params::getController() == DmaController::Dma1) {RCC->AHB1ENR &= (~RCC_AHB1ENR_DMA1EN);}
        if constexpr(params::getController() == DmaController::Dma2) {RCC->AHB1ENR &= (~RCC_AHB1ENR_DMA2EN);}
    }
    static constexpr DMA_Stream_TypeDef* getStreamInstance() {
        if constexpr(params::getController() == DmaController::Dma1) {
            constexpr std::array<DMA_Stream_TypeDef*, 8> streams = {
                DMA1_Stream0, DMA1_Stream1, DMA1_Stream2, DMA1_Stream3, DMA1_Stream4, DMA1_Stream5, DMA1_Stream6, DMA1_Stream7};
            return streams[static_cast<uint32_t>(params::getStream())];
        }
        if constexpr(params::getController() == DmaController::Dma2) {
            constexpr std::array<DMA_Stream_TypeDef*, 8> streams = {
                DMA2_Stream0, DMA2_Stream1, DMA2_Stream2, DMA2_Stream3, DMA2_Stream4, DMA2_Stream5, DMA2_Stream6, DMA2_Stream7};
            return streams[static_cast<uint32_t>(params::getStream())];
        }
    }
    static constexpr uint32_t getCRConfig() {
        constexpr uint32_t cr = 
            (static_cast<uint32_t>(params::getDirection()) << DMA_SxCR_DIR_Pos) |
            (params::isCircular()?DMA_SxCR_CIRC:0) |
            (params::isPeripheralIncrement()?DMA_SxCR_PINC:0) |
            (params::isMemoryIncrement()?DMA_SxCR_MINC:0) |
            (static_cast<uint32_t>(params::getPeripheralSize()) << DMA_SxCR_PSIZE_Pos) |
            (static_cast<uint32_t>(params::getMemorySize()) << DMA_SxCR_MSIZE_Pos) |
            (static_cast<uint32_t>(params::getPriority()) << DMA_SxCR_PL_Pos) |
            (static_cast<uint32_t>(params::getPeripheralBurst()) << DMA_SxCR_PBURST_Pos) |
            (static_cast<uint32_t>(params::getMemoryBurst()) << DMA_SxCR_MBURST_Pos) |
            (static_cast<uint32_t>(params::getChannel()) << DMA_SxCR_CHSEL_Pos);
        return cr;
    }
public:
    static void init() {
        enableClock();
        getStreamInstance()->CR = getCRConfig();
        //transferCompleteCallback = nullptr;
        //transferErrorCallback = nullptr;
    }
    static void stop() {
        getStreamInstance()->CR &= (~DMA_SxCR_EN);
        while(getStreamInstance()->CR & DMA_SxCR_EN) {}
    }
    static void configure(uint32_t src, uint32_t dest, uint16_t size) {
        if constexpr(params::getDirection() == DmaDirection::MemToPerif) {
            getStreamInstance()->M0AR = src;
            getStreamInstance()->PAR = dest;
        } else {
            getStreamInstance()->PAR = src;
            getStreamInstance()->M0AR = dest;
        }
        getStreamInstance()->NDTR = size;
    }
    static void start() {
        getStreamInstance()->CR |= DMA_SxCR_EN;
    }
    static void setTransferCompleteCallback(Callback cb) {
        transferCompleteCallback = cb;
    }
    static void setTransferErrorCallback(Callback cb) {
        transferErrorCallback = cb;
    }
    static void enableInterrupts() {
        getStreamInstance()->CR |= DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE;
    }
    static void disableInterrupts() {
        getStreamInstance()->CR &= ~(DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
    }
    static void handleInterrupt() {
        if constexpr (params::getStream() == DmaStream::S0) {
            if(getControllerInstance()->LISR & DMA_LISR_TCIF0) {
                getControllerInstance()->LIFCR |= DMA_LIFCR_CTCIF0;
                if (transferCompleteCallback) {
                    transferCompleteCallback();
                }
            }
            if(getControllerInstance()->LISR & DMA_LISR_TEIF0) {
                getControllerInstance()->LIFCR |= DMA_LIFCR_CTEIF0;
                if (transferErrorCallback) {
                    transferErrorCallback();
                }
            }
        }
        if constexpr (params::getStream() == DmaStream::S1) {
            if(getControllerInstance()->LISR & DMA_LISR_TCIF1) {
                getControllerInstance()->LIFCR |= DMA_LIFCR_CTCIF1;
                if (transferCompleteCallback) {
                    transferCompleteCallback();
                }
            }
            if(getControllerInstance()->LISR & DMA_LISR_TEIF1) {
                getControllerInstance()->LIFCR |= DMA_LIFCR_CTEIF1;
                if (transferErrorCallback) {
                    transferErrorCallback();
                }
            }
        }
        if constexpr (params::getStream() == DmaStream::S2) {
            if(getControllerInstance()->LISR & DMA_LISR_TCIF2) {
                getControllerInstance()->LIFCR |= DMA_LIFCR_CTCIF2;
                if (transferCompleteCallback) {
                    transferCompleteCallback();
                }
            }
            if(getControllerInstance()->LISR & DMA_LISR_TEIF2) {
                getControllerInstance()->LIFCR |= DMA_LIFCR_CTEIF2;
                if (transferErrorCallback) {
                    transferErrorCallback();
                }
            }
        }
        if constexpr (params::getStream() == DmaStream::S3) {
            if(getControllerInstance()->LISR & DMA_LISR_TCIF3) {
                getControllerInstance()->LIFCR |= DMA_LIFCR_CTCIF3;
                if (transferCompleteCallback) {
                    transferCompleteCallback();
                }
            }
            if(getControllerInstance()->LISR & DMA_LISR_TEIF3) {
                getControllerInstance()->LIFCR |= DMA_LIFCR_CTEIF3;
                if (transferErrorCallback) {
                    transferErrorCallback();
                }
            }
        }
        if constexpr (params::getStream() == DmaStream::S4) {
            if(getControllerInstance()->HISR & DMA_HISR_TCIF4) {
                getControllerInstance()->HIFCR |= DMA_HIFCR_CTCIF4;
                if (transferCompleteCallback) {
                    transferCompleteCallback();
                }
            }
            if(getControllerInstance()->HISR & DMA_HISR_TEIF4) {
                getControllerInstance()->HIFCR |= DMA_HIFCR_CTEIF4;
                if (transferErrorCallback) {
                    transferErrorCallback();
                }
            }
        }
        if constexpr (params::getStream() == DmaStream::S5) {
            if(getControllerInstance()->HISR & DMA_HISR_TCIF5) {
                getControllerInstance()->HIFCR |= DMA_HIFCR_CTCIF5;
                if (transferCompleteCallback) {
                    transferCompleteCallback();
                }
            }
            if(getControllerInstance()->HISR & DMA_HISR_TEIF5) {
                getControllerInstance()->HIFCR |= DMA_HIFCR_CTEIF5;
                if (transferErrorCallback) {
                    transferErrorCallback();
                }
            }
        }
        if constexpr (params::getStream() == DmaStream::S6) {
            if(getControllerInstance()->HISR & DMA_HISR_TCIF6) {
                getControllerInstance()->HIFCR |= DMA_HIFCR_CTCIF6;
                if (transferCompleteCallback) {
                    transferCompleteCallback();
                }
            }
            if(getControllerInstance()->HISR & DMA_HISR_TEIF6) {
                getControllerInstance()->HIFCR |= DMA_HIFCR_CTEIF6;
                if (transferErrorCallback) {
                    transferErrorCallback();
                }
            }
        }
        if constexpr (params::getStream() == DmaStream::S7) {
            if(getControllerInstance()->HISR & DMA_HISR_TCIF7) {
                getControllerInstance()->HIFCR |= DMA_HIFCR_CTCIF7;
                if (transferCompleteCallback) {
                    transferCompleteCallback();
                }
            }
            if(getControllerInstance()->HISR & DMA_HISR_TEIF7) {
                getControllerInstance()->HIFCR |= DMA_HIFCR_CTEIF7;
                if (transferErrorCallback) {
                    transferErrorCallback();
                }
            }
        }
    }
};