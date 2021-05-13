#include <core/hw/cartridge/cartridge.h>
#include <core/core.h>

Cartridge::Cartridge(Core* core) : core(core) {

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

    log_debug("[Cartridge] Rom data loaded");
    log_debug("[Cartridge] Size: %08lx", rom_size);

    LoadHeaderData();
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
    log_debug("[ARM9]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm9_rom_offset, header.arm9_entrypoint, header.arm9_ram_address, header.arm9_size);
    log_debug("[ARM7]\nOffset: 0x%08x\nEntrypoint: 0x%08x\nRAM Address: 0x%08x\nSize: 0x%08x", header.arm7_rom_offset, header.arm7_entrypoint, header.arm7_ram_address, header.arm7_size);

    log_debug("[Cartridge] Header data loaded");
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
    case CHIP_ID:
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

void Cartridge::WriteAUXSPIDATA(u16 data) {
    // might be used for cartridge backup try to emulate later
    AUXSPIDATA = data;
}

void Cartridge::ReceiveCommand(u8 command, int command_index) {
    command_buffer[command_index] = command;
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