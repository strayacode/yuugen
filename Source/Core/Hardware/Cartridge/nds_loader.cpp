#include "Core/Hardware/Cartridge/nds_loader.h"
#include "Core/Hardware/Cartridge/save_database.h"

NDSLoader::~NDSLoader() {
    if (backup) {
        backup->SaveBackup();
    }
}

void NDSLoader::LoadHeader() {
    memcpy(&header.game_title, GetPointer(0), 12);

    // load the u32 variables in the header struct from the respective areas of the rom
    memcpy(&header.gamecode, GetPointer(0x0C), 4);
    memcpy(&header.arm9_offset, GetPointer(0x20), 4);
    memcpy(&header.arm9_entrypoint, GetPointer(0x24), 4);
    memcpy(&header.arm9_ram_address, GetPointer(0x28), 4);
    memcpy(&header.arm9_size, GetPointer(0x2C), 4);
    memcpy(&header.arm7_offset, GetPointer(0x30), 4);
    memcpy(&header.arm7_entrypoint, GetPointer(0x34), 4);
    memcpy(&header.arm7_ram_address, GetPointer(0x38), 4);
    memcpy(&header.arm7_size, GetPointer(0x3C), 4);
    memcpy(&header.icon_title_offset, GetPointer(0x68), 4);
    log_debug("[ARM9]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm9_offset, header.arm9_entrypoint, header.arm9_ram_address, header.arm9_size);
    log_debug("[ARM7]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm7_offset, header.arm7_entrypoint, header.arm7_ram_address, header.arm7_size);

    log_debug("[Cartridge] Header data loaded");
}

void NDSLoader::LoadBackup() {
    DetectBackupType();

    // now we want to do backup stuff
    std::string save_path = rom_path.replace(rom_path.begin(), rom_path.begin() + 7, "../saves");

    save_path.replace(save_path.find(".nds"), 4, ".sav");

    switch (backup_type) {
    case FLASH:
        backup = std::make_unique<FlashBackup>(save_path, backup_size);
        break;
    case EEPROM:
        backup = std::make_unique<EEPROMBackup>(save_path, backup_size);
        break;
    case NO_BACKUP:
        backup = std::make_unique<NoBackup>(save_path, 0);
        break;
    default:
        log_fatal("backup type %d not handled", backup_type);
    }
}

void NDSLoader::DetectBackupType() {
    // loop through each entry in the save database
    for (int i = 0; i < 6776; i++) {
        if (header.gamecode == save_database[i].gamecode) {
            // get the save type
            switch (save_database[i].save_type) {
            case 0:
                backup_type = NO_BACKUP;
                break;
            case 1:
                log_fatal("handle eeprom smol");
                break;
            case 2: case 3: case 4:
                backup_type = EEPROM;
                break;
            case 5: case 6: case 7:
                backup_type = FLASH;
                break;
            default:
                log_fatal("handle savetype %02x", save_database[i].save_type);
            }

            // get the save size
            backup_size = save_sizes[save_database[i].save_type];
            return;
        }
    }

    // if the game entry is not found in the save database,
    // then default to flash 512K
    backup_type = FLASH;
    backup_size = SIZE_512K;
}

u32 NDSLoader::GetARM7Entrypoint() {
    return header.arm7_entrypoint;
}

u32 NDSLoader::GetARM9Entrypoint() {
    return header.arm9_entrypoint;
}

u32 NDSLoader::GetARM7Size() {
    return header.arm7_size;
}

u32 NDSLoader::GetARM9Size() {
    return header.arm9_size;
}

u32 NDSLoader::GetARM7RAMAddress() {
    return header.arm7_ram_address;
}

u32 NDSLoader::GetARM9RAMAddress() {
    return header.arm9_ram_address;
}

u32 NDSLoader::GetARM7Offset() {
    return header.arm7_offset;
}

u32 NDSLoader::GetARM9Offset() {
    return header.arm9_offset;
}

u32 NDSLoader::GetGamecode() {
    return header.gamecode;
}