#pragma once
#include <nds/common/types.h>

class NDS;

class Cartridge {
public:
    Cartridge(NDS *nds);

    void load_header_data();
private:
    NDS *nds;

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