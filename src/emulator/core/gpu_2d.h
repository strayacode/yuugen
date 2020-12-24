#pragma once
#include <emulator/common/types.h>

// class GPU;

typedef union vramcnt_a_reg {
    u8 raw;
    struct {
        u8 mst: 3;
        u8 offset: 2;
        u8 : 2;
        u8 enable: 1;
    };
} vramcnt_a_t;

typedef union dispcnt_reg {
    u32 raw;
    struct {
        u8 bg_mode: 3; // bg mode for engine A+B
        u32 : 29; // ignore the rest for now lol
    };
} dispcnt_t;

class GPU2D {
public:
    dispcnt_t dispcnt;
    vramcnt_a_t vramcnt_a;
    // GPU2D(GPU *gpu);
    const u32* get_framebuffer();
    u32 convert_15_to_24(u32 colour);
    
    u32 framebuffer[256 * 192] = {};
private:
    // GPU *gpu;
    
};