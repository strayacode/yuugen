#pragma once

#include "common/types.h"
#include "core/nds/video/vram.h"
#include "core/nds/video/ppu.h"

namespace core::nds {

class VideoUnit {
public:
    void reset();
    void write_powcnt1(u16 value);

    VRAM vram;
    PPU ppu_a;
    PPU ppu_b;
    
private:
    union POWCNT1 {
        struct {
            bool enable_both_lcds : 1;
            bool enable_engine_a : 1;
            bool enable_rendering_engine : 1;
            bool enable_geometry_engine : 1;
            u32 : 5;
            bool enable_engine_b : 1;
            u32 : 5;
            bool display_swap : 1;
        };

        u16 data;
    };

    POWCNT1 powcnt1;
};

} // namespace core::nds