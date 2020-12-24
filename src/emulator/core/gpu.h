#pragma once
#include <emulator/common/types.h>
#include <emulator/core/gpu_2d.h>

typedef union powcnt1_reg {
    u16 raw;
    struct {
        bool enable_lcd: 1; // enable flag for both lcds
        bool engine_a: 1; // 2d graphics engine a
        bool rendering_3d: 1; // 3d rendering engine
        bool geometry_3d: 1; // 3d geometry engine
        u8 : 5; // unused
        bool engine_b: 1; // 2d graphics engine b
        u8 : 5; // unused
        bool display_swap: 1; // display swap
    };
} powcnt1_t;


class Emulator;

class GPU {
public:
    powcnt1_t powcnt1;
    GPU(Emulator *emulator);
    GPU2D engine_a, engine_b;
    void fill_framebuffer();
private:
    Emulator *emulator;

    
};