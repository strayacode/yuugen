#include <core/core.h>
#include <core/spi.h>

SPI::SPI(Core* core) : core(core) {
    
}

void SPI::Reset() {
    memset(firmware, 0, 0x40000);

    LoadFirmware();

    SPICNT = 0;
    SPIDATA = 0;
}

void SPI::WriteSPICNT(u16 data) {
    SPICNT = (SPICNT & ~0xCF03) | (data & 0xCF03);
}

void SPI::WriteSPIDATA(u8 data) {
    // TODO: properly handle behaviour of spi
    SPIDATA = data;
}

void SPI::DirectBoot() {
    // write user settings 1 (0x70 in length) to address 0x027FFC80 in main memory
    for (u32 i = 0; i < 0x70; i++) {
        core->memory.ARM9WriteByte(0x027FFC80 + i, firmware[0x3FF00 + i]);
    }
    log_debug("firmware data transfers completed successfully!");
}

void SPI::LoadFirmware() {
    FILE* file_buffer = fopen("../firmware/firmware.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("error when opening the firmware! make sure the file firmware.bin exists in the firmware folder");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(firmware, 0x40000, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("firmware loaded successfully!");
}