#pragma once

#include "common/types.h"

namespace core::nds {

class Input {
public:
    void reset();
    u16 read_extkeyin();

private:
    u16 extkeyin;
};

} // namespace core::nds