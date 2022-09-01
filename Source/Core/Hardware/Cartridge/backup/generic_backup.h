#pragma once

#include "Common/Types.h"

struct GenericBackup {
    virtual ~GenericBackup() = default;
    virtual auto Transfer(u8 data, u32 write_count) -> u8 = 0;
    virtual void ReceiveCommand(u8 data) = 0;
    virtual void Reset() = 0;
    virtual void SaveBackup() = 0;
};