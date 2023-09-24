#pragma once

#include "arm/basic_block.h"

namespace arm {

struct Pass {
    virtual ~Pass() = default;

    virtual void optimise(BasicBlock& basic_block);
};

} // namespace arm