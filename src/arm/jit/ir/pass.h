#pragma once

#include "arm/jit/basic_block.h"

namespace arm {

class Pass {
public:
    virtual ~Pass() = default;
    virtual void optimise(BasicBlock& basic_block) = 0;

    bool modified_basic_block() const { return modified; }
    void clear_modified() { modified = false; }
    void mark_modified() { modified = true; }

private:
    bool modified{false};
};

} // namespace arm