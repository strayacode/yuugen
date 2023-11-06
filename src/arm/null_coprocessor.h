#pragma once

#include "common/types.h"
#include "arm/coprocessor.h"

namespace arm {

class NullCoprocessor : public Coprocessor {
public:
    void reset() override {}

    u32 read(u32 /* cn */, u32 /* cm */, u32 /* cp */) override {
        return 0;
    }

    void write(u32 /* cn */, u32 /* cm */, u32 /* cp */, u32 /* value */) override {}

    u32 get_exception_base() override {
        return 0;
    }

    bool has_side_effects(u32 /* cn */, u32 /* cm */, u32 /* cp */) override {
        return false;
    }
};

} // namespace arm