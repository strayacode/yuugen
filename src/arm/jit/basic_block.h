#pragma once

#include <vector>
#include <memory>
#include <string>
#include "common/bits.h"
#include "common/string.h"
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

        bool is_arm() {
            return !common::get_bit<36>(value);
        }

        u64 value = 0;
    };

    BasicBlock(Key key) : key(key) {}

    std::string dump() {
        std::string message;
        u32 pc = key.get_address();
        auto instruction_size = key.is_arm() ? sizeof(u32) : sizeof(u16);

        for (auto& opcode : opcodes) {
            message += common::format("%08x: %s\n", pc, opcode->to_string().c_str());
            pc += instruction_size;
        }

        return message;
    }

    Key key;
    Condition condition;
    JitFunction jit_function = nullptr;
    std::vector<std::unique_ptr<IROpcode>> opcodes;
};

} // namespace arm