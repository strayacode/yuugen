#include "Core/hw/cartridge/cartridge.h"
#include "Core/core.h"

Cartridge::Cartridge(System& system) : system(system) {}

void Cartridge::Reset() {
    ROMCTRL = 0;
    AUXSPICNT = 0;
    AUXSPIDATA = 0;
    transfer_count = 0;
    transfer_size = 0;
    rom_position = 0;
    command = 0;
    seed0 = seed1 = 0;
    key1_encryption = false;
    command_type = CartridgeCommandType::Dummy;
    loader.Reset();

    for (int i = 0; i < 3; i++) {
        key1_code[i] = 0;
    }
}

void Cartridge::LoadRom(std::string rom_path) {
    loader.Load(rom_path);
}

void Cartridge::WriteROMCTRL(u32 data) {
    u32 old_romctrl = ROMCTRL;
    ROMCTRL = data;
    // setup a cartridge transfer if cartridge enable (bit 31) goes from 0 to 1
    if (!(old_romctrl & (1 << 31)) && (ROMCTRL & (1 << 31))) {
        StartTransfer();
    }
}

void Cartridge::StartTransfer() {
    // setup the transfer size as indicated by the recently set ROMCTRL
    u8 block_size = (ROMCTRL >> 24) & 0x7;

    if (block_size == 0) {
        transfer_size = 0;
    } else if (block_size == 7) {
        transfer_size = 4;
    } else {
        transfer_size = 0x100 << block_size;
    }

    command = 0;

    for (int i = 0; i < 8; i++) {
        command |= ((u64)command_buffer[i] << ((7 - i) * 8));
    }

    if (key1_encryption) {
        InterpretEncryptedCommand();
    } else {
        InterpretDecryptedCommand();
    }

    if (transfer_size == 0) {
        // we won't need to read any data from the rom
        // indicate that the block is ready (empty block) and that no word is ready for reading yet
        ROMCTRL &= ~(1 << 23);
        ROMCTRL &= ~(1 << 31);

        // send a transfer ready interrupt if enabled in AUXSPICNT
        // TODO: are we meant to send interrupt to both cpus?
        if (AUXSPICNT & (1 << 14)) {
            system.cpu_core[0].SendInterrupt(InterruptType::CartridgeTransfer);
            system.cpu_core[1].SendInterrupt(InterruptType::CartridgeTransfer);
        }
    } else {
        // since we are starting a new transfer transfer_count must start at 0
        // (no words have been read yet)
        transfer_count = 0;

        // indicate that a word is ready
        ROMCTRL |= (1 << 23);

        // start a dma transfer to read from rom
        if (system.CartridgeAccessRights()) {
            system.dma[1].Trigger(5);
        } else {
            system.dma[0].Trigger(2);
        }
    }
    
}

void Cartridge::InterpretEncryptedCommand() {
    InitKeyCode(2, 2);
    command = Decrypt64(command);
    
    if (loader.GetSize()) {
        switch (command >> 60) {
        case 0x1:
            command_type = CartridgeCommandType::GetSecondID;
            break;
        case 0x2:
            rom_position = ((command >> 44) & 0xFFFF) * 0x1000;
            command_type = CartridgeCommandType::ReadSecureArea;
            break;
        case 0x4:
            command_type = CartridgeCommandType::None;
            break;
        case 0xA:
            // enter main data mode (disable key1 encryption)
            key1_encryption = false;
            command_type = CartridgeCommandType::None;
            break;
        default:
            log_fatal("handle encrypted %016lx", command);
        }
    }
}

void Cartridge::InterpretDecryptedCommand() {
    if (loader.GetSize()) {
        if ((command & 0xFF00000000FFFFFF) == 0xB700000000000000) {
            rom_position = (command >> 24) & 0xFFFFFFFF;
            command_type = CartridgeCommandType::ReadData;
        } else if (command == 0xB800000000000000) {
            command_type = CartridgeCommandType::GetThirdID;  
        } else if (command == 0x9F00000000000000) {
            command_type = CartridgeCommandType::Dummy;
        } else if (command == 0x0000000000000000) {
            command_type = CartridgeCommandType::ReadHeader;  
        } else if (command == 0x9000000000000000) {
            command_type = CartridgeCommandType::GetFirstID;  
        } else if ((command >> 56) == 0x3C) {
            key1_encryption = true;
            command_type = CartridgeCommandType::None;
        } else {
            log_fatal("handle unencrypted %016lx", command);
        }
    }
}

u32 Cartridge::ReadData() {
    u32 data = 0xFFFFFFFF;

    if (!(ROMCTRL & (1 << 23))) {
        // if we are still busy with getting a word then just return 0xFFFFFFFF
        return data;
    }

    // only do commands if a rom exists
    if (loader.GetSize()) {
        // check the first command in the command buffer
        switch (command_type) {
        case CartridgeCommandType::Dummy:
            break;
        case CartridgeCommandType::ReadData:
            if (rom_position < 0x8000) {
                rom_position = 0x8000 + (rom_position & 0x1FF);
            }

            if ((rom_position + transfer_count) >= loader.GetSize()) {
                log_fatal("[Cartridge] Read data command exceeds rom size");
            }

            // otherwise read
            memcpy(&data, loader.GetPointer(rom_position + transfer_count), 4);
            break;
        case CartridgeCommandType::GetFirstID:
        case CartridgeCommandType::GetSecondID:
        case CartridgeCommandType::GetThirdID:
            data = 0x1FC2;
            break;
        case CartridgeCommandType::ReadHeader:
            // return the cartridge header repeated every 0x1000 bytes
            memcpy(&data, loader.GetPointer(transfer_count & 0xFFF), 4);
            break;
        case CartridgeCommandType::ReadSecureArea:
            memcpy(&data, loader.GetPointer(rom_position + transfer_count), 4);
            break;
        default:
            log_fatal("handle cartridge command type %d", static_cast<int>(command_type));
        }
    }

    // after reading a word from the cartridge we must increment transfer_count by 4 as we just read 4 bytes
    transfer_count += 4;

    if (transfer_count == transfer_size) {
        // finished with the cartridge transfer so indicate that a block is ready and that a word is not ready
        ROMCTRL &= ~(1 << 23);
        ROMCTRL &= ~(1 << 31);

        // send a transfer ready interrupt if enabled in AUXSPICNT
        if (AUXSPICNT & (1 << 14)) {
            system.cpu_core[0].SendInterrupt(InterruptType::CartridgeTransfer);
            system.cpu_core[1].SendInterrupt(InterruptType::CartridgeTransfer);
        }
    } else {
        // trigger another nds cartridge dma
        if (system.CartridgeAccessRights()) {
            system.dma[1].Trigger(5);
        } else {
            system.dma[0].Trigger(2);
        }
    }

    return data;
}

void Cartridge::WriteAUXSPICNT(u16 data) {
    AUXSPICNT = data;
}

void Cartridge::WriteAUXSPIDATA(u8 data) {
    // don't set if no backup exists
    if (loader.GetBackupSize() == 0) {
        return;
    }

    if (loader.backup_write_count == 0) {
        // interpret a new command
        loader.backup->ReceiveCommand(data);

        AUXSPIDATA = 0;
    } else {
        // TODO: make this cleaner later
        AUXSPIDATA = loader.backup->Transfer(data, loader.backup_write_count);
    }
    
    if (AUXSPICNT & (1 << 6)) {
        // keep selected
        loader.backup_write_count++;
    } else {
        // deselect
        loader.backup_write_count = 0;
    }
}

void Cartridge::ReceiveCommand(u8 command, int command_index) {
    command_buffer[command_index] = command;
}

u8 Cartridge::ReadCommand(int command_index) {
    return command_buffer[command_index];
}

void Cartridge::DirectBoot() {
    // first transfer the cartridge header (this is taken from rom address 0 and loaded into main memory at address 0x27FFE00)
    for (u32 i = 0; i < 0x170; i++) {
        system.arm9_memory.FastWrite<u8>(0x027FFE00 + i, *loader.GetPointer(i));
    }

    // next transfer the arm9 code
    for (u32 i = 0; i < loader.GetARM9Size(); i++) {
        system.arm9_memory.FastWrite<u8>(loader.GetARM9RAMAddress() + i, *loader.GetPointer(loader.GetARM9Offset() + i));
    }

    // finally transfer the arm7 code
    for (u32 i = 0; i < loader.GetARM7Size(); i++) {
        system.arm7_memory.FastWrite<u8>(loader.GetARM7RAMAddress() + i, *loader.GetPointer(loader.GetARM7Offset() + i));
    }

    log_debug("[Cartridge] Data transferred into memory");
}

void Cartridge::FirmwareBoot() {
    if (loader.GetSize() >= 0x8000) {
        u64 encry_obj = 0x6A624F7972636E65;

        memcpy(loader.GetPointer(0x4000), &encry_obj, 8);

        InitKeyCode(3, 2);

        // encrypt the first 2kb of the secure area
        for (int i = 0; i < 0x800; i += 8) {
            u64 data = 0;
            memcpy(&data, loader.GetPointer(0x4000 + i), 8);

            u64 encrypted_data = Encrypt64(data);
            memcpy(loader.GetPointer(0x4000 + i), &encrypted_data, 8);
        }

        // double encrypt the first 8 bytes
        u64 data = 0;
        memcpy(&data, loader.GetPointer(0x4000), 8);

        InitKeyCode(2, 2);
        u64 encrypted_data = Encrypt64(data);
        memcpy(loader.GetPointer(0x4000), &encrypted_data, 8);

        log_debug("[Cartridge] First 2kb of secure area encrypted successfully");
    }
}

// TODO: handle key2 encryption later
void Cartridge::WriteSeed0_L(u32 data) {

}
    
void Cartridge::WriteSeed1_L(u32 data) {

}

void Cartridge::WriteSeed0_H(u16 data) {

}

void Cartridge::WriteSeed1_H(u16 data) {

}

u64 Cartridge::Decrypt64(u64 data) {
    u32 y = data & 0xFFFFFFFF;
    u32 x = data >> 32;

    for (int i = 0x11; i >= 0x02; i--) {
        u32 z = key1_buffer[i] ^ x;
        x = key1_buffer[0x12 + ((z >> 24) & 0xFF)];
        x = key1_buffer[0x112 + ((z >> 16) & 0xFF)] + x;
        x = key1_buffer[0x212 + ((z >> 8) & 0xFF)] ^ x;
        x = key1_buffer[0x312 + (z & 0xFF)] + x;
        x = y ^ x;
        y = z;
    }

    u64 result = ((u64)(y ^ key1_buffer[0]) << 32) | (x ^ key1_buffer[1]);
    return result;
}

u64 Cartridge::Encrypt64(u64 data) {
    u32 y = data & 0xFFFFFFFF;
    u32 x = data >> 32;

    for (int i = 0; i <= 0x0F; i++) {
        u32 z = key1_buffer[i] ^ x;
        x = key1_buffer[0x12 + ((z >> 24) & 0xFF)];
        x = key1_buffer[0x112 + ((z >> 16) & 0xFF)] + x;
        x = key1_buffer[0x212 + ((z >> 8) & 0xFF)] ^ x;
        x = key1_buffer[0x312 + (z & 0xFF)] + x;
        x = y ^ x;
        y = z;
    }

    u64 result = ((u64)(y ^ key1_buffer[0x11]) << 32) | (x ^ key1_buffer[0x10]);
    return result;
}

void Cartridge::InitKeyCode(u32 level, u32 modulo) {
    // copy the key1 buffer from the arm7 bios
    for (int i = 0; i < 0x412; i++) {
        key1_buffer[i] = system.arm7_memory.FastRead<u32>(0x30 + (i * 4));
    }

    key1_code[0] = loader.GetGamecode();
    key1_code[1] = loader.GetGamecode() / 2;
    key1_code[2] = loader.GetGamecode() * 2;

    if (level >= 1) {
        ApplyKeyCode(modulo);
    }

    if (level >= 2) {
        ApplyKeyCode(modulo);
    }

    if (level >= 3) {
        key1_code[1] *= 2;
        key1_code[2] /= 2;

        ApplyKeyCode(modulo);
    }
}

void Cartridge::ApplyKeyCode(u32 modulo) {
    u64 encrypt_result = Encrypt64(((u64)key1_code[2] << 32) | key1_code[1]);
    key1_code[1] = encrypt_result & 0xFFFFFFFF;
    key1_code[2] = encrypt_result >> 32;

    u64 encrypt_result1 = Encrypt64(((u64)key1_code[1] << 32) | key1_code[0]);
    key1_code[0] = encrypt_result1 & 0xFFFFFFFF;
    key1_code[1] = encrypt_result1 >> 32;

    for (int i = 0; i <= 0x11; i++) {
        key1_buffer[i] ^= BSwap32(key1_code[i % modulo]);
    }

    u64 scratch = 0;

    for (int i = 0; i <= 0x410; i += 2) {
        scratch = Encrypt64(scratch);
        key1_buffer[i] = scratch >> 32;
        key1_buffer[i + 1] = scratch & 0xFFFFFFFF;
    }
}

u32 Cartridge::BSwap32(u32 data) {
    u32 result = 0;
    result |= data >> 24;
    result |= (data >> 8) & 0xFF00;
    result |= (data << 8) & 0xFF0000;
    result |= data << 24;

    return result;
}