#include "Core/Hardware/Cartridge/backup/no_backup/no_backup.h"

NoBackup::NoBackup(std::string path, u32 size) {
    // do nothing
}

auto NoBackup::Transfer(u8 data, u32 write_count) -> u8 {
    return 0;
}

void NoBackup::ReceiveCommand(u8 data) {
    // do nothing
}

void NoBackup::Reset() {
    // do nothing
}

void NoBackup::SaveBackup() {
    // do nothing
} 