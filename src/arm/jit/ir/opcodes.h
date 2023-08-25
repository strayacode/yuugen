#pragma once

#include <string>

namespace arm {

struct IROpcode {
    virtual ~IROpcode() = default;
    virtual std::string to_string() = 0;
};

} // namespace arm