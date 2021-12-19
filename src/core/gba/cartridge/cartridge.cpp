#include "core/gba/cartridge/cartridge.h"

void GBACartridge::Reset() {
    rom.clear();
    rom_size = 0;
}

void GBACartridge::LoadRom(std::string path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        log_fatal("rom with path %s does not exist!", path.c_str());
    }

    file.unsetf(std::ios::skipws);

    std::streampos size;

    file.seekg(0, std::ios::end);
    size = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve bytes for our rom vector specified by rom_size
    rom.reserve(size);

    rom.insert(rom.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());

    rom_size = rom.size();

    file.close();

    log_debug("[Cartridge] Rom data loaded");
    log_debug("[Cartridge] Size: %08lx", rom_size);

    LoadHeaderData();
}

void GBACartridge::LoadHeaderData() {
    
}

u64 GBACartridge::GetRomSize() {
    return rom_size;
}