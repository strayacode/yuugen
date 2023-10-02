#pragma once

#include <string>
#include "common/memory.h"
#include "common/memory_mapped_file.h"

namespace gba {

class Cartridge {
public:
    void reset();
    void load(const std::string& path);

    u8* get_rom_pointer() {
        return memory_mapped_file.get_pointer(0);
    }
    
private:
    common::MemoryMappedFile memory_mapped_file;
};

} // namespace gba