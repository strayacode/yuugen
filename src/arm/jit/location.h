#pragma once

#include "arm/state.h"

namespace arm {

struct Location {
    Location(State& state) {
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

    u32 get_instruction_size() {
        return is_arm() ? sizeof(u32) : sizeof(u16);
    }

    u64 value = 0;
};

} // namespace arm