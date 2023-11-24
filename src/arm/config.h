#pragma once

#include "arm/cpu.h"

namespace arm {

struct Config {
    int block_size{1};
    BackendType backend_type{BackendType::Interpreter};
    bool optimisations{false};
};

} // namespace arm