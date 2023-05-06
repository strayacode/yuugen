#pragma once

#include "common/types.h"

namespace core {

class Input {
public:
    void reset();
    u16 read_extkeyin();

private:
    u16 extkeyin;
};

} // namespace core