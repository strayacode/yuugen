#pragma once

#include <unordered_set>
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/pass.h"

namespace arm {

class DeadCodeEliminationPass : public Pass {
public:
    void optimise(BasicBlock& basic_block) override;

private:

    // marks if variables are used or not
    std::unordered_set<u32> uses;
};

} // namespace arm