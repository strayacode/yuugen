#pragma once

#include "arm/jit/basic_block.h"
#include "arm/jit/location.h"
#include "arm/jit/backend/code.h"

namespace arm {

struct Backend {
    virtual ~Backend() = default;

    virtual void reset() = 0;
    virtual Code get_code_at(Location location) = 0;
    virtual Code compile(BasicBlock& basic_block) = 0;
    virtual int run(Code code, int cycles_left) = 0;
};

} // namespace arm