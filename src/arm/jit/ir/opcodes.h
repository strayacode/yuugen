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
        return common::format("mov%s %s, %s", set_flags ? "s" : "", dst.to_string().c_str(), src.to_string().c_str());
    }

    IRVariable dst;
    IRValue src;
    bool set_flags;
};

} // namespace arm