#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/memory_helpers.h>
#include <string.h>
#include <type_traits>
#include <vector>
#include <fstream>
#include <vector>
#include <iterator>
#include <array>

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

struct SharedMemoryConfig {
    u16 mask;
    u8* base;
};

struct Memory {
    Memory(Core* core);
    void Reset();
    void DirectBoot();

    // regular memory read/write handlers
    template <typename T>
    auto ARM7Read(u32 addr) -> T;

    template <typename T>
    void ARM7Write(u32 addr, T data);

    template <typename T>
    auto ARM9Read(u32 addr) -> T;

    template <typename T>
    void ARM9Write(u32 addr, T data);

    // software fastmem read/write handlers
    template <typename T>
    auto ARM7FastRead(u32 addr) -> T;

    template <typename T>
    void ARM7FastWrite(u32 addr, T data);

    template <typename T>
    auto ARM9FastRead(u32 addr) -> T;

    template <typename T>
    void ARM9FastWrite(u32 addr, T data);

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

    void WriteWRAMCNT(u8 data);

    auto CartridgeAccessRights() -> bool;

    void MapSharedMemory(u8 data);

    void UpdateARM9MemoryMap(u32 low_addr, u32 high_addr);
    void UpdateARM7MemoryMap(u32 low_addr, u32 high_addr);

    Core* core;

    // todo: maybe make vectors later or std::array?
    u8 main_memory[0x400000] = {};
    u8 arm7_wram[0x10000] = {};
    u8 shared_wram[0x8000] = {};

    std::vector<u8> arm7_bios;
    std::vector<u8> arm9_bios;

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

    // used for protection of the arm7 bios
    u32 BIOSPROT;

    // the arm7 provides io ports for the link port but it doesn't seem to be used
    u16 SIOCNT;

    SharedMemoryConfig arm9_shared_memory_config;
    SharedMemoryConfig arm7_shared_memory_config;

    std::array<u8*, 0x100000> arm7_read_page_table;
    std::array<u8*, 0x100000> arm9_read_page_table;
    std::array<u8*, 0x100000> arm7_write_page_table;
    std::array<u8*, 0x100000> arm9_write_page_table;

    // each page is 4kb
    int static constexpr page_size = 0x1000;
};