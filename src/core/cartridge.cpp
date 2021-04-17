#include <core/cartridge.h>
#include <core/core.h>

Cartridge::Cartridge(Core* core) : core(core) {

}

Cartridge::~Cartridge() {
    // delete rom only if not a nullptr
    if (rom) {
        delete[] rom;
    }
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
    log_debug("arm9 offset: 0x%08x arm9 entrypoint: 0x%08x arm9 ram address: 0x%08x arm9 size: 0x%08x", header.arm9_rom_offset, header.arm9_entrypoint, header.arm9_ram_address, header.arm9_size);
    log_debug("arm7 offset: 0x%08x arm7 entrypoint: 0x%08x arm7 ram address: 0x%08x arm7 size: 0x%08x", header.arm7_rom_offset, header.arm7_entrypoint, header.arm7_ram_address, header.arm7_size);

    log_debug("header data loaded successfully!");
}

// void Cartridge::LoadIconTitle() {
//     if (header.icon_title_offset != 0) {
//         memcpy(icon_title.icon_bitmap, &rom[header.icon_title_offset + 0x20], 0x200);
//         memcpy(icon_title.icon_palette, &rom[header.icon_title_offset + 0x220], 0x20);
//         for (int i = 0; i < 0x80; i += 2) {
//             icon_title.english_title_raw[i] = (rom[header.icon_title_offset + 0x340 + i + 1] << 8) | (rom[header.icon_title_offset + 0x340 + i]);
//         }
//         icon_title.english_title.assign(icon_title.english_title_raw, icon_title.english_title_raw + 0x80);
//     }
// }

void Cartridge::LoadROM(const char* rom_path) {
    FILE* buffer = fopen(rom_path, "rb");
    if (buffer == NULL) {
        log_fatal("file with path %s was not found!", rom_path);
    }

    fseek(buffer, 0, SEEK_END);
    int rom_size = ftell(buffer);
    fseek(buffer, 0, SEEK_SET);
    rom = new u8[rom_size];
    fread(rom, 1, rom_size, buffer);
    fclose(buffer);

    log_debug("file with path %s was loaded successfully!", rom_path);

    LoadHeaderData();
}

void Cartridge::Reset() {
    memset(&header, 0, sizeof(CartridgeHeader));
    memset(command_buffer, 0, 8);

    ROMCTRL = 0;
    AUXSPICNT = 0;
    AUXSPIDATA = 0;

    transfer_count = 0;

    command = 0;
}

void Cartridge::WriteROMCTRL(u32 data) {
    u32 old_romctrl = ROMCTRL;
    ROMCTRL = data;
    // setup a cartridge transfer if cartridge enable (bit 31) goes from 0 to 1
    if (!(old_romctrl & (1 << 31)) && (ROMCTRL & (1 << 31))) {
        Transfer();
    }
}

void Cartridge::Transfer() {
    u8 block_size = (ROMCTRL >> 24) & 0x7;


    if (block_size == 0) {
        transfer_count = 0;
    } else if (block_size == 7) {
        // transfer count is 4 bytes
        transfer_count = 4;
    } else {
        // 100h SHL (1..6) bytes
        transfer_count = 0x100 << block_size;
    }

    // check the first command in the command buffer
    switch (command_buffer[0]) {
    case DUMMY_COMMAND:
        command = DUMMY_COMMAND;
        break;
    default:
        log_fatal("implemented support for cartridge command %02x", command_buffer[0]);
    }
}

void Cartridge::WriteAUXSPICNT(u16 data) {
    AUXSPICNT = data;
}

void Cartridge::WriteAUXSPIDATA(u16 data) {
    AUXSPIDATA = data;
}

void Cartridge::ReceiveCommand(u8 command, int command_index) {
    command_buffer[command_index] = command;
}

void Cartridge::DirectBoot() {
    // perform data transfers

    // first transfer the cartridge header (this is taken from rom address 0 and loaded into main memory at address 0x27FFE00)
    for (u32 i = 0; i < 0x170; i++) {
        core->memory.ARM9WriteByte(0x027FFE00 + i, rom[i]);
    }

    // next transfer the arm9 code
    for (u32 i = 0; i < header.arm9_size; i++) {
        core->memory.ARM9WriteByte(header.arm9_ram_address + i, rom[header.arm9_rom_offset + i]);
    }

    // finally transfer the arm7 code
    for (u32 i = 0; i < header.arm7_size; i++) {

        core->memory.ARM7WriteByte(header.arm7_ram_address + i, rom[header.arm7_rom_offset + i]);
    }

    log_debug("cartridge data transfers completed successfully!");
}
