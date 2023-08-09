#pragma once

#include "common/types.h"
#include "core/hardware/cartridge/backup/backup.h"

namespace core {

class NoBackup : public Backup {
public:
    NoBackup() = default;

    void reset() override;
    void save() override;
    u8 transfer(u8 data, u32 write_count) override;
    void receive(u8 data) override;
};

} // namespace core