#pragma once

#include "common/types.h"

namespace nds {

struct Backup {
    virtual ~Backup() = default;
    virtual void reset() = 0;
    virtual void save() = 0;
    virtual u8 transfer(u8 data, u32 write_count) = 0;
    virtual void receive(u8 data) = 0;
    virtual u32 get_size() = 0; 
};

} // namespace nds