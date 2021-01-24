#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>

class GPU;

class GPU2D {
public:
	GPU2D(GPU *gpu, int engine_id);

	u32 DISPCNT;

	const u32* get_framebuffer();

	void render_scanline(int line);
private:
	GPU *gpu;

	// 0 = engine A, 1 = engine B
	int engine_id;


	u32 framebuffer[256 * 192] = {};

	// primarily for use with vram display mode 2 graphics by getting the vram bank from dispcnt 18..19
    u8 get_vram_bank();

    void render_blank_screen(int line);
    void render_vram_display(int line);

    u32 convert_15_to_24(u32 colour);
};