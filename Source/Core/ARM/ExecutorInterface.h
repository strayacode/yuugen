#pragma once

#include "Common/Types.h"

struct ExecutorInterface {
    virtual void execute(u64 target) = 0;
};
