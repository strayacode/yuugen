#pragma once

#include <common/types.h>
#include <common/log.h>
#include <core/hw/cartridge/backup/generic_backup.h>
#include <core/hw/cartridge/backup/flash/flash.h>
#include <core/hw/cartridge/backup/eeprom/eeprom.h>
#include <core/hw/cartridge/backup/no_backup/no_backup.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>
#include <iterator>
#include <memory>

// TODO: change this to an enum class
enum CartridgeCommand {
    READ_HEADER = 0x00,
    ENABLE_KEY1 = 0x3C,
    FIRST_CHIP_ID = 0x90,
    DUMMY_COMMAND = 0x9F,
    READ_DATA = 0xB7,
    SECOND_CHIP_ID = 0xB8,
};

enum class CartridgeCommandType {
    Dummy,
    ReadData,
    GetFirstID,
    GetSecondID,
    GetThirdID,
    ReadHeader,
    ReadSecureArea,
    None,
};

enum BackupType {
    EEPROM_SMALL,
    EEPROM,
    FLASH,
    NO_BACKUP,
};

class System;

// TODO: check if the rom is encrypted. if it is then 
// transfer as unencrypted
class Cartridge {
public:
    Cartridge(System& system);
    ~Cartridge();
    void Reset();
    void LoadRom(std::string rom_path);
    void LoadHeaderData();
    void DetectBackupType();

    void DirectBoot();
    void FirmwareBoot();

    void WriteROMCTRL(u32 data);
    void WriteAUXSPICNT(u16 data);
    void WriteAUXSPIDATA(u8 data);

    void ReceiveCommand(u8 command, int command_index);
    auto ReadCommand(int command_index) -> u8;
    auto ReadData() -> u32;
    void StartTransfer();
    void InterpretEncryptedCommand();
    void InterpretDecryptedCommand();

    void WriteSeed0_L(u32 data);
    void WriteSeed1_L(u32 data);

    void WriteSeed0_H(u16 data);
    void WriteSeed1_H(u16 data);

    u64 Decrypt64(u64 data);
    u64 Encrypt64(u64 data);
    void InitKeyCode(u32 level, u32 modulo);
    void ApplyKeyCode(u32 modulo);
    u32 BSwap32(u32 data);

    struct CartridgeHeader {
        char game_title[12];

        u32 arm9_rom_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm9_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm9_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm9_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 arm7_rom_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm7_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm7_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm7_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 icon_title_offset; // specifies the offset in the rom image to where the icon and title is
        
        // used to identify the backup type
        u32 gamecode;
    } header;

    u32 transfer_count;
    u32 transfer_size;
    u32 rom_position;

    u32 ROMCTRL;
    u16 AUXSPICNT;
    u16 AUXSPIDATA;

    u8 command_buffer[8];

    u64 command;

    System& system;

    std::vector<u8> rom;

    u64 rom_size;

    u64 seed0;
    u64 seed1;

    u32 backup_write_count;

    std::unique_ptr<GenericBackup> backup;

    u32 backup_type;
    u32 backup_size;

    bool key1_encryption;
    CartridgeCommandType command_type;

    u32 key1_buffer[0x412];
    u32 key1_code[3];

    u8 secure_area[0x4000];
};