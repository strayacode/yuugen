#include <fstream>
#include <iterator>
#include <iostream>
#include <algorithm>
#include "common/logger.h"
#include "nds/hardware/cartridge/backup/flash_backup.h"

namespace nds {

FlashBackup::FlashBackup(const std::string& path, u32 size) : path(path), size(size) {
    reset();
}

void FlashBackup::reset() {
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

void FlashBackup::save() {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    std::copy(backup.begin(), backup.end(), std::ostream_iterator<u8>(file));
    file.close();
}

u8 FlashBackup::transfer(u8 data, u32 write_count) {
    switch (command) {
    case 0x00:
        // ignore this for now, seems to a command specific to ir stuff for pokemon games
        break;
    case 0x03:
        if (write_count < 4) {
            address |= data << ((3 - write_count) * 8);
        } else {
            if (address >= size) {
                logger.error("FlashBackup: address is out of range");
            }

            return backup[address++];
        }
        break;
    case 0x05:
        return (write_in_progress ? 1 : 0) | (write_enable_latch ? (1 << 1) : 0);
    case 0x08:
        // stubbed for games that have IR support
        return 0xaa;
    case 0x0a:
        if (write_count < 4) {
            address |= (data << ((3 - write_count) * 8));
        } else {
            if (address >= size) {
                logger.error("FlashBackup: address is out of range");
            }

            backup[address++] = data;
        }

        break;
    default:
        logger.todo("FlashBackup: handle command %02x", command);
        break;
    }

    return 0;
}

void FlashBackup::receive(u8 data) {
    command = data;
    address = 0;
}

u32 FlashBackup::get_size() {
    return size;
}

} // namespace nds