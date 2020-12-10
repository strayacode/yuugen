#pragma once
#include <nds/common/types.h>

class NDS;

class Memory {
public:
    // common between arm9 and arm7
    u8 main_ram[4 * 1024 * 1024]; // 4mb
    u8 shared_wram[32 * 1024]; // 0kb, 16kb or 32kb can be allocated to arm9 or arm7
    // TODO: add gba slot rom and ram later

    // arm9 specific
    u8 instruction_tcm[32 * 1024]; // 0x00000000-0x00008000 (non-movable 32kb) (mirrorable to 0x01000000)
    u8 data_tcm[16 * 1024]; // 16kb movable
    u8 palette_ram[2 * 1024]; // 2kb for engine A obj/bg and engine B obj/bg
    u8 bg_a_vram[512 * 1024]; // 512kb max
    u8 bg_b_vram[128 * 1024]; // 128kb max
    u8 obj_a_vram[256 * 1024]; // 256kb max
    u8 obj_b_vram[128 * 1024]; // 128kb max
    u8 main_vram[656 * 1024]; // 656kb max lcdc allocated vram
    u8 oam[2 * 1024]; // 2kb oam for both engine A and B
    
    u8 arm9_bios[32 * 1024]; // 32kb arm9 bios only 3kb used


    // arm7 specific
    u8 arm7_bios[16 * 1024]; // 16kb
    u8 arm7_wram[64 * 1024]; // 64 kb
    // TODO: add wireless communications
    u8 arm7_vram[256 * 1024]; // vram allocated as wram to arm7 max 256kb

    // external to the arm7/arm9 bus
    u8 firmware[256 * 1024]; // built-in serial flash memory

    u8 arm9_read_byte(u32 addr);
    u16 arm9_read_halfword(u32 addr);
    u32 arm9_read_word(u32 addr);
    u32 arm9_read_io(u32 addr);

    u8 arm7_read_byte(u32 addr);
    u16 arm7_read_halfword(u32 addr);
    u32 arm7_read_word(u32 addr);
    u32 arm7_read_io(u32 addr);

    void load_arm9_bios();
    void load_arm7_bios();
    void load_firmware();

    Memory(NDS *nds);
private:
    NDS *nds;

    
};