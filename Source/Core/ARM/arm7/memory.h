#pragma once

#include "Common/Types.h"
#include "Common/Log.h"
#include "Core/ARM/MemoryBase.h"

class System;

class ARM7Memory : public MemoryBase {
public:
    ARM7Memory(System& system);

    void reset();
    void update_memory_map(u32 low_addr, u32 high_addr);

    u8 slow_read_byte(u32 addr) override;
    u16 slow_read_half(u32 addr) override;
    u32 slow_read_word(u32 addr) override;

    void slow_write_byte(u32 addr, u8 data) override;
    void slow_write_half(u32 addr, u16 data) override;
    void slow_write_word(u32 addr, u32 data) override;

private:
    template <typename T>
    T slow_read(u32 addr);

    template <typename T>
    void slow_write(u32 addr, T data);

    void build_mmio();

    System& system;
    std::array<u8, 0x4000> bios;
    u8 arm7_wram[0x10000] = {};
};