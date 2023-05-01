#pragma once

#include "common/types.h"
#include "core/arm/memory.h"

namespace core::nds {

class System;

class ARM9Memory : public arm::Memory {
public:
    ARM9Memory(System& system);

    void reset();
    void update_memory_map();

    u8 read_byte(u32 addr) override;
    u16 read_half(u32 addr) override;
    u32 read_word(u32 addr) override;

    void write_byte(u32 addr, u8 value) override;
    void write_half(u32 addr, u16 value) override;
    void write_word(u32 addr, u32 value) override;

private:
    template <u32 mask>
    u32 read_word(u32 addr);

    template <u32 mask>
    void write_word(u32 addr, u32 value);

    System& system;
};

} // namespace core::nds