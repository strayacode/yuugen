#pragma once

#include <common/types.h>

struct GenericBackup {
    virtual auto Transfer(u8 data, u32 write_count) -> u8 = 0;
    virtual void ReceiveCommand(u8 command) = 0;
    virtual void Reset() = 0;
    virtual void SaveBackup() = 0;
};