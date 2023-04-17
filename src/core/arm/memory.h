#pragma once

#include "common/types.h"

namespace core::arm {

enum class Bus {
    // for instructions
    Code,

    // for regular data that gets read by instructions
    Data,

    // for components other than the cpu (e.g. DMA)
    System,
};

// this class will be used as a base class which both the arm7 and arm9 implement
class Memory {
public:
    template <typename T, Bus bus>
    T read(u32 addr) {
        return 0;
    }

    template <typename T, Bus bus>
    void write(u32 addr, T value) {

    }

    virtual u8 read_byte(u32 addr) = 0;
    virtual u16 read_half(u32 addr) = 0;
    virtual u32 read_word(u32 addr) = 0;

    virtual void write_byte(u32 addr, u8 value) = 0;
    virtual void write_half(u32 addr, u16 value) = 0;
    virtual void write_word(u32 addr, u32 value) = 0;
    
private:
};

} // namespace core::arm