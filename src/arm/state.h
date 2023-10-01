#pragma once

#include <array>
#include <string>
#include "common/types.h"

namespace arm {

enum class Mode : u8 {
    USR = 0x10,
    FIQ = 0x11,
    IRQ = 0x12,
    SVC = 0x13,
    ABT = 0x17,
    UND = 0x1b,
    SYS = 0x1f,
};

inline std::string mode_to_string(Mode mode) {
    switch (mode) {
    case Mode::USR:
        return "usr";
    case Mode::FIQ:
        return "fiq";
    case Mode::IRQ:
        return "irq";
    case Mode::SVC:
        return "svc";
    case Mode::ABT:
        return "abt";
    case Mode::UND:
        return "und";
    case Mode::SYS:
        return "sys";
    default:
        return "...";
    }
}

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
    StatusRegister cpsr;
    StatusRegister* spsr;
    std::array<StatusRegister, 6> spsr_banked;
};

} // namespace arm