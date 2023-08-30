#pragma once

#include <string>

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

} // namespace arm