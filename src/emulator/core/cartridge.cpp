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
}