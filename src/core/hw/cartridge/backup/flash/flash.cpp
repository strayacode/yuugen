#include <core/hw/cartridge/backup/flash/flash.h>

FlashBackup::FlashBackup(std::string path, u32 size) : path(path), size(size) {
    Reset();
}

auto FlashBackup::Transfer(u8 data, u32 write_count) -> u8 {
    switch (command) {
    case 0x03:
        // what we do will depend on the write count
        // on write_count < 4, we will set up the 3 byte address
        // on write_count >= 4, we will finally read a byte from the firmware
        // in setting up the 3 byte address the most significant byte is done first
        if (write_count < 4) {
            address |= (data << ((3 - write_count) * 8));
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
    case 0x0A:
        // page write (use 3 byte address, write 1..256 bytes)
        if (write_count < 4) {
            // set up the 3 byte address
            address |= (data << ((3 - write_count) * 8));
        } else {
            // write to the flash backup
            if (address >= size) {
                log_fatal("auxspi address is out of range");
            }

            backup[address] = data;

            address++;
        }
        break;
    default:
        log_fatal("[FlashBackup] Command %02x is unimplemented", command);
    }

    return 0;
}

void FlashBackup::ReceiveCommand(u8 data) {
    command = data;
    address = 0;
}

void FlashBackup::Reset() {
    command = 0;
    address = 0;

    write_enable_latch = false;
    write_in_progress = false;

    // // check if the save exists, if not then create a new save
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

void FlashBackup::SaveBackup() {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    std::copy(backup.begin(), backup.end(), std::ostream_iterator<u8>(file));
    file.close();
}