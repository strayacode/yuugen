#pragma once

#include <common/types.h>
#include <common/mem_helpers.h>
#include <common/arithmetic.h>
#include <nds/gpu_2d.h>

class NDS;

class GPU {
public:
	GPU(NDS *nds);


	const u32* get_framebuffer(int screen);

	u32 POWCNT1;

	GPU2D engine_a, engine_b;

	u8 vramcnt_a;
    u8 vramcnt_b;
    u8 vramcnt_c;
    u8 vramcnt_d;
    u8 vramcnt_e;
    u8 vramcnt_f;
    u8 vramcnt_g;
    u8 vramcnt_h;
    u8 vramcnt_i;

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

    void write_lcdc_vram(u32 addr, u16 data);

    // render scanline begin is executed when 256 * 3 have passed in a scanline (when hblank starts)
    // it renders the scanline to the relative framebuffer
    void render_scanline_begin(int line);

    // render scanline end is called when at the very end of a scanline (when 355 * 3 cycles have passed)
    // this does stuff mainly related to dispstat to reflect changes in stuff
    void render_scanline_end(int line);

    enum screen {
		TOP_SCREEN,
		BOTTOM_SCREEN,
	};

	u16 DISPSTAT9, DISPSTAT7 = 0;

	// indicates the current scanline
	u16 VCOUNT = 0;

	void write_dispstat9(u16 data);
    void write_dispstat7(u16 data);

private:
	

	NDS *nds;

	bool get_vram_bank_enabled(u8 vramcnt);
    bool get_vram_bank_mst(u8 vramcnt);

	
};