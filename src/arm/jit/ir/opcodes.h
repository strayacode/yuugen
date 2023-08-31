#pragma once

#include <string>
#include "common/string.h"
#include "common/logger.h"
#include "arm/jit/ir/types.h"

namespace arm {

enum class IROpcodeType {
    SetCarry,
    ClearCarry,
    Move,
    LoadGPR,
    StoreGPR,
    Add,
};

struct IROpcode {
    IROpcode(IROpcodeType type) : type(type) {}

    virtual ~IROpcode() = default;
    virtual std::string to_string() = 0;

    IROpcodeType get_type() {
        return type;
    }

    template <typename T>
    T* as() {
        return reinterpret_cast<T*>(this);
    }

    IROpcodeType type;
};

struct IRSetCarry : IROpcode {
    IRSetCarry() : IROpcode(IROpcodeType::SetCarry) {}

    std::string to_string() override {
        return "sec";
    }
};

struct IRClearCarry : IROpcode {
    IRClearCarry() : IROpcode(IROpcodeType::ClearCarry) {}

    std::string to_string() override {
        return "clc";
    }
};

struct IRMove : IROpcode {
    IRMove(IRVariable dst, IRValue src, bool set_flags) : IROpcode(IROpcodeType::Move), dst(dst), src(src), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("mov%s %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    bool set_flags;
};

struct IRLoadGPR : IROpcode {
    IRLoadGPR(IRVariable dst, GuestRegister src) : IROpcode(IROpcodeType::LoadGPR), dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("ld %s, %s", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    GuestRegister src;
};

struct IRStoreGPR : IROpcode {
    IRStoreGPR(GuestRegister dst, IRValue src) : IROpcode(IROpcodeType::StoreGPR), dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("st %s, %s", dst.to_string().c_str(), src.to_string().c_str());
    }

    GuestRegister dst;
    IRValue src;
};

struct IRAdd : IROpcode {
    IRAdd(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : IROpcode(IROpcodeType::Add), dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("add%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

} // namespace arm