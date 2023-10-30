#pragma once

#include <unordered_map>
#include <bitset>
#include "common/types.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/backend/a64/register.h"

namespace arm {

class RegisterAllocator {
public:
    void reset();
    void record_lifetimes(BasicBlock& basic_block);
    void advance();

    // allocates a register for an ir variable
    WReg allocate(IRVariable variable);

    // gets the register corresponding to an already allocated ir variable
    WReg get(IRVariable variable);

private:
    u32 current_index{0};

    // maps IRVariable ids to the index of which instruction their lifetime lasts until
    std::unordered_map<u32, u32> lifetime_map;

    // maps IRVariable ids to an index in allocation_order
    std::unordered_map<u32, u32> variable_map;

    // keep track of which registers are currently allocated
    std::bitset<8> allocated_registers{0};

    static constexpr WReg allocation_order[8] = {w21, w22, w23, w24, w25, w26, w27, w28};
};

} // namespace arm