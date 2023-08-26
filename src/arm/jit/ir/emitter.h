#pragma once

#include "arm/jit/basic_block.h"

namespace arm {

class Emitter {
public:
    Emitter(BasicBlock& basic_block);

private:
    BasicBlock& basic_block;
};

} // namespace arm