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
            logger.debug("potential dead copy %s detected", copy_opcode.to_string().c_str());
        } else {
            logger.todo("handle this");
        }

        it++;
    }
}

// v0 = copy(v1)
// v2 = copy(v0)
// v3 = copy(v2)
// v4 = add(v3, 0)

// record all arguments used (variables only)
// go in reverse
// if we encounter a copy with the src being a variable, then we loop through our uses vector
// if any variable has a matching index with the copy dst, then update those to have the index of copy src

// encounter v3
// see that v3 in uses for copy(v2)
// update v3 to be v2
// dont add v2 from copy(v2) as used, as the opcode gets removed
// encounter v2 = copy(v0)
// check uses vector
// see that v2 is used
// replace v2 with v0
// remove copy
// do same for v0 = copy(v1)

// actual basic block:
// record all uses into a std::vector<IRVariable&> structure
// encounter v7 = copy(v6)
// loop through uses, replace v7 with v6
// remove v7 = copy(v6)
// encounter v5 = copy(v2)
// loop through uses, replace v5 with v2 in uses
// remove v5 = copy(v2)

} // namespace arm