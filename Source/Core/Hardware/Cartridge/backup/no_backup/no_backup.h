#pragma once

#include <string>
#include "Common/Log.h"
#include "Common/Types.h"
#include "Core/Hardware/Cartridge/backup/generic_backup.h"

class NoBackup : public GenericBackup {
public:
    NoBackup(std::string path, u32 size);
    u8 Transfer(u8 data, u32 write_count) override;
    void ReceiveCommand(u8 data) override;
    void Reset() override;
    void SaveBackup() override;
};