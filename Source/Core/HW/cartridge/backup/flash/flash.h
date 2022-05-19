#pragma once

#include "Common/Log.h"
#include "Common/Types.h"
#include "Core/HW/cartridge/backup/generic_backup.h"
#include <vector>
#include <fstream>
#include <iterator>
#include <iostream>
#include <algorithm>

enum FlashBackupSize : u32 {
    SIZE_256K = 0x40000,
    SIZE_512K = 0x80000,
    SIZE_1024K = 0x100000,
    SIZE_8192K = 0x800000,
};

struct FlashBackup : public GenericBackup {
    FlashBackup(std::string path, u32 size);
    auto Transfer(u8 data, u32 write_count) -> u8 override;
    void ReceiveCommand(u8 data) override;
    void Reset() override;
    void SaveBackup() override;

    std::string path;

    u32 size;

    u8 command;

    u32 address;

    std::vector<u8> backup;

    bool write_enable_latch;
    bool write_in_progress;
};