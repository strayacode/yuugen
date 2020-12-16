#pragma once
#include <emulator/common/types.h>
#include <string>

class Emulator;

class Cartridge {
public:
    Cartridge(Emulator *emulator);
    ~Cartridge();
    void load_header_data();
    void load_cartridge(std::string rom_path);

    std::string nds_rom_name;

private:
    Emulator *emulator;

    // allocate dynamically using malloc() to not use 4gb :joy:
    u8 *rom;


    struct cartridge_header {
        char game_title[12];
        u32 gamecode; // this is 0 on homebrew
        u16 makercode; // this is 0 on homebrew and 01 on nintendo
        u32 arm9_rom_offset;
        u32 arm9_entry_address;
        u32 arm9_ram_address;
        u32 arm9_size;
        u32 arm7_rom_offset;
        u32 arm7_entry_address;
        u32 arm7_ram_address;
        u32 arm7_size;
    } header;

};