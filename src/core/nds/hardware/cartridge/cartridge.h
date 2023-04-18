#pragma once

#include <string>
#include "common/memory_mapped_file.h"

namespace core::nds {

class Cartridge {
public:
    void load(const std::string& path);

private:
    void load_header();

    struct Header {
        char game_title[12];

        u32 arm9_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm9_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm9_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm9_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 arm7_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm7_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm7_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm7_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 icon_title_offset; // specifies the offset in the rom image to where the icon and title is
        
        // used to identify the backup type
        u32 gamecode;
    };

    Header header;
    common::MemoryMappedFile memory_mapped_file;
};

} // namespace core::nds