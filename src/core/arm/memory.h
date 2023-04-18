#pragma once

#include "common/types.h"
#include "common/memory.h"
#include "core/arm/fastmem.h"

namespace core::arm {

// this class will be used as a base class which both the arm7 and arm9 implement
class Memory {
public:
    virtual ~Memory() = default;

    template <typename T, Bus bus>
    T read(u32 addr) {
        static_assert(is_one_of_v<T, u8, u16, u32>, "T is not valid");
        return 0;
    }

    template <typename T, Bus bus>
    void write(u32 addr, T value) {
        static_assert(is_one_of_v<T, u8, u16, u32>, "T is not valid");
    }

    virtual u8 system_read_byte(u32 addr) = 0;
    virtual u16 system_read_half(u32 addr) = 0;
    virtual u32 system_read_word(u32 addr) = 0;

    virtual void system_write_byte(u32 addr, u8 value) = 0;
    virtual void system_write_half(u32 addr, u16 value) = 0;
    virtual void system_write_word(u32 addr, u32 value) = 0;
    
private:
    Fastmem fastmem;
};

} // namespace core::arm