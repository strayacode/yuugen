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
    IRVariable() {}
    IRVariable(u32 id) : id(id) {}

    std::string to_string() {
        return common::format("v%d", id);
    }

    bool is_assigned() {
        return id != 0xffffffff;
    }

    u32 id{0xffffffff};
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

    bool is_constant() {
        return type == IRValueType::Constant;
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

enum Flag : u32 {
    N = (static_cast<u32>(1) << 31),
    Z = (static_cast<u32>(1) << 30),
    C = (static_cast<u32>(1) << 29),
    V = (static_cast<u32>(1) << 28),
};

} // namespace arm