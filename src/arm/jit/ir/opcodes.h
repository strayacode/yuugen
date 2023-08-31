#pragma once

#include <string>
#include "common/string.h"
#include "arm/jit/ir/types.h"

namespace arm {

struct IROpcode {
    virtual ~IROpcode() = default;
    virtual std::string to_string() = 0;
};

struct IRSetCarry : IROpcode {
    std::string to_string() override {
        return "sec";
    }
};

struct IRClearCarry : IROpcode {
    std::string to_string() override {
        return "clc";
    }
};

struct IRMove : IROpcode {
    IRMove(IRVariable dst, IRValue src, bool set_flags) : dst(dst), src(src), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("mov%s %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    bool set_flags;
};

struct IRLoadGPR : IROpcode {
    IRLoadGPR(IRVariable dst, GuestRegister src) : dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("ld %s, %s", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    GuestRegister src;
};

struct IRStoreGPR : IROpcode {
    IRStoreGPR(GuestRegister dst, IRVariable src) : dst(dst), src(src) {}

    std::string to_string() override {
        return common::format("st %s, %s", dst.to_string().c_str(), src.to_string().c_str());
    }

    GuestRegister dst;
    IRVariable src;
};

struct IRAdd : IROpcode {
    IRAdd(IRVariable dst, IRValue lhs, IRValue rhs, bool set_flags) : dst(dst), lhs(lhs), rhs(rhs), set_flags(set_flags) {}

    std::string to_string() override {
        return common::format("add%s %s, %s, %s", set_flags ? ".s" : "", dst.to_string().c_str(), lhs.to_string().c_str(), rhs.to_string().c_str());
    }

    IRVariable dst;
    IRValue lhs;
    IRValue rhs;
    bool set_flags;
};

} // namespace arm