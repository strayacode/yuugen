#include "arm/jit/ir/passes/dead_copy_elimination_pass.h"

namespace arm {

void DeadCopyEliminationPass::optimise(BasicBlock& basic_block) {
    uses.clear();

    auto it = basic_block.opcodes.rbegin();
    auto end = basic_block.opcodes.rend();

    while (it != end) {
        auto& opcode = *it;
        if (opcode->get_type() == IROpcodeType::Copy) {
            auto copy_opcode = *opcode->as<IRCopy>();
            if (copy_opcode.src.is_variable()) {
                auto& src_variable = copy_opcode.src.as_variable();
                for (auto& use : uses) {
                    if (use->id == copy_opcode.dst.id) {
                        use->id = src_variable.id;
                    }
                }

                it = std::reverse_iterator(basic_block.opcodes.erase(std::next(it).base()));
                mark_modified();
            } else {
                it++;
            }
        } else {
            auto parameters = opcode->get_parameters();
            for (auto& parameter : parameters) {
                if (parameter->is_variable()) {
                    uses.push_back(&parameter->as_variable());
                }
            }

            it++;
        }
    }
}

} // namespace arm