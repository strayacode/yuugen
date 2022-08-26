#pragma once

#include "Common/Types.h"

struct ExecutorInterface {
    virtual ~ExecutorInterface() = default;

    // runs the executor until target,
    // and returns the number of cycles executed
    virtual u64 run(u64 target) = 0;
};
