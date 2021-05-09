#include <core/hw/cartridge/cartridge.h>
#include <core/core.h>

Cartridge::Cartridge(Core* core) : core(core) {

}

void Cartridge::Reset() {
    rom.clear();
}

void Cartridge::LoadRom(std::string rom_path) {
    std::ifstream file(rom_path, std::ios::binary);

    if (!file) {
        log_fatal("rom with path %s does not exist!", rom_path.c_str());
    }

    file.unsetf(std::ios::skipws);

    std::streampos rom_size;

    file.seekg(0, std::ios::end);
    rom_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve bytes for our rom vector specified by rom_size
    rom.reserve(rom_size);

    rom.insert(rom.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());

    LoadHeaderData();
}

void Cartridge::LoadHeaderData() {
    // load the u32 variables in the header struct from the respective areas of the rom
    memcpy(&header.arm9_rom_offset, &rom[0x20], 4);
    memcpy(&header.arm9_entrypoint, &rom[0x24], 4);
    memcpy(&header.arm9_ram_address, &rom[0x28], 4);
    memcpy(&header.arm9_size, &rom[0x2C], 4);
    memcpy(&header.arm7_rom_offset, &rom[0x30], 4);
    memcpy(&header.arm7_entrypoint, &rom[0x34], 4);
    memcpy(&header.arm7_ram_address, &rom[0x38], 4);
    memcpy(&header.arm7_size, &rom[0x3C], 4);
    memcpy(&header.icon_title_offset, &rom[0x68], 4);
    // LoadIconTitle();
    log_debug("[ARM9]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm9_rom_offset, header.arm9_entrypoint, header.arm9_ram_address, header.arm9_size);
    log_debug("[ARM7]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm7_rom_offset, header.arm7_entrypoint, header.arm7_ram_address, header.arm7_size);

    log_debug("[Cartridge] Header data loaded");
}

void Cartridge::DirectBoot() {
    // perform data transfers

    // first transfer the cartridge header (this is taken from rom address 0 and loaded into main memory at address 0x27FFE00)
    for (u32 i = 0; i < 0x170; i++) {
        core->memory.ARM9Write<u8>(0x027FFE00 + i, rom[i]);
    }

    // next transfer the arm9 code
    for (u32 i = 0; i < header.arm9_size; i++) {
        core->memory.ARM9Write<u8>(header.arm9_ram_address + i, rom[header.arm9_rom_offset + i]);
    }

    // finally transfer the arm7 code
    for (u32 i = 0; i < header.arm7_size; i++) {
        // if (header.arm7_ram_address + i < 0x037f9008) {

        //     printf("%08x = %08x\n", header.arm7_ram_address + i, rom[header.arm7_rom_offset + i]);
        // }
        core->memory.ARM7Write<u8>(header.arm7_ram_address + i, rom[header.arm7_rom_offset + i]);
    }

    log_debug("[Cartridge] Data transferred into memory");
}