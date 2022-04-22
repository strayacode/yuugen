#pragma once

#include <memory>
#include <string.h>
#include "Common/Types.h"
#include "Core/hw/cartridge/loader_base.h"
#include "Core/hw/cartridge/backup/generic_backup.h"
#include "Core/hw/cartridge/backup/flash/flash.h"
#include "Core/hw/cartridge/backup/eeprom/eeprom.h"
#include "Core/hw/cartridge/backup/no_backup/no_backup.h"

enum BackupType {
    EEPROM_SMALL,
    EEPROM,
    FLASH,
    NO_BACKUP,
};

class NDSLoader : public LoaderBase {
public:
    ~NDSLoader();

    void LoadHeader() override;
    void LoadBackup() override;
    void DetectBackupType();

    u32 GetARM7Entrypoint();
    u32 GetARM9Entrypoint();
    u32 GetARM7Size();
    u32 GetARM9Size();
    u32 GetARM7RAMAddress();
    u32 GetARM9RAMAddress();
    u32 GetARM7Offset();
    u32 GetARM9Offset();
    u32 GetGamecode();

    struct CartridgeHeader {
        char game_title[12];

        u32 arm9_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm9_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm9_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm9_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 arm7_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm7_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm7_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm7_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 icon_title_offset; // specifies the offset in the rom image to where the icon and title is
        
        // used to identify the backup type
        u32 gamecode;
    } header;

    std::unique_ptr<GenericBackup> backup;
    u32 backup_write_count = 0;
};