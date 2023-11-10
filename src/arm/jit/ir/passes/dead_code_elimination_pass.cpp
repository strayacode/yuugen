#include "common/logger.h"
#include "arm/jit/ir/passes/dead_code_elimination_pass.h"

namespace arm {

void DeadCodeEliminationPass::optimise(BasicBlock& basic_block) {
    uses.clear();

    auto it = basic_block.opcodes.rbegin();
    auto end = basic_block.opcodes.rend();
    while (it != end) {
        auto& opcode = *it;
        auto parameters = opcode->get_parameters();
        auto destinations = opcode->get_destinations();

        bool erase_opcode = false;
        for (auto& destination : destinations) {
            // remove the opcode if its destination isn't used anywhere
            if (!uses.contains(destination->id)) {
                erase_opcode = true;
            } else {
                erase_opcode = false;
                break;
            }
        }

        if (erase_opcode) {
            it = std::reverse_iterator(basic_block.opcodes.erase(std::next(it).base()));
            mark_modified();
        } else {
            // only add an opcode's uses if it wasn't erased
            for (auto& parameter : parameters) {
                if (parameter->is_variable()) {
                    uses.insert(parameter->as_variable().id);
                }
            }

            it++;
        }
    }
}

} // namespace arm