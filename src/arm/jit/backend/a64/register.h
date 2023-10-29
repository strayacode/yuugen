#pragma once

#include "common/types.h"

namespace arm {

struct Reg {
    Reg(u32 id, u32 bits) : id(id), bits(bits) {}

    u32 id;
    u32 bits;
};

struct WReg : public Reg {
    WReg(u32 id) : Reg(id, 32) {}
};

struct XReg : public Reg {
    XReg(u32 id) : Reg(id, 64) {}
};

} // namespace arm;