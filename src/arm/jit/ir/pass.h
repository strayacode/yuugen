#pragma once

#include "arm/jit/basic_block.h"

namespace arm {

struct Pass {
    virtual ~Pass() = default;
    virtual void optimise(BasicBlock& basic_block) = 0;
};

} // namespace arm