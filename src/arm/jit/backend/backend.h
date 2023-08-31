#pragma once

#include "arm/jit/basic_block.h"
#include "arm/jit/location.h"

namespace arm {

struct Backend {
    virtual ~Backend() = default;

    virtual void reset() = 0;
    virtual bool has_code_at(Location location) = 0;
    virtual void compile(BasicBlock& basic_block) = 0;
    virtual int run(Location location) = 0;
};

} // namespace arm