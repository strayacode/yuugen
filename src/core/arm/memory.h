#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/memory_helpers.h>
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
    void DirectBoot();

    // general memory read/write handlers
    template <typename T>
    auto ARM7Read(u32 addr) -> T;

    template <typename T>
    void ARM7Write(u32 addr, T data);

    template <typename T>
    auto ARM9Read(u32 addr) -> T;

    template <typename T>
    void ARM9Write(u32 addr, T data);

    // mmio handlers
    auto ARM7ReadByteIO(u32 addr) -> u8;
    auto ARM7ReadHalfIO(u32 addr) -> u16;
    auto ARM7ReadWordIO(u32 addr) -> u32;

    void ARM7WriteByteIO(u32 addr, u8 data);
    void ARM7WriteHalfIO(u32 addr, u16 data);
    void ARM7WriteWordIO(u32 addr, u32 data);

    auto ARM9ReadByteIO(u32 addr) -> u8;
    auto ARM9ReadHalfIO(u32 addr) -> u16;
    auto ARM9ReadWordIO(u32 addr) -> u32;

    void ARM9WriteByteIO(u32 addr, u8 data);
    void ARM9WriteHalfIO(u32 addr, u16 data);
    void ARM9WriteWordIO(u32 addr, u32 data);

    // TODO: change bios to std::vector or std::array and use fstream
    void LoadARM7Bios();
    void LoadARM9Bios();

    void WriteHALTCNT(u8 data);

    Core* core;

    // todo: maybe make vectors later or std::array?
    u8 main_memory[0x400000] = {};
    u8 arm7_wram[0x10000] = {};
    u8 shared_wram[0x8000] = {};

    u8 arm7_bios[0x4000] = {};
    u8 arm9_bios[0x8000] = {};

    // used by the arm7 and arm9 to configure mapping of shared wram
    // but is only on the arm9 bus
    u8 WRAMCNT;
    u8 POWCNT2;
    u16 RCNT;

    // used to halt the arm7
    u8 HALTCNT;

    u16 EXMEMCNT;

    u8 POSTFLG7;
    u8 POSTFLG9;
};