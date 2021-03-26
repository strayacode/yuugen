#pragma once

#include <util/types.h>
#include <util/log.h>
#include <util/memory_helpers.h>
#include <stdlib.h>
#include <string.h>


enum MemoryRegion {
    REGION_ARM7_BIOS = 0x00,
    REGION_MAIN_MEMORY = 0x02,
    REGION_SHARED_WRAM = 0x03,
    REGION_IO = 0x04,
    REGION_PALETTE_RAM = 0x05,
    REGION_VRAM = 0x06,
    REGION_OAM = 0x07,
    REGION_GBA_ROM_L = 0x08,
    REGION_GBA_ROM_H = 0x09,
    REGION_GBA_RAM = 0x0A,
    REGION_ARM9_BIOS = 0xFF,
};

struct Core;

struct Memory {
    Memory(Core* core);

    void Reset();

    // generic memory read and write handlers
    u8 ARM7ReadByte(u32 addr);
    u16 ARM7ReadHalfword(u32 addr);
    u32 ARM7ReadWord(u32 addr);
    void ARM7WriteByte(u32 addr, u8 data);
    void ARM7WriteHalfword(u32 addr, u16 data);
    void ARM7WriteWord(u32 addr, u32 data);

    u8 ARM9ReadByte(u32 addr);
    u16 ARM9ReadHalfword(u32 addr);
    u32 ARM9ReadWord(u32 addr);
    void ARM9WriteByte(u32 addr, u8 data);
    void ARM9WriteHalfword(u32 addr, u16 data);
    void ARM9WriteWord(u32 addr, u32 data);

    void LoadARM7BIOS();
    void LoadARM9BIOS();

    u8 main_memory[0x400000] = {};
    u8 arm7_wram[0x10000] = {};
    u8 shared_wram[0x8000] = {};

    u8 arm7_bios[0x4000] = {};
    u8 arm9_bios[0x8000] = {};

    Core* core;

    u8 WRAMCNT;

    // used for enabling sound speakers and wifi 
    // TODO: put in a better place
    u8 POWCNT2;

    // used to halt the arm7
    u8 HALTCNT;

    // not too sure what this is lol
    u16 RCNT;

    // not too sure what everything in this does
    u16 EXMEMCNT;

    void WriteHALTCNT(u8 data);
};