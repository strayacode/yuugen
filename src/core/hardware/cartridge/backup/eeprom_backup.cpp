#include <fstream>
#include <iterator>
#include <iostream>
#include <algorithm>
#include "common/logger.h"
#include "core/hardware/cartridge/backup/eeprom_backup.h"

namespace core {

EEPROMBackup::EEPROMBackup(const std::string& path, u32 size) : path(path), size(size) {
    reset();
}

void EEPROMBackup::reset() {
    command = 0;
    address = 0;

    write_enable_latch = false;
    write_in_progress = false;

    // check if the save exists, if not then create a new save
    std::ifstream file(path, std::ios::in | std::ios::binary);

    backup.resize(size);
    backup.reserve(size);

    if (file.good()) {
        backup.insert(backup.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());
    } else {
        std::fill(backup.begin(), backup.end(), 0xFF);
    }
}

void EEPROMBackup::save() {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    std::copy(backup.begin(), backup.end(), std::ostream_iterator<u8>(file));
    file.close();
}

u8 EEPROMBackup::transfer(u8 data, u32 write_count) {
    switch (command) {
    case 0x02:
        if (write_count < ((size == 0x20000) ? 4 : 3)) {
            address |= data << ((size == 0x20000 ? 3 : 2 - write_count) * 8);
        } else {
            if (address >= size) {
                logger.error("EEPROMBackup: address is out of range");
            }

            backup[address] = data;

            address++;
        }
        break;
    case 0x03:
        if (write_count < ((size == 0x20000) ? 4 : 3)) {
            address |= data << ((size == 0x20000 ? 3 : 2 - write_count) * 8);
        } else {
            if (address >= size) {
                logger.error("EEPROMBackup: address is out of range");
            }

            return backup[address++];
        }
        break;
    case 0x05:
        return (write_in_progress ? 1 : 0) | (write_enable_latch ? (1 << 1) : 0);
    default:
        logger.error("EEPROMBackup: handle command %02x", command);
    }

    return 0;
}

void EEPROMBackup::receive(u8 data) {
    command = data;
    address = 0;
}

u32 EEPROMBackup::get_size() {
    return size;
}

} // namespace core