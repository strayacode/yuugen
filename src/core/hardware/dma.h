#pragma once

#include <array>
#include "common/types.h"

namespace core {

class DMA {
public:
    void reset();

    void write_length(int index, u16 value);

private:
    struct Channel {
        u16 length;
    };

    std::array<Channel, 4> channels;
};

} // namespace core