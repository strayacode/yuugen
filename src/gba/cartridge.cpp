#include <gba/cartridge.h>

GBA_Cartridge::~GBA_Cartridge() {
	// delete only if not a nullptr
	if (rom) {
		delete[] rom;
	}
}

void GBA_Cartridge::load_cartridge(std::string rom_path) {
	printf("rom path is %s\n", rom_path.c_str());
    FILE *file_buffer = fopen(rom_path.c_str(), "rb");
    if (file_buffer == NULL) {
        log_fatal("[Cartridge] error while opening the selected rom! make you sure specify the path correctly");
    }
    fseek(file_buffer, 0, SEEK_END);
    int cartridge_size = ftell(file_buffer);
    fseek(file_buffer, 0, SEEK_SET);
    rom = new u8[cartridge_size];
    fread(rom, 1, cartridge_size, file_buffer);
    fclose(file_buffer);  
    printf("[Cartridge] cartridge loaded successfully!\n");
    log_fatal("still gotta load header data or smth idk");
}