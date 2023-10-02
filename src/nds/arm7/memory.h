#pragma once

#include <string>
#include "common/types.h"
#include "arm/memory.h"

namespace nds {

class System;

class ARM7Memory : public arm::Memory {
public:
    ARM7Memory(System& system);

    void reset();
    void update_wram_mapping();

    u8 read_byte(u32 addr) override;
    u16 read_half(u32 addr) override;
    u32 read_word(u32 addr) override;

    void write_byte(u32 addr, u8 value) override;
    void write_half(u32 addr, u16 value) override;
    void write_word(u32 addr, u32 value) override;

private:
    u8 mmio_read_byte(u32 addr);
    u16 mmio_read_half(u32 addr);
    u32 mmio_read_word(u32 addr);

    void mmio_write_byte(u32 addr, u8 value);
    void mmio_write_half(u32 addr, u16 value);
    void mmio_write_word(u32 addr, u32 value);

    template <u32 mask>
    u32 mmio_read_word(u32 addr);

    template <u32 mask>
    void mmio_write_word(u32 addr, u32 value);

    int get_access_size(u32 mask);
    u32 get_access_offset(u32 mask);

    void load_bios(const std::string& path);

    u8 read_postflg() { return postflg; }
    void write_postflg(u8 value);

    System& system;
    std::array<u8, 0x10000> arm7_wram;
    std::array<u8, 0x4000> bios;
    u16 rcnt;
    u8 postflg;
};

} // namespace nds