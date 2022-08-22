#pragma once

#include <string>
#include <string.h>
#include "Common/Types.h"
#include "Common/Log.h"
#include "Core/Hardware/Cartridge/nds_loader.h"
#include "Core/ARM/MMIO.h"

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

class System;

// TODO: check if the rom is encrypted. if it is then 
// transfer as unencrypted
class Cartridge {
public:
    Cartridge(System& system);

    void reset();
    void build_mmio(MMIO& mmio, Arch arch);

    void LoadRom(std::string rom_path);

    void DirectBoot();
    void FirmwareBoot();

    void write_romctrl(u32 data);
    void write_auxspidata(u8 data);

    u32 read_data();
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

    // TODO: put this into a common file
    u32 BSwap32(u32 data);

    u32 transfer_count;
    u32 transfer_size;
    u32 rom_position;
    u32 romctrl;
    u16 auxspicnt;
    u16 auxspidata;
    u8 command_buffer[8];
    u64 command;
    System& system;
    NDSLoader loader;
    u64 seed0;
    u64 seed1;
    bool key1_encryption;
    CartridgeCommandType command_type;
    u32 key1_buffer[0x412];
    u32 key1_code[3];
    u8 secure_area[0x4000];
};