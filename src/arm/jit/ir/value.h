#pragma once

#include <type_traits>
#include "common/types.h"
#include "common/string.h"
#include "arm/cpu.h"
#include "arm/state.h"

namespace arm {

enum class Type : u8 {
    U1 = 1 << 0,
    U8 = 1 << 1,
    U16 = 1 << 2,
    U32 = 1 << 3,
    U64 = 1 << 4,
};

enum class IRValueType {
    Variable,
    Constant,
    None,
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

    u32 value{0};
};

struct IRValue {
    IRValue() : type(IRValueType::None) {}
    IRValue(bool value) : type(IRValueType::Constant), constant(IRConstant{value}) {}
    IRValue(u8 value) : type(IRValueType::Constant), constant(IRConstant{value}) {}
    IRValue(u32 value) : type(IRValueType::Constant), constant(IRConstant{value}) {}
    IRValue(IRVariable variable) : type(IRValueType::Variable), variable(variable) {}
    IRValue(IRConstant constant) : type(IRValueType::Constant), constant(constant) {}

    bool is_variable() {
        return type == IRValueType::Variable;
    }

    bool is_constant() {
        return type == IRValueType::Constant;
    }

    bool is_equal(u32 value) {
        return type == IRValueType::Constant && constant.value == value;
    }

    bool is_equal(IRValue value) const {
        if (type != value.type) {
            return false;
        }

        if (type == IRValueType::Constant) {
            return constant.value == value.as_constant().value;
        } else if (type == IRValueType::Variable) {
            return variable.id == value.as_variable().id;
        }

        return false;
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

    bool is_assigned() {
        return type != IRValueType::None;
    }

    IRValueType type;

    union {
        IRVariable variable;
        IRConstant constant;
    };
};

template <Type T>
struct TypedValue : public IRValue {
    TypedValue() : IRValue() {}

    explicit TypedValue(IRValue value) : IRValue(value) {}

    explicit TypedValue(bool value) : IRValue(value) {
        static_assert(TypedValue::is_equal<T, Type::U1>());
    }

    explicit TypedValue(u8 value) : IRValue(value) {
        static_assert(TypedValue::is_equal<T, Type::U8>());
    }

    explicit TypedValue(u32 value) : IRValue(value) {
        static_assert(TypedValue::is_equal<T, Type::U32>());
    }

    explicit TypedValue(IRVariable variable) : IRValue(variable) {}

    template <Type A, Type B>
    static constexpr bool is_equal() {
        return A == B;
    }

    template <Type A, Type B>
    static constexpr bool is_larger() {
        return A > B;
    }

    template <Type A, Type B>
    static constexpr bool is_smaller() {
        return A < B;
    }
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

    int get_id() {
        auto start = mode == Mode::FIQ ? GPR::R8 : GPR::R13;
        if (gpr < start || gpr == GPR::PC) {
            return static_cast<int>(gpr);
        }

        if (mode == Mode::USR) {
            return (static_cast<int>(Mode::SYS) << 4) | static_cast<int>(gpr);
        } else {
            return (static_cast<int>(mode) << 4) | static_cast<int>(gpr);
        }
    }
};

template <typename T>
struct IRPair {
    T first;
    T second;

    IRPair(T first, T second) : first(first), second(second) {}

    std::string to_string() {
        return common::format("%s, %s", first.to_string().c_str(), second.to_string().c_str());
    }
};

enum Flag : u32 {
    N = 31,
    Z = 30,
    C = 29,
    V = 28,
    Q = 27,
    I = 7,
    T = 5,
};

} // namespace arm