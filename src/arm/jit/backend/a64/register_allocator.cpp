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

        // Record the last use of any variables.
        auto parameters = opcode->get_parameters();
        for (auto& parameter : parameters) {
            if (parameter->is_variable()) {
                const u32 id = parameter->as_variable().id;
                if (!lifetime_map.contains(id)) {
                    lifetime_map[id] = index;
                }
            }
        }

        // If the above didn't record the lifetime, then use
        // the destination as the last use
        auto destinations = opcode->get_destinations();
        for (auto& destination : destinations) {
            if (destination->is_variable()) {
                const u32 id = destination->as_variable().id;
                if (!lifetime_map.contains(id)) {
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
    
    for (auto& it : lifetime_map) {
        if (it.second == index) {
            free_variable(it.first);
        }
    }

    free_temporaries();
    current_index++;
}

WReg RegisterAllocator::allocate(IRValue variable) {
    for (int i = 0; i < 16; i++) {
        if (!allocated_registers.test(i)) {
            WReg reg = allocation_order[i];

            allocated_registers.set(i);
            variable_map.insert({variable.as_variable().id, i});
            return reg;
        }
    }

    LOG_TODO("all registers are being used!");
    return WReg{0xffffffff};
}

WReg RegisterAllocator::allocate_temporary() {
    for (int i = 0; i < 16; i++) {
        if (!allocated_registers.test(i)) {
            WReg reg = allocation_order[i];

            allocated_registers.set(i);
            temporary_registers.push_back(i);
            return reg;
        }
    }

    LOG_TODO("all registers are being used!");
    return WReg{0xffffffff};
}

WReg RegisterAllocator::get(IRValue variable) {
    auto it = variable_map.find(variable.as_variable().id);
    if (it == variable_map.end()) {
        LOG_TODO("%s wasn't allocated when it should be", variable.as_variable().to_string().c_str());
    }

    return allocation_order[it->second];
}

void RegisterAllocator::free_temporaries() {
    for (auto& id : temporary_registers) {
        allocated_registers.reset(id);
    }

    temporary_registers.clear();
}

void RegisterAllocator::free_variable(u32 var_id) {
    u32 id = variable_map[var_id];
    allocated_registers.reset(id);
    variable_map.erase(var_id);
}

} // namespace arm