#pragma once

#include <vector>
#include <fstream>
#include <iterator>
#include <iostream>
#include <algorithm>
#include "Common/Log.h"
#include "Common/Types.h"
#include "Core/HW/cartridge/backup/generic_backup.h"

struct EEPROMBackup : public GenericBackup {
    EEPROMBackup(std::string path, u32 size);
    
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