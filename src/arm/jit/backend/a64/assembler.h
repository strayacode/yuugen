#pragma once

#include <vector>
#include "common/types.h"
#include "arm/jit/backend/a64/register.h"

namespace arm {

class A64Assembler {
public:
    void ret();

private:
    void emit(u32 data);

    std::vector<u32> code;
};

} // namespace