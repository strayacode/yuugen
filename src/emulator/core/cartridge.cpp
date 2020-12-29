#include <emulator/core/cartridge.h>
#include <stdio.h>
#include <stdlib.h>
#include <emulator/emulator.h>

Cartridge::Cartridge(Emulator *emulator) : emulator(emulator) {
    
}

Cartridge::~Cartridge() {
    // check if rom is a nullptr first
    if (rom) {
        delete[] rom;
    }
}

void Cartridge::load_header_data() {
    // load the game title
    for (int i = 0; i < 12; i++) {
        header.game_title[i] = rom[i];
    }

    // load gamecode and makercode
    header.gamecode = (rom[0xC] << 24 | rom[0xD] << 16 | rom[0xE] << 8 | rom[0xF]);
    header.makercode = (rom[0x10] << 8 | rom[0x11]);

    // load arm9 stuff
    header.arm9_rom_offset = (rom[0x23] << 24 | rom[0x22] << 16 | rom[0x21] << 8 | rom[0x20]);
    header.arm9_entry_address = (rom[0x27] << 24 | rom[0x26] << 16 | rom[0x25] << 8 | rom[0x24]);
    header.arm9_ram_address = (rom[0x2B] << 24 | rom[0x2A] << 16 | rom[0x29] << 8 | rom[0x28]);
    header.arm9_size = (rom[0x2F] << 24 | rom[0x2E] << 16 | rom[0x2D] << 8 | rom[0x2C]);

    header.arm7_rom_offset = (rom[0x33] << 24 | rom[0x32] << 16 | rom[0x31] << 8 | rom[0x30]);
    header.arm7_entry_address = (rom[0x37] << 24 | rom[0x36] << 16 | rom[0x35] << 8 | rom[0x34]);
    header.arm7_ram_address = (rom[0x3B] << 24 | rom[0x3A] << 16 | rom[0x39] << 8 | rom[0x38]);
    header.arm7_size = (rom[0x3F] << 24 | rom[0x3E] << 16 | rom[0x3D] << 8 | rom[0x3C]);
    printf("arm9 offset: 0x%08x arm9 entry address: 0x%08x arm9 ram address: 0x%08x arm9 size: 0x%08x\n", header.arm9_rom_offset, header.arm9_entry_address, header.arm9_ram_address, header.arm9_size);
    printf("arm7 offset: 0x%08x arm7 entry address: 0x%08x arm7 ram address: 0x%08x arm7 size: 0x%08x\n", header.arm7_rom_offset, header.arm7_entry_address, header.arm7_ram_address, header.arm7_size);

    printf("[Cartridge] header data loaded successfully!\n");
}

void Cartridge::direct_boot() {
    // first transfer header data to main memory
    for (u32 i = 0; i < 0x170; i++) {
        emulator->memory.arm9_write_byte(0x027FFE00 + i, rom[i]);
    }
    
    // transfer arm9 code
    for (u32 i = 0; i < header.arm9_size; i++) { 
        emulator->memory.arm9_write_byte(header.arm9_entry_address + i, rom[header.arm9_rom_offset + i]);
    }
    
    // transfer arm7 code
    for (u32 i = 0; i < header.arm7_size; i++) {
        emulator->memory.arm7_write_byte(header.arm7_entry_address + i, rom[header.arm7_rom_offset + i]);
    }
}

void Cartridge::load_cartridge(std::string rom_path) {
    FILE *file_buffer = fopen(rom_path.c_str(), "rb");
    if (file_buffer == NULL) {
        printf("[Cartridge] error while opening the selected rom! make you sure specify the path correctly\n");
        emulator->running = false;
    }
    fseek(file_buffer, 0, SEEK_END);
    int cartridge_size = ftell(file_buffer);
    fseek(file_buffer, 0, SEEK_SET);
    rom = new u8[cartridge_size];
    fread(rom, 1, cartridge_size, file_buffer);
    fclose(file_buffer);  
    printf("[Cartridge] cartridge loaded successfully!\n");
    load_header_data();
}