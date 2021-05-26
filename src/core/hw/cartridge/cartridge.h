#include <common/types.h>
#include <common/log.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>
#include <iterator>

#pragma once

enum CartridgeCommand {
    READ_HEADER = 0x00,
    FIRST_CHIP_ID = 0x90,
    DUMMY_COMMAND = 0x9F,
    READ_DATA = 0xB7,
    SECOND_CHIP_ID = 0xB8,
};

struct Core;

struct Cartridge {
    Cartridge(Core* core);
    void Reset();
    void LoadRom(std::string rom_path);
    void LoadHeaderData();
    void DirectBoot();

    void WriteROMCTRL(u32 data);
    void WriteAUXSPICNT(u16 data);
    void WriteAUXSPIDATA(u16 data);

    void ReceiveCommand(u8 command, int command_index);
    auto ReadCommand(int command_index) -> u8;
    auto ReadData() -> u32;
    void StartTransfer();

    void WriteSeed0_L(u32 data);
    void WriteSeed1_L(u32 data);

    void WriteSeed0_H(u16 data);
    void WriteSeed1_H(u16 data);

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

    u32 transfer_count;
    u32 transfer_size;

    u32 ROMCTRL;
    u16 AUXSPICNT;
    u16 AUXSPIDATA;

    u8 command_buffer[8];

    u8 command;

    Core* core;

    std::vector<u8> rom;

    u64 rom_size;

    u64 seed0;
    u64 seed1;
};