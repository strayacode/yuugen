#include <common/types.h>
#include <common/log.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>
#include <iterator>

#pragma once

struct Core;

struct Cartridge {
    Cartridge(Core* core);
    void Reset();
    void LoadRom(std::string rom_path);
    void LoadHeaderData();
    void DirectBoot();

    struct CartridgeHeader {
        u32 arm9_rom_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm9_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm9_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm9_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 arm7_rom_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm7_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm7_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm7_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 icon_title_offset; // specifies the offset in the rom image to where the icon and title is
        // 0 = None
    } header;

    Core* core;

    std::vector<u8> rom;
};