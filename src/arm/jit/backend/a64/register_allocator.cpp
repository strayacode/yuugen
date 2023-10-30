#include "common/logger.h"
#include "arm/jit/backend/a64/register_allocator.h"

namespace arm {

void RegisterAllocator::reset() {
    current_index = 0;
    lifetime_map.clear();
}

void RegisterAllocator::record_lifetimes(BasicBlock& basic_block) {
    auto it = basic_block.opcodes.rbegin();
    auto end = basic_block.opcodes.rend();
    int index = basic_block.opcodes.size() - 1;

    while (it != end) {
        auto& opcode = *it;
        auto parameters = opcode->get_parameters();
        for (auto& parameter : parameters) {
            if (parameter->is_variable()) {
                const u32 id = parameter->as_variable().id;
                if (!lifetime_map[id]) {
                    logger.debug("detected use of %s in %s last use at instruction %d", parameter->to_string().c_str(), opcode->to_string().c_str(), index);
                    lifetime_map[id] = index;
                }
            }
        }

        it++;
        index--;
    }
}

void RegisterAllocator::advance() {
    current_index++;
}

} // namespace arm