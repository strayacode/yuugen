#pragma once

#include <vector>
#include <memory>
#include "arm/jit/ir/opcodes.h"

namespace arm {

class BasicBlock {
public:

private:
    std::vector<std::unique_ptr<IROpcode>> opcodes;
};

} // namespace arm