#pragma once

#include <cstdint>
#include "stm32f4xx.h"
#include <cassert>

#if !defined  (HSI_VALUE)
  #define HSI_VALUE    ((uint32_t)16000000)
#endif /* HSI_VALUE */

enum struct SysClockSource {HSI, HSE, PLL_HSI, PLL_HSE};

template <uint32_t m, uint32_t n, uint32_t p, uint32_t q>
struct PllConfig {
    static_assert(m >= 2 && m <= 63, "Invalid M value");
    static_assert(n >= 50 && n <= 432, "Invalid N value");
    static_assert(p == 2 || p == 4 || p == 6 || p == 8, "Invalid P value");
    static_assert(q >= 2 && q <= 15, "Invalid Q value");

    static constexpr uint32_t M() {return m;}
    static constexpr uint32_t N() {return n;}
    static constexpr uint32_t P() {return p;}
    static constexpr uint32_t Q() {return q;}
};

template <uint32_t ahb, uint32_t apb1, uint32_t apb2>
struct Prescalers {
    static_assert(ahb==1||ahb==2||ahb==4||ahb==8||ahb==16||ahb==64||ahb==128||ahb==256||ahb==512, "Invalid AHB value");
    static_assert(apb1==1||apb1==2||apb1==4||apb1==8||apb1==16, "Invalid APB1 value");
    static_assert(apb2==1||apb2==2||apb2==4||apb2==8||apb2==16, "Invalid APB2 value");

    static constexpr uint32_t AHB() {return ahb;}
    static constexpr uint32_t APB1() {return apb1;}
    static constexpr uint32_t APB2() {return apb2;}
};

using defaultPsc = Prescalers<1,2,1>;
using defaultPll = PllConfig<8, 100, 2, 2>;

template <SysClockSource ClockSource, uint32_t hse = 8000000, typename PllConfigType = defaultPll, typename PrescalersType = defaultPsc>
struct ClockParams {
    using pll = PllConfigType;
    using psc = PrescalersType;

    static constexpr SysClockSource getClkSource() {return ClockSource;}
    static constexpr uint32_t getHSE() {return hse;}
};

template<typename params>
class SysClock {
    
    using Pll = typename params::pll;
    using Psc = typename params::psc;
private:
    static inline void enableHSI() {
        RCC->CR |= RCC_CR_HSION;
        while (!(RCC->CR & RCC_CR_HSIRDY)) {}
    }
    static inline void disableHSI() {
        RCC->CR &= (~RCC_CR_HSION);
        while ((RCC->CR & RCC_CR_HSIRDY)) {}
    }
    static inline void enableHSE() {
        RCC->CR |= RCC_CR_HSEON;
        while (!(RCC->CR & RCC_CR_HSERDY)) {}
    }
    static inline void disableHSE() {
        RCC->CR &= (~RCC_CR_HSEON);
        while ((RCC->CR & RCC_CR_HSERDY)) {}
    }
    static inline void enablePLL(uint32_t pllSrc, uint32_t M, uint32_t N, uint32_t P, uint32_t Q) {
        disablePLL();
        RCC->PLLCFGR = ((pllSrc == RCC_PLLCFGR_PLLSRC_HSE) ? RCC_PLLCFGR_PLLSRC_HSE : RCC_PLLCFGR_PLLSRC_HSI) | 
                       (M << RCC_PLLCFGR_PLLM_Pos) |
                       (N << RCC_PLLCFGR_PLLN_Pos) |
                       ((P / 2 - 1) << RCC_PLLCFGR_PLLP_Pos) |
                       (Q << RCC_PLLCFGR_PLLQ_Pos);
        
        RCC->CR |= RCC_CR_PLLON;
        while (!(RCC->CR & RCC_CR_PLLRDY)) {} 
    }
    static inline void disablePLL() {
        RCC->CR &= (~RCC_CR_PLLON);
        while ((RCC->CR & RCC_CR_PLLRDY)) {} 
    }
    static inline uint32_t calcNewFlashLatency() {
        return getSysClock()/30000000 - (getSysClock()%30000000==0?1:0);
    }
    static inline uint32_t getFlashLatency() {
        return (FLASH->ACR & FLASH_ACR_LATENCY);
    }
    static inline void setFlashLatency(uint32_t latency) {
        FLASH->ACR &= (~FLASH_ACR_LATENCY);
        FLASH->ACR |= (latency & FLASH_ACR_LATENCY);
    }
    static inline void switchSysClock(uint32_t source) {
        uint32_t oldLatency = getFlashLatency();
        uint32_t newLatency = calcNewFlashLatency();
        if(newLatency > oldLatency) {
            setFlashLatency(newLatency);
        }
        RCC->CFGR &= (~RCC_CFGR_SW);
        RCC->CFGR |= source;
        while ((RCC->CFGR & RCC_CFGR_SWS) != (source << RCC_CFGR_SWS_Pos)) {}
        if(newLatency < oldLatency) {
            setFlashLatency(newLatency);
        }
    }
    static inline void setPrescalers() {
        RCC->CFGR &= (~RCC_CFGR_PPRE1);
        if constexpr(Psc::APB1() == 1) {
            RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;
        } else if constexpr(Psc::APB1() == 2) {
            RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
        } else if constexpr(Psc::APB1() == 4) {
            RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
        } else if constexpr(Psc::APB1() == 8) {
            RCC->CFGR |= RCC_CFGR_PPRE1_DIV8;
        } else if constexpr(Psc::APB1() == 16) {
            RCC->CFGR |= RCC_CFGR_PPRE1_DIV16;
        } else {
            RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;
            //static_assert(false, "Incorrect APB1 prescaler");
        }
        RCC->CFGR &= (~RCC_CFGR_PPRE2);
        if constexpr(Psc::APB2() == 1) {
            RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
        } else if constexpr(Psc::APB2() == 2) {
            RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
        } else if constexpr(Psc::APB2() == 4) {
            RCC->CFGR |= RCC_CFGR_PPRE2_DIV4;
        } else if constexpr(Psc::APB2() == 8) {
            RCC->CFGR |= RCC_CFGR_PPRE2_DIV8;
        } else if constexpr(Psc::APB2() == 16) {
            RCC->CFGR |= RCC_CFGR_PPRE2_DIV16;
        } else {
            RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
            //static_assert(false, "Incorrect APB2 prescaler");
        }
    }

public:

    static constexpr uint32_t getSysClock() {
        if constexpr (params::getClkSource() == SysClockSource::HSI) {
            return HSI_VALUE;
        }
        if constexpr (params::getClkSource() == SysClockSource::HSE) {
            static_assert(params::getHSE() != 0, "Something wrong: SysClock must not be equals to 0");
            return params::getHSE();
        }
        if constexpr (params::getClkSource() == SysClockSource::PLL_HSI) {
            return (HSI_VALUE / Pll::M()) * Pll::N() / Pll::P();
        }
        if constexpr (params::getClkSource() == SysClockSource::PLL_HSE) {
            static_assert(params::getHSE() != 0, "Something wrong: SysClock must not be equals to 0");
            return (params::getHSE() / Pll::M()) * Pll::N() / Pll::P();
        }
        return 0;
    }

    static void init() {
        setPrescalers();
        if constexpr (params::getClkSource() == SysClockSource::HSI) {
            enableHSI();
            switchSysClock(RCC_CFGR_SW_HSI);
            disableHSE();
            disablePLL();
        }
        if constexpr (params::getClkSource() == SysClockSource::HSE) {
            enableHSE();
            switchSysClock(RCC_CFGR_SW_HSE);
            disableHSI();
            disablePLL();
        }
        if constexpr (params::getClkSource() == SysClockSource::PLL_HSI) {
            enableHSI();
            enablePLL(RCC_PLLCFGR_PLLSRC_HSI, Pll::M(), Pll::N(), Pll::P(), Pll::Q());
            switchSysClock(RCC_CFGR_SW_PLL);
            disableHSE();
        }
        if constexpr (params::getClkSource() == SysClockSource::PLL_HSE) {
            enableHSE();
            enablePLL(RCC_PLLCFGR_PLLSRC_HSE, Pll::M(), Pll::N(), Pll::P(), Pll::Q());
            switchSysClock(RCC_CFGR_SW_PLL);
            disableHSI();
        }
    }
    static constexpr uint32_t getAHBClock() {
        return getSysClock() / Psc::AHB();
    }
    static constexpr uint32_t getAPB1Clock() {
        return getSysClock() / Psc::APB1();
    }
    static constexpr uint32_t getAPB2Clock() {
        return getSysClock() / Psc::APB2();
    }
};