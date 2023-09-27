#pragma once

#include <vector>
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/value.h"
#include "arm/jit/ir/pass.h"

namespace arm {

class DeadCopyEliminationPass : public Pass {
public:
    void optimise(BasicBlock& basic_block) override;

private:
    std::vector<IRVariable*> uses;
};

} // namespace arm