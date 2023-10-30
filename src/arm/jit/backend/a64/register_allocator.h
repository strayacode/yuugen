#pragma once

#include <unordered_map>
#include "common/types.h"
#include "arm/jit/basic_block.h"
#include "arm/jit/backend/a64/register.h"

namespace arm {

class RegisterAllocator {
public:
    void reset();
    void record_lifetimes(BasicBlock& basic_block);
    void advance();

private:
    u32 current_index{0};

    // maps IRVariable ids to the index of which instruction their lifetime lasts until
    std::unordered_map<u32, u32> lifetime_map;

    static constexpr WReg allocation_order[8] = {w21, w22, w23, w24, w25, w26, w27, w28};
};

} // namespace arm