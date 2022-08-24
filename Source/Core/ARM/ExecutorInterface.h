#pragma once

#include "Common/Types.h"

struct ExecutorInterface {
    virtual ~ExecutorInterface() = default;
    virtual void run(u64 target) = 0;
};
