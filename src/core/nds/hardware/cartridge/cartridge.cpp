#include "common/memory.h"
#include "common/logger.h"
#include "core/nds/hardware/cartridge/cartridge.h"

namespace core::nds {

void Cartridge::load(const std::string& path) {
    memory_mapped_file.load(path);
    load_header();
}

void Cartridge::load_header() {
    memcpy(&header.game_title, memory_mapped_file.get_pointer(0), 12);

    // load the u32 variables in the header struct from the respective areas of the rom
    memcpy(&header.gamecode, memory_mapped_file.get_pointer(0x0c), 4);
    memcpy(&header.arm9_offset, memory_mapped_file.get_pointer(0x20), 4);
    memcpy(&header.arm9_entrypoint, memory_mapped_file.get_pointer(0x24), 4);
    memcpy(&header.arm9_ram_address, memory_mapped_file.get_pointer(0x28), 4);
    memcpy(&header.arm9_size, memory_mapped_file.get_pointer(0x2c), 4);
    memcpy(&header.arm7_offset, memory_mapped_file.get_pointer(0x30), 4);
    memcpy(&header.arm7_entrypoint, memory_mapped_file.get_pointer(0x34), 4);
    memcpy(&header.arm7_ram_address, memory_mapped_file.get_pointer(0x38), 4);
    memcpy(&header.arm7_size, memory_mapped_file.get_pointer(0x3c), 4);
    memcpy(&header.icon_title_offset, memory_mapped_file.get_pointer(0x68), 4);
    logger.debug("[ARM9]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm9_offset, header.arm9_entrypoint, header.arm9_ram_address, header.arm9_size);
    logger.debug("[ARM7]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm7_offset, header.arm7_entrypoint, header.arm7_ram_address, header.arm7_size);

    logger.debug("[Cartridge] Header data loaded");
}

} // namespace core::nds