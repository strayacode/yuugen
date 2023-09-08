#pragma once

#include <vector>
#include <memory>
#include <string>
#include "common/bits.h"
#include "common/logger.h"
#include "arm/jit/location.h"
#include "arm/jit/ir/opcodes.h"
#include "arm/state.h"
#include "arm/cpu.h"

namespace arm {

struct BasicBlock {
    BasicBlock(Location location) : location(location) {}

    void dump() {
        auto instruction_size = location.get_instruction_size();
        u32 pc = location.get_address() - 2 * instruction_size;
        logger.debug("basic block location: %016lx address: %08x arm: %d mode: %02x condition: %02x", location, pc, location.is_arm(), static_cast<u8>(location.get_mode()), condition);
        logger.debug("cycles: %d", cycles);
        for (auto& opcode : opcodes) {
            logger.debug("%s", opcode->to_string().c_str());
        }
    }

    Location location;
    Condition condition;
    int cycles{0};
    int num_instructions{0};
    std::vector<std::unique_ptr<IROpcode>> opcodes;
};

} // namespace arm