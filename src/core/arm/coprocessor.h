#pragma once

#include "common/types.h"

namespace core::arm {

struct Coprocessor {
    virtual ~Coprocessor() = default;

    virtual u32 read(u32 cn, u32 cm, u32 cp) = 0;
    virtual void write(u32 cn, u32 cm, u32 cp, u32 value) = 0;
    virtual u32 get_exception_base() = 0;
};

} // namespace core::arm