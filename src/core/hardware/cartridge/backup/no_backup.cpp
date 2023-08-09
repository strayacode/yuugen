#include "core/hardware/cartridge/backup/no_backup.h"

namespace core {

void NoBackup::reset() {}

void NoBackup::save() {} 

u8 NoBackup::transfer(u8 data, u32 write_count) { return 0; }

void NoBackup::receive(u8 data) {}

} // namespace core