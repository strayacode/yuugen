#pragma once

#include <common/types.h>
#include <string.h>
#include <stdio.h>
#include <common/log.h>

struct Core;

struct Cartridge {
    Cartridge(Core* core);
    ~Cartridge();

    struct CartridgeHeader {
        u32 arm9_rom_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm9_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm9_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm9_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 arm7_rom_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm7_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm7_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm7_size; // specifies the amount of bytes to be transferred from the cartridge to memory
    } header;

    void LoadHeaderData();
    void DirectBoot();
    void LoadROM(const char* rom_path);
    void Reset();

    u8* rom = nullptr;

    Core* core;
};