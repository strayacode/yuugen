#pragma once

#include <array>
#include <memory>
#include "arm/cpu.h"
#include "arm/jit/ir/pass.h"

namespace arm {

class DeadLoadStoreGPREliminationPass : public Pass {
public:
    void optimise(BasicBlock& basic_block) override {
        // if its the first load for a gpr, or the last store for a gpr
        // we just record the use

        for (auto& opcode : basic_block.opcodes) {
            if (opcode->get_type() == IROpcodeType::LoadGPR) {
                auto load_gpr_opcode = *opcode->as<IRLoadGPR>();
                if (uses[load_gpr_opcode.src].is_assigned()) {
                    opcode = std::make_unique<IRCopy>(load_gpr_opcode.dst, uses[load_gpr_opcode.src]);
                }

                uses[load_gpr_opcode.src] = load_gpr_opcode.dst;
            }
        }
    }

private:
    std::array<IRVariable, 16> uses;
};

} // namespace arm