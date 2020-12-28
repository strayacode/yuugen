#pragma once
#include <emulator/common/types.h>

class Emulator;

class GPU2D {
public:
    u32 dispcnt;
    u8 vramcnt_a;
    GPU2D(Emulator *emulator, int engine_id);
    const u32* get_framebuffer();
    u32 convert_15_to_24(u32 colour);
    // draws a scanline for either engine a or b
    // also use int line arg so we dont need to increment an extra variable
    void render_scanline(int line);

    void render_blank_screen(int line);
    void render_vram_display(int line);
    u32 framebuffer[256 * 192] = {};
private:
    Emulator *emulator;
    // 1 = engine a, 0 = engine b
    int engine_id;
    
};