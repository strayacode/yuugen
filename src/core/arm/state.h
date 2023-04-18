#pragma once

#include <array>
#include "common/types.h"

namespace core::arm {

enum class Mode : u8 {
    USR = 0x10,
    FIQ = 0x11,
    IRQ = 0x12,
    SVC = 0x13,
    ABT = 0x17,
    UND = 0x1b,
    SYS = 0x1f,
};

union StatusRegister {
    struct {
        Mode mode : 5;
        bool t : 1;
        bool f : 1;
        bool i : 1;
        u32 : 19;
        bool q : 1;
        bool v : 1;
        bool c : 1;
        bool z : 1;
        bool n : 1;
    };

    u32 data;
};

struct State {
    std::array<u32, 16> gpr;
    std::array<std::array<u32, 7>, 6> gpr_banked;

    // TODO: change this to be a pointer or similar
    StatusRegister cpsr;
    std::array<StatusRegister, 6> spsr_banked;
};

} // namespace core::arm