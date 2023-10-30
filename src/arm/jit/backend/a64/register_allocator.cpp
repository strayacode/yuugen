#include "common/logger.h"
#include "arm/jit/backend/a64/register_allocator.h"

namespace arm {

void RegisterAllocator::reset() {
    current_index = 0;
    lifetime_map.clear();
    variable_map.clear();
    allocated_registers = 0;
    temporary_registers.clear();
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
                if (!lifetime_map.contains(id)) {
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
    const u32 index = current_index;
    logger.debug("checking if any variables are last used at %d", index);

    for (auto& it : lifetime_map) {
        if (it.second == index) {
            free_variable(variable_map[it.first]);
        }
    }

    logger.debug("freeing any temporaries");
    free_temporaries();
    
    current_index++;
}

WReg RegisterAllocator::allocate(IRVariable variable) {
    for (int i = 0; i < 8; i++) {
        if (!allocated_registers.test(i)) {
            WReg reg = allocation_order[i];

            allocated_registers.set(i);
            variable_map.insert({variable.id, i});
            logger.debug("allocate %s into w%d", variable.to_string().c_str(), reg.id);
            return reg;
        }
    }

    logger.todo("all registers are being used!");
    return WReg{0xffffffff};
}

WReg RegisterAllocator::allocate_temporary() {
    for (int i = 0; i < 8; i++) {
        if (!allocated_registers.test(i)) {
            WReg reg = allocation_order[i];

            allocated_registers.set(i);
            temporary_registers.push_back(i);
            logger.debug("allocate temporary into w%d", reg.id);
            return reg;
        }
    }

    logger.todo("all registers are being used!");
    return WReg{0xffffffff};
}

WReg RegisterAllocator::get(IRVariable variable) {
    auto it = variable_map.find(variable.id);
    if (it == variable_map.end()) {
        logger.todo("%s wasn't allocated when it should be", variable.to_string().c_str());
    }

    return allocation_order[it->second];
}

void RegisterAllocator::free_temporaries() {
    for (auto& id : temporary_registers) {
        WReg reg = allocation_order[id];
        logger.debug("free temporary register w%d", reg.id);
        allocated_registers.reset(id);
    }

    temporary_registers.clear();
}

void RegisterAllocator::free_variable(u32 var_id) {
    u32 id = variable_map[var_id];
    logger.debug("w%d available for use", allocation_order[id].id);
    allocated_registers.reset(id);
    variable_map.erase(var_id);
    logger.debug("v%d deallocated from variable map", var_id);
}

} // namespace arm