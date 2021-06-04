#pragma once

#include <common/log.h>
#include <common/types.h>
#include <core/hw/cartridge/backup/generic_backup.h>
#include <string>

struct NoBackup : public GenericBackup {
    NoBackup(std::string path, u32 size);
    auto Transfer(u8 data, u32 write_count) -> u8 override;
    void ReceiveCommand(u8 data) override;
    void Reset() override;
    void SaveBackup() override;
};