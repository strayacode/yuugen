#pragma once

#include "common/types.h"

namespace gba {

class APU {
public:
    void reset();

    u16 read_soundbias() { return soundbias.data; }
    void write_soundbias(u16 value, u32 mask);

private:
    union SOUNDBIAS {
        struct {
            u16 : 1;
            u16 bias_level : 9;
            u16 : 4;
            u16 amplitude_resolution : 2;
        };

        u16 data;
    };

    SOUNDBIAS soundbias;
};

} // namespace gba