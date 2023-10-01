#pragma once

#include <vector>
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/pass.h"

namespace arm {

class Optimiser {
public:
    void optimise(BasicBlock& basic_block);
    void add_pass(std::unique_ptr<Pass> pass);

private:
    std::vector<std::unique_ptr<Pass>> passes;
};

} // namespace arm