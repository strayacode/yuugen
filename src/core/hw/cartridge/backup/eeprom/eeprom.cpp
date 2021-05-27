#include <core/hw/cartridge/backup/eeprom/eeprom.h>

EEPROMBackup::EEPROMBackup(std::string path, u32 size) : path(path), size(size) {
    Reset();
}
    
auto EEPROMBackup::Transfer(u8 data, u32 write_count) -> u8 {
    switch (command) {
    case 0x03:
        // read from memory
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

}