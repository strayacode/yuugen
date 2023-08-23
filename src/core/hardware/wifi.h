#pragma once

#include "common/types.h"

namespace core {

class Wifi {
public:
    void reset();
    void write_wifiwaitcnt(u16 value, u32 mask);

private:
    u16 wifiwaitcnt;
};

} // namespace core