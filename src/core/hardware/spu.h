#pragma once

#include "common/types.h"

namespace core {

class SPU {
public:
    void reset();
    void write_soundbias(u32 value);

private:
    u32 soundbias;
};

} // namespace core