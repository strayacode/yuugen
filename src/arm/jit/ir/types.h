#pragma once

#include "arm/cpu.h"
#include "arm/state.h"

namespace arm {

enum class IRValueType {
    Variable,
    Constant,
}

struct IRVariable {
    IRVariable(int id) : id(id) {}

    int id;
};

struct IRConstant {

};

struct IRValue {
    IRValue(IRVariable variable) : type(IRValueType::Variable), variable(variable) {}
    IRValue(IRConstant constant) : type(IRValueType::Constant), constant(constant) {}

    IRValueType type;

    union {
        IRVariable variable;
        IRConstant constant;
    };
};

struct GuestRegister {
    GPR gpr;
    Mode mode;
};

} // namespace arm