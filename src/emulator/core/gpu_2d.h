#pragma once
#include <emulator/common/types.h>

class GPU2D {
public:
    u32 dispcnt;
    u8 vramcnt_a;
    // GPU2D(GPU *gpu);
    const u32* get_framebuffer();
    u32 convert_15_to_24(u32 colour);
    // draws a scanline for either engine a or b
    // also use int line arg so we dont need to increment an extra variable
    void draw_scanline(int line);
    u32 framebuffer[256 * 192] = {};
private:
    // GPU *gpu;
    
};