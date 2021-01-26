#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>

class GPU;

class GPU2D {
public:
	GPU2D(GPU *gpu, int engine_id);

	u32 DISPCNT = 0;

	u16 BGCNT[4] = {};

	// BG x-offset
	u16 BGHOFS[4] = {};

	// BG y-offset
	u16 BGVOFS[4] = {};

	// bg layer 2 parameters for rotation and scaling
	u16 BG2PA = 0;
	u16 BG2PB = 0;
	u16 BG2PC = 0;
	u16 BG2PD = 0;

	// bg layer 2 parameters for rotation and scaling
	u16 BG3PA = 0;
	u16 BG3PB = 0;
	u16 BG3PC = 0;
	u16 BG3PD = 0;

	// bg2 reference points
	u32 BG2X = 0;
	u32 BG2Y = 0;

	// bg3 reference points
	u32 BG3X = 0;
	u32 BG3Y = 0;

	// window horizontal and vertical dimensions
	u16 WIN0H = 0;
	u16 WIN1H = 0;

	u16 WIN0V = 0;
	u16 WIN1V = 0;

	// control for inside and outside windows
	u16 WININ = 0;
	u16 WINOUT = 0;

	// mosaic size
	u16 MOSAIC = 0;

	// special effects registers
	u16 BLDCNT = 0;
	u16 BLDALPHA = 0;



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