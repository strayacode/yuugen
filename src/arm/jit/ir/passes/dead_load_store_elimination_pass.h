#pragma once

#include <array>
#include <memory>
#include "arm/cpu.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/ir/pass.h"

namespace arm {

class DeadLoadStoreEliminationPass : public Pass {
public:
    void optimise(BasicBlock& basic_block) override;

private:
    std::array<IRValue, 512> gpr_uses;
    IRValue cpsr_use;
    IRValue spsr_use;
};

} // namespace arm