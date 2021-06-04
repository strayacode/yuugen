#include <core/hw/cartridge/cartridge.h>
#include <core/core.h>
#include <core/hw/cartridge/save_database.h>

Cartridge::Cartridge(Core* core) : core(core) {

}

Cartridge::~Cartridge() {
    // only save if there is a backup already
    if (backup) {
        backup->SaveBackup();
    }
}

void Cartridge::Reset() {
    rom.clear();

    ROMCTRL = 0;
    AUXSPICNT = 0;
    AUXSPIDATA = 0;

    transfer_count = 0;
    transfer_size = 0;

    command = 0;

    rom_size = 0;

    seed0 = seed1 = 0;

    backup_write_count = 0;

    backup_type = 0;

    backup_size = 0;
}

void Cartridge::LoadRom(std::string rom_path) {
    std::ifstream file(rom_path, std::ios::binary);

    if (!file) {
        log_fatal("rom with path %s does not exist!", rom_path.c_str());
    }

    file.unsetf(std::ios::skipws);

    std::streampos size;

    file.seekg(0, std::ios::end);
    size = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve bytes for our rom vector specified by rom_size
    rom.reserve(size);

    rom.insert(rom.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());

    rom_size = rom.size();

    file.close();

    log_debug("[Cartridge] Rom data loaded");
    log_debug("[Cartridge] Size: %08lx", rom_size);

    LoadHeaderData();

    DetectBackupType();

    // now we want to do backup stuff
    std::string save_path = rom_path.replace(rom_path.find("nds"), 3, "sav");

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

void Cartridge::LoadHeaderData() {
    // load the u32 variables in the header struct from the respective areas of the rom
    memcpy(&header.gamecode, &rom[0x0C], 4);
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
    log_debug("[ARM9]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm9_rom_offset, header.arm9_entrypoint, header.arm9_ram_address, header.arm9_size);
    log_debug("[ARM7]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm7_rom_offset, header.arm7_entrypoint, header.arm7_ram_address, header.arm7_size);

    log_debug("[Cartridge] Header data loaded");
}

void Cartridge::DetectBackupType() {
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

    if (transfer_size == 0) {
        // we won't need to read any data from the rom
        // indicate that the block is ready (empty block) and that no word is ready for reading yet
        ROMCTRL &= ~(1 << 23);
        ROMCTRL &= ~(1 << 31);

        // send a transfer ready interrupt if enabled in AUXSPICNT
        if (AUXSPICNT & (1 << 14)) {
            core->arm7.SendInterrupt(19);
            core->arm9.SendInterrupt(19);
        }
    } else {
        // since we are starting a new transfer transfer_count must start at 0
        // (no words have been read yet)
        transfer_count = 0;

        // indicate that a word is ready
        ROMCTRL |= (1 << 23);

        // start a dma transfer to read from rom
        if (core->memory.CartridgeAccessRights()) {
            core->dma[1].Trigger(5);
        } else {
            core->dma[0].Trigger(2);
        }
    }
    
}

auto Cartridge::ReadData() -> u32 {
    u32 data = 0xFFFFFFFF;

    if (!(ROMCTRL & (1 << 23))) {
        // if we are still busy with getting a word then just return 0xFFFFFFFF
        return data;
    }

    // check the first command in the command buffer
    switch (command_buffer[0]) {
    case READ_HEADER:
        // return the cartridge header repeated every 0x1000 bytes
        memcpy(&data, &rom[transfer_count & 0xFFF], 4);
        break;
    case DUMMY_COMMAND:
        // data remains as 0xFFFFFFFF
        break;
    case READ_DATA: {
        // get the address from the 4 parameter bytes after the command byte in command buffer
        u32 address = (command_buffer[1] << 24) | (command_buffer[2] << 16) | (command_buffer[3] << 8) | (command_buffer[4]);
        if (address < 0x8000) {
            address = 0x8000 + (address & 0x1FF);
        }

        if (address + transfer_count >= rom_size) {
            log_fatal("[Cartridge] Read data command exceeds rom size");
        }

        // otherwise read
        memcpy(&data, &rom[address + transfer_count], 4);
        break;
    }
    case FIRST_CHIP_ID: case SECOND_CHIP_ID:
        data = 0x1FC2;
        break;
    default:
        log_fatal("[Cartridge] Handle cartridge command %02x", command_buffer[0]);
    }

    // after reading a word from the cartridge we must increment transfer_count by 4 as we just read 4 bytes
    transfer_count += 4;

    if (transfer_count == transfer_size) {
        // finished with the cartridge transfer so indicate that a block is ready and that a word is not ready
        ROMCTRL &= ~(1 << 23);
        ROMCTRL &= ~(1 << 31);

        // send a transfer ready interrupt if enabled in AUXSPICNT
        if (AUXSPICNT & (1 << 14)) {
            core->arm7.SendInterrupt(19);
            core->arm9.SendInterrupt(19);
        }
    } else {
        // trigger another nds cartridge dma
        if (core->memory.CartridgeAccessRights()) {
            core->dma[1].Trigger(5);
        } else {
            core->dma[0].Trigger(2);
        }
    }

    return data;
}


void Cartridge::WriteAUXSPICNT(u16 data) {
    AUXSPICNT = data;
}

void Cartridge::WriteAUXSPIDATA(u8 data) {
    if (backup_write_count == 0) {
        // interpret a new command
        backup->ReceiveCommand(data);

        AUXSPIDATA = 0;
    } else {
        // TODO: make this cleaner later
        AUXSPIDATA = backup->Transfer(data, backup_write_count);
    }
    
    if (AUXSPICNT & (1 << 6)) {
        // keep selected
        backup_write_count++;
    } else {
        // deselect
        backup_write_count = 0;
    }
}

void Cartridge::ReceiveCommand(u8 command, int command_index) {
    command_buffer[command_index] = command;
}

auto Cartridge::ReadCommand(int command_index) -> u8 {
    return command_buffer[command_index];
}

void Cartridge::DirectBoot() {
    // first transfer the cartridge header (this is taken from rom address 0 and loaded into main memory at address 0x27FFE00)
    for (u32 i = 0; i < 0x170; i++) {
        core->memory.ARM9Write<u8>(0x027FFE00 + i, rom[i]);
    }

    // next transfer the arm9 code
    for (u32 i = 0; i < header.arm9_size; i++) {
        core->memory.ARM9Write<u8>(header.arm9_ram_address + i, rom[header.arm9_rom_offset + i]);
    }

    // finally transfer the arm7 code
    for (u32 i = 0; i < header.arm7_size; i++) {
        core->memory.ARM7Write<u8>(header.arm7_ram_address + i, rom[header.arm7_rom_offset + i]);
    }

    log_debug("[Cartridge] Data transferred into memory");
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