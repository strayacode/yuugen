#pragma once

#include "common/types.h"
#include "common/string.h"
#include "arm/cpu.h"
#include "arm/state.h"

namespace arm {

enum class IRValueType {
    Variable,
    Constant,
};

struct IRVariable {
    IRVariable(int id) : id(id) {}

    std::string to_string() {
        return common::format("%%%d", id);
    }

    int id;
};

struct IRConstant {
    IRConstant(u32 value) : value(value) {}

    std::string to_string() {
        return common::format("0x%08x", value);
    }

    u32 value;
};

struct IRValue {
    IRValue() {}
    IRValue(IRVariable variable) : type(IRValueType::Variable), variable(variable) {}
    IRValue(IRConstant constant) : type(IRValueType::Constant), constant(constant) {}

    std::string to_string() {
        if (type == IRValueType::Variable) {
            return variable.to_string();
        } else {
            return constant.to_string();
        }
    }

    IRValueType type;

    union {
        IRVariable variable;
        IRConstant constant;
    };
};

struct GuestRegister {
    GPR gpr;
    Mode mode;

    std::string to_string() {
        auto start = mode == Mode::FIQ ? GPR::R8 : GPR::R13;
        auto has_spsr = mode != Mode::USR && mode != Mode::SYS;
        if (has_spsr && gpr >= start && gpr <= GPR::R14) {
            return common::format("r%d_%s", gpr, mode_to_string(mode).c_str());
        } else {
            return common::format("r%d", gpr);
        }
    }
};

} // namespace arm