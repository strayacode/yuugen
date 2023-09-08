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
    IRVariable(u32 id) : id(id) {}

    std::string to_string() {
        return common::format("%%%d", id);
    }

    u32 id;
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

    bool is_variable() {
        return type == IRValueType::Variable;
    }

    IRVariable& as_variable() {
        return variable;
    }

    IRConstant& as_constant() {
        return constant;
    }

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

enum Flags : u8 {
    N = (1 << 3),
    Z = (1 << 2),
    C = (1 << 1),
    V = (1 << 0),
    None = 0,
    NZ = N | Z,
    NZCV = N | Z | C | V,
};

} // namespace arm