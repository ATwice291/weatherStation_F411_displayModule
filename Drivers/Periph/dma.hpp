#pragma once

enum class DmaController {Dma1, Dma2};
enum class DmaStream {S0, S1, S2, S3, S4, S5, S6, S7};
enum class DmaChannel {CH0, CH1, CH2, CH3, CH4, CH5, CH6, CH7}; 

enum class DmaBurst {No, Incr4, Incr8, Incr16};
enum class DmaDataSize {Byte, HalfWord, Word};
enum class DmaDirection {PerifToMem, MemToPerif, MemToMem};
enum class DmaPriority {Low, Medium, High, VeryHigh};

template <DmaController controller,
          DmaStream stream,
          DmaChannel channel,
          DmaDirection DIR = DmaDirection::PerifToMem,
          bool CIRC = false,
          bool PINC = false,
          bool MINC = false,
          DmaDataSize PSIZE = DmaDataSize::Byte,
          DmaDataSize MSIZE = DmaDataSize::Byte,
          DmaPriority PL = DmaPriority::Low,
          DmaBurst PBURST = DmaBurst::No,
          DmaBurst MBURST = DmaBurst::No>
struct DmaParams {
    static constexpr DmaController getController() { return controller; }
    static constexpr DmaStream getStream() { return stream; }
    static constexpr DmaChannel getChannel() { return channel; }
    static constexpr DmaDirection getDirection() { return DIR; }
    static constexpr bool isCircular() { return CIRC; }
    static constexpr bool isPeripheralIncrement() { return PINC; }
    static constexpr bool isMemoryIncrement() { return MINC; }
    static constexpr DmaDataSize getPeripheralSize() { return PSIZE; }
    static constexpr DmaDataSize getMemorySize() { return MSIZE; }
    static constexpr DmaPriority getPriority() { return PL; }
    static constexpr DmaBurst getPeripheralBurst() { return PBURST; }
    static constexpr DmaBurst getMemoryBurst() { return MBURST; }
};

#if defined(STM32F4)
#include "dma_f4.hpp"

#elif defined(STM32F1)
#include "dma_f1.hpp"

#else
#error "Unsupported MCU family"
#endif