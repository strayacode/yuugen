#pragma once

#include <vector>
#include <string>
#include "common/types.h"
#include "nds/hardware/cartridge/backup/backup.h"

namespace nds {

class FlashBackup : public Backup {
public:
    FlashBackup(const std::string& path, u32 size);

    void reset() override;
    void save() override;
    u8 transfer(u8 data, u32 write_count) override;
    void receive(u8 data) override;
    u32 get_size() override;
    
    const std::string& path;

    u32 size;
    u8 command;
    u32 address;
    std::vector<u8> backup;
    bool write_enable_latch;
    bool write_in_progress;
};

} // namespace nds