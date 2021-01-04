#pragma once
#include <emulator/common/types.h>
#include <emulator/core/GPU2D.h>

class Emulator;

class GPU {
public:
    enum screen {
        bottom_screen,
        top_screen,
    };
    u8 vramcnt_a;
    u8 vramcnt_b;
    u8 vramcnt_c;
    u8 vramcnt_d;
    u8 vramcnt_e;
    u8 vramcnt_f;
    u8 vramcnt_g;
    u8 vramcnt_h;
    u8 vramcnt_i;

    // display status and interrupt control register. is applied for both engine A and B
    u16 dispstat = 0; 
    
    // 0 = bottom, 1 = top
    const u32* get_framebuffer(int screen);
    // implement these later
    // template <typename T>
    // T read_bga(u32 addr); // reads from bg vram allocated for engine a
    // template <typename T>
    // T read_bgb(u32 addr); // reads from bg vram allocated for engine b
    // template <typename T>
    // T read_obja(u32 addr); // reads from obj vram allocated for engine a
    // template <typename T>
    // T read_objb(u32 addr); // reads from obj vram allocated for engine b
    template <typename T>
    T read_lcdc(u32 addr); // reads from "lcdc" allocated vram (not allocated specifically to any 2d engine)

    template <typename T>
    void write_lcdc(u32 addr, T data); // writes to "lcdc" allocated vram (not allocated specifically to any 2d engine)

    u16 powcnt1;
    GPU(Emulator *emulator);
    GPU2D engine_a, engine_b;

    // this handles drawing the scanline for 2d engine a and b and eventually the 3d engine
    // also use int line argument so we dont have to increment an extra variable
    void render_scanline(int line);

    // implement vram banking
    u8 vram_a[0x20000] = {};
    u8 vram_b[0x20000] = {};
    u8 vram_c[0x20000] = {};
    u8 vram_d[0x20000] = {};
    u8 vram_e[0x10000] = {};
    u8 vram_f[0x4000] = {};
    u8 vram_g[0x4000] = {};
    u8 vram_h[0x8000] = {};
    u8 vram_i[0x4000] = {};
    
private:
    Emulator *emulator;
    



    u8 palette_ram[0x800] = {}; // 2kb for engine A obj/bg and engine B obj/bg
    u8 oam[0x800] = {}; // 2kb oam for both engine A and B
    u16 vcount = 0;

    bool get_vram_bank_enabled(u8 vramcnt);
    u8 get_vram_bank_offset(u8 vramcnt);
    u8 get_vram_bank_mst(u8 vramcnt);

    
};