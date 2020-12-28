#pragma once
#include <emulator/common/types.h>
#include <emulator/core/gpu_2d.h>

class Emulator;

class GPU {
public:
    u16 powcnt1;
    GPU(Emulator *emulator);
    GPU2D engine_a, engine_b;
    // TODO: replace this
    void fill_framebuffer();

    // this handles drawing the scanline for 2d engine a and b and eventually the 3d engine
    // also use int line argument so we dont have to increment an extra variable
    void render_scanline(int line);
private:
    Emulator *emulator;

    u16 vcount;

    
};