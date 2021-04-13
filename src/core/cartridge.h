#pragma once

#include <util/types.h>
#include <util/log.h>
#include <string.h>
#include <stdio.h>

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

        u32 icon_title_offset; // specifies the offset in the rom image to where the icon and title is
        // 0 = None
    } header;

    // struct IconTitle {
    //     u8 icon_bitmap[0x200];
    //     u8 icon_palette[0x20];
    //     // TODO: add japanese title
    //     // const char* japanese_title;
    //     // just use u16string later if possible
    //     u16 english_title_raw[0x80];
    //     std::u16string english_title;

    // } icon_title;

    // void LoadIconTitle();

    void LoadHeaderData();
    void DirectBoot();
    void LoadROM(const char* rom_path);
    void Reset();

    void WriteROMCTRL(u32 data);
    void WriteAUXSPICNT(u16 data);
    void WriteAUXSPIDATA(u16 data);

    void ReceiveCommand(u8 command, int command_index);

    u8* rom = nullptr;

    u32 ROMCTRL;
    u16 AUXSPICNT;
    u16 AUXSPIDATA;

    u8 command_buffer[8];

    Core* core;
};