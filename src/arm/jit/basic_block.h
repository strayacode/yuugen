#pragma once

#include <vector>
#include <memory>
#include <string>
#include "common/bits.h"
#include "common/logger.h"
#include "arm/jit/ir/opcodes.h"
#include "arm/state.h"
#include "arm/cpu.h"

namespace arm {

struct BasicBlock {
    using JitFunction = void *;

    struct Key {
        Key(State& state) {
            value = state.gpr[15] >> 1;
            value |= common::get_field<0, 6>(static_cast<u64>(state.cpsr.data)) << 31;
        }

        u32 get_address() {
            return common::get_field<0, 31>(value) << 1;
        }

        Mode get_mode() {
            return static_cast<Mode>(common::get_field<31, 5>(value));
        }

        bool is_arm() {
            return !common::get_bit<36>(value);
        }

        u64 value = 0;
    };

    BasicBlock(Key key) : key(key) {}

    void dump() {
        auto instruction_size = key.is_arm() ? sizeof(u32) : sizeof(u16);
        u32 pc = key.get_address() - 2 * instruction_size;
        logger.debug("basic block key: %016lx address: %08x arm: %d mode: %02x", key, pc, key.is_arm(), static_cast<u8>(key.get_mode()));
        
        for (auto& opcode : opcodes) {
            logger.debug("%s\n", opcode->to_string().c_str());
        }
    }

    Key key;
    Condition condition;
    JitFunction jit_function = nullptr;
    std::vector<std::unique_ptr<IROpcode>> opcodes;
};

} // namespace arm