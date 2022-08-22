#include "Core/Hardware/Cartridge/backup/eeprom/eeprom.h"

EEPROMBackup::EEPROMBackup(std::string path, u32 size) : path(path), size(size) {
    Reset();
}
    
auto EEPROMBackup::Transfer(u8 data, u32 write_count) -> u8 {
    switch (command) {
    case 0x02:
        // write to memory
        // if we are dealing with 128K eeprom, we have 3 parameter bytes for a 24 bit address
        // otherwise it's 2 parameter bytes for a 16 bit address
    
        if (write_count < ((size == 0x20000) ? 4 : 3)) {
            address |= (data << ((size == 0x20000 ? 3 : 2 - write_count) * 8));
        } else {
            // write to the flash backup
            if (address >= size) {
                log_fatal("auxspi address is out of range");
            }

            backup[address] = data;

            address++;
        }
        break;
    case 0x03:
        // read from memory
        // if we are dealing with 128K eeprom, we have 3 parameter bytes for a 24 bit address
        // otherwise it's 2 parameter bytes for a 16 bit address
        if (write_count < ((size == 0x20000) ? 4 : 3)) {
            address |= (data << ((size == 0x20000 ? 3 : 2 - write_count) * 8));
        } else {
            // read endless stream of bytes (keep incrementing the address unless chip is deselected)
            // read data from firmware
            if (address >= size) {
                log_fatal("auxspi address is out of range");
            }

            return backup[address++];
        }
        break;
    case 0x05:
        // read status register
        return (write_in_progress ? 1 : 0) | (write_enable_latch ? (1 << 1) : 0);
    default:
        log_fatal("[EEPROMBackup] Command %02x is unimplemented", command);
    }

    return 0;
}

void EEPROMBackup::ReceiveCommand(u8 data) {
    command = data;
    address = 0;
}


void EEPROMBackup::Reset() {
    command = 0;
    address = 0;

    write_enable_latch = false;
    write_in_progress = false;

    // check if the save exists, if not then create a new save
    std::ifstream file(path, std::ios::in | std::ios::binary);

    backup.resize(size);
    backup.reserve(size);

    if (file.good()) {
        // read in the backup
        backup.insert(backup.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());
    } else {
        // fill the backup with 0xFFs
        std::fill(backup.begin(), backup.end(), 0xFF);
    }
}


void EEPROMBackup::SaveBackup() {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    std::copy(backup.begin(), backup.end(), std::ostream_iterator<u8>(file));
    file.close();
}