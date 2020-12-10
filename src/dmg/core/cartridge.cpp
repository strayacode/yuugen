#include <dmg/core/cartridge.h>
#include <stdio.h>

static const int BOOTROM_SIZE = 0xFF;

void DMGCartridge::load_bootrom() {
    FILE *file_buffer = fopen("../bootrom/bootrom.bin", "rb");
    if (file_buffer == NULL) {
        printf("[Cartridge] error when opening bootrom! make sure the file bootrom.bin exists in the bootrom folder\n");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(rom, BOOTROM_SIZE, 1, file_buffer);
    fclose(file_buffer);  
    printf("[Cartridge] bootrom loaded successfully!\n");
}