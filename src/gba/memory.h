#pragma once

#include <string>
#include "common/types.h"
#include "arm/memory.h"

namespace gba {

class System;

class Memory : public arm::Memory {
public:
    Memory(System& system);

    void reset();
    
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

    void write_haltcnt(u8 value);

    u16 read_waitcnt() { return waitcnt.data; }
    void write_waitcnt(u16 value, u32 mask);

    System& system;
    std::array<u8, 0x40000> ewram;
    std::array<u8, 0x8000> iwram;
    std::array<u8, 0x4000> bios;
    u8 haltcnt;

    union WAITCNT {
        struct {
            u16 sram_wait_control : 2;
            u16 ws0_first_access : 2;
            bool ws0_second_access : 1;
            u16 ws1_first_access : 2;
            bool ws1_second_access : 1;
            u16 ws2_first_access : 2;
            bool ws2_second_access : 1;
            u16 phi_terminal_output : 2;
            u16 : 1;
            bool prefetch_buffer : 1;
            bool gamepak_type : 1;
        };

        u16 data;
    };

    WAITCNT waitcnt;
};

} // namespace gba