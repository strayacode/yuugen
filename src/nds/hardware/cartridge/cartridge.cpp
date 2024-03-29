#include "common/memory.h"
#include "common/logger.h"
#include "common/bits.h"
#include "nds/hardware/cartridge/cartridge.h"
#include "nds/hardware/cartridge/save_database.h"
#include "nds/hardware/cartridge/backup/no_backup.h"
#include "nds/hardware/cartridge/backup/flash_backup.h"
#include "nds/hardware/cartridge/backup/eeprom_backup.h"
#include "nds/system.h"

namespace nds {

Cartridge::Cartridge(System& system) : system(system) {}

void Cartridge::reset() {
    auxspicnt.data = 0;
    auxspidata = 0;
    romctrl.data = 0;
    command_buffer = 0;
    command = 0;
    transfer_count = 0;
    transfer_size = 0;
    rom_position = 0;
    seed0 = 0;
    seed1 = 0;
    key1_encryption = false;
    command_type = CommandType::Dummy;
    key1_buffer.fill(0);
    key1_code.fill(0);
    secure_area.fill(0);
    cartridge_inserted = false;
    backup_write_count = 0;
}

void Cartridge::load(const std::string& path) {
    memory_mapped_file.load(path);
    load_header();
    load_backup(path);
    cartridge_inserted = true;
}

void Cartridge::direct_boot() {
    using Bus = arm::Bus;

    // transfer the cartridge header
    for (u32 i = 0; i < 0x170; i++) {
        system.arm9.get_memory().write<u8, Bus::System>(0x027ffe00 + i, *memory_mapped_file.get_pointer(i));
    }

    // transfer the arm9 code
    for (u32 i = 0; i < header.arm9_size; i++) {
        system.arm9.get_memory().write<u8, Bus::System>(header.arm9_ram_address + i, *memory_mapped_file.get_pointer(header.arm9_offset + i));
    }

    // transfer the arm7 code
    for (u32 i = 0; i < header.arm7_size; i++) {
        system.arm7.get_memory().write<u8, Bus::System>(header.arm7_ram_address + i, *memory_mapped_file.get_pointer(header.arm7_offset + i));
    }

    LOG_DEBUG("cartridge data transferred into memory");
}

void Cartridge::firmware_boot() {
    if (memory_mapped_file.get_size() >= 0x8000) {
        LOG_TODO("handle encryption in firmware boot");
        // u64 encry_obj = 0x6A624F7972636E65;

        // memcpy(loader.GetPointer(0x4000), &encry_obj, 8);

        // InitKeyCode(3, 2);

        // // encrypt the first 2kb of the secure area
        // for (int i = 0; i < 0x800; i += 8) {
        //     u64 data = 0;
        //     memcpy(&data, loader.GetPointer(0x4000 + i), 8);

        //     u64 encrypted_data = Encrypt64(data);
        //     memcpy(loader.GetPointer(0x4000 + i), &encrypted_data, 8);
        // }

        // // double encrypt the first 8 bytes
        // u64 data = 0;
        // memcpy(&data, loader.GetPointer(0x4000), 8);

        // InitKeyCode(2, 2);
        // u64 encrypted_data = Encrypt64(data);
        // memcpy(loader.GetPointer(0x4000), &encrypted_data, 8);

        // log_debug("[Cartridge] First 2kb of secure area encrypted successfully");
    }
}

void Cartridge::write_auxspicnt(u16 value, u32 mask) {
    auxspicnt.data = (auxspicnt.data & ~mask) | (value & mask);
}

void Cartridge::write_auxspidata(u8 value) {
    if (backup == nullptr) {
        return;
    }

    if (backup_write_count == 0) {
        if (value == 0) {
            return;
        }

        backup->receive(value);
        auxspidata = 0;
    } else {
        auxspidata = backup->transfer(value, backup_write_count);
    }

    if (auxspicnt.chipselect_hold) {
        backup_write_count++;
    } else {
        backup_write_count = 0;
    }
}

void Cartridge::write_romctrl(u32 value, u32 mask) {
    auto old_romctrl = romctrl;
    romctrl.data = (romctrl.data & ~mask) | (value & mask);

    if (!old_romctrl.block_start && romctrl.block_start) {
        start_transfer();
    }
}

void Cartridge::write_command_buffer(u64 value, u64 mask) {
    command_buffer = (command_buffer & ~mask) | (value & mask);
}

u32 Cartridge::read_data() {
    u32 data = 0xffffffff;
    if (!romctrl.word_ready) {
        return data;
    }

    if (cartridge_inserted) {
        switch (command_type) {
        case CommandType::Dummy:
            break;
        case CommandType::ReadData:
            if (rom_position < 0x8000) {
                rom_position = 0x8000 + (rom_position & 0x1ff);
            }

            if ((rom_position + transfer_count) >= memory_mapped_file.get_size()) {
                // TODO: handle this case
                // LOG_WARN("read data command exceeds rom size");
                return data;
            }

            data = common::read<u32>(memory_mapped_file.get_pointer(rom_position + transfer_count));
            break;
        case CommandType::GetFirstId:
        case CommandType::GetSecondId:
        case CommandType::GetThirdId:
            data = 0x1fc2;
            break;
        case CommandType::ReadHeader:
            data = common::read<u32>(memory_mapped_file.get_pointer(transfer_count & 0xfff));
            break;
        case CommandType::ReadSecureArea:
            data = common::read<u32>(memory_mapped_file.get_pointer(rom_position + transfer_count));
            break;
        default:
            LOG_ERROR("handle unknown cartridge command");
        }
    }

    transfer_count += 4;
    if (transfer_count == transfer_size) {
        romctrl.word_ready = false;
        romctrl.block_start = false;

        // TODO: are we meant to send an interrupt to both cpus?
        if (auxspicnt.transfer_ready_irq) {
            system.arm7.get_irq().raise(IRQ::Source::CartridgeTransfer);
            system.arm9.get_irq().raise(IRQ::Source::CartridgeTransfer);
        }
    } else {
        if (common::get_bit<11>(system.exmemcnt)) {
            system.dma7.trigger(DMA::Timing::Slot1);
        } else {
            system.dma9.trigger(DMA::Timing::Slot1);
        }
    }

    return data;
}

void Cartridge::load_header() {
    memcpy(&header.game_title, memory_mapped_file.get_pointer(0x00), 12);

    // load the u32 variables in the header struct from the respective areas of the rom
    memcpy(&header.gamecode, memory_mapped_file.get_pointer(0x0c), 4);
    memcpy(&header.arm9_offset, memory_mapped_file.get_pointer(0x20), 4);
    memcpy(&header.arm9_entrypoint, memory_mapped_file.get_pointer(0x24), 4);
    memcpy(&header.arm9_ram_address, memory_mapped_file.get_pointer(0x28), 4);
    memcpy(&header.arm9_size, memory_mapped_file.get_pointer(0x2c), 4);
    memcpy(&header.arm7_offset, memory_mapped_file.get_pointer(0x30), 4);
    memcpy(&header.arm7_entrypoint, memory_mapped_file.get_pointer(0x34), 4);
    memcpy(&header.arm7_ram_address, memory_mapped_file.get_pointer(0x38), 4);
    memcpy(&header.arm7_size, memory_mapped_file.get_pointer(0x3c), 4);
    memcpy(&header.icon_title_offset, memory_mapped_file.get_pointer(0x68), 4);
    LOG_DEBUG("arm9 offset %08x", header.arm9_offset);
    LOG_DEBUG("arm9 entrypoint %08x", header.arm9_entrypoint);
    LOG_DEBUG("arm9 ram address %08x", header.arm9_ram_address);
    LOG_DEBUG("arm9 size %08x", header.arm9_size);
    LOG_DEBUG("arm7 offset %08x", header.arm7_offset);
    LOG_DEBUG("arm7 entrypoint %08x", header.arm7_entrypoint);
    LOG_DEBUG("arm7 ram address %08x", header.arm7_ram_address);
    LOG_DEBUG("arm7 size %08x", header.arm7_size);
    LOG_DEBUG("header data loaded");
}

void Cartridge::load_backup(std::string path) {
    std::string save_path = path.replace(path.begin(), path.begin() + 7, "../saves");
    save_path.replace(save_path.find(".nds"), 4, ".sav");

    for (int i = 0; i < 6776; i++) {
        if (header.gamecode == save_database[i].gamecode) {
            auto save_size = save_sizes[save_database[i].save_type];
            switch (save_database[i].save_type) {
            case 0:
                backup = std::make_unique<NoBackup>();
                return;
            case 1:
                LOG_ERROR("handle small eeprom");
                return;
            case 2: case 3: case 4:
                backup = std::make_unique<EEPROMBackup>(save_path, save_size);
                return;
            case 5: case 6: case 7:
                backup = std::make_unique<FlashBackup>(save_path, save_size);
                return;
            default:
                LOG_ERROR("handle save type %d", save_database[i].save_type);
            }
        }
    }

    // if the game entry is not found in the save database,
    // then default to flash with size 512kb
    backup = std::make_unique<FlashBackup>(save_path, 0x80000);
}

void Cartridge::start_transfer() {
    if (romctrl.block_size == 0) {
        transfer_size = 0;
    } else if (romctrl.block_size == 7) {
        transfer_size = 4;
    } else {
        transfer_size = 0x100 << romctrl.block_size;
    }

    command = common::bswap64(command_buffer);
    if (key1_encryption) {
        LOG_ERROR("handle key1 encryption");
    } else {
        process_decrypted_command();
    }

    if (transfer_size == 0) {
        romctrl.word_ready = false;
        romctrl.block_start = false;

        // TODO: are we meant to send an interrupt to both cpus?
        if (auxspicnt.transfer_ready_irq) {
            system.arm7.get_irq().raise(IRQ::Source::CartridgeTransfer);
            system.arm9.get_irq().raise(IRQ::Source::CartridgeTransfer);
        }
    } else {
        transfer_count = 0;
        romctrl.word_ready = true;

        if (common::get_bit<11>(system.exmemcnt)) {
            system.dma7.trigger(DMA::Timing::Slot1);
        } else {
            system.dma9.trigger(DMA::Timing::Slot1);
        }
    }
}

void Cartridge::process_decrypted_command() {
    if (cartridge_inserted) {
        if ((command & 0xff00000000ffffff) == 0xb700000000000000) {
            rom_position = common::get_field<24, 32>(command);
            command_type = CommandType::ReadData;
        } else if (command == 0xb800000000000000) {
            command_type = CommandType::GetThirdId;  
        } else if (command == 0x9f00000000000000) {
            command_type = CommandType::Dummy;
        } else if (command == 0x0000000000000000) {
            command_type = CommandType::ReadHeader;  
        } else if (command == 0x9000000000000000) {
            command_type = CommandType::GetFirstId;  
        } else if ((command >> 56) == 0x3c) {
            key1_encryption = true;
            command_type = CommandType::None;
        } else {
            LOG_ERROR("handle decrypted command: %016lx", command);
        }
    }
}

} // namespace nds