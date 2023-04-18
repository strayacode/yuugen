#pragma once

#include "arm/memory.h"
#include "arm/coprocessor.h"

namespace core::arm {

enum class Arch {
  ARMv4,
  ARMv5,
};

struct Config {
    Memory& memory;
    Coprocessor& coprocessor;

    Arch arch;

    // TODO: make software fastmem an option
};

} // namespace core::arm