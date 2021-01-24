#include <nds/gpu.h>
#include <nds/gpu_2d.h>

GPU2D::GPU2D(GPU *gpu, int engine_id) : gpu(gpu), engine_id(engine_id) {
	
}

const u32* GPU2D::get_framebuffer() {
	return &framebuffer[0];
}

u8 GPU2D::get_vram_bank() {
	return (DISPCNT >> 18) & 0x3;
}

void GPU2D::render_scanline(int line) {
	// get the display mode (bits 16..17)
	u8 display_mode = (DISPCNT >> 16) & 0x3;
	switch (display_mode) {
	case 0:
		// display off (screen becomes white)
		render_blank_screen(line);
		break;
	case 2:
		// render using vram display mode
		render_vram_display(line);
		break;
	default:
        log_fatal("2d display mode %d is not implemented yet!", display_mode);
	}
}

void GPU2D::render_blank_screen(int line) {
	memset(&framebuffer[line * 256], 0xFF, 256 * sizeof(u32));
}

// TODO: optimise later by using 2d array for the vram banks ig?
void GPU2D::render_vram_display(int line) {
    for (int i = 0; i < 256; i++) {
        u16 data;
        switch (get_vram_bank()) {
        case 0:
            // log_debug("vram a");
            data = (gpu->vram_a[(256 * line * 2) + (i * 2) + 1] << 8 | gpu->vram_a[(256 * line * 2) + (i * 2)]);
            break;
        case 1:
            // log_debug("vram b");
            data = (gpu->vram_b[(256 * line * 2) + (i * 2) + 1] << 8 | gpu->vram_b[(256 * line * 2) + (i * 2)]);
            break;
        case 2:
            data = (gpu->vram_c[(256 * line * 2) + (i * 2) + 1] << 8 | gpu->vram_c[(256 * line * 2) + (i * 2)]);
            break;
        case 3:
            data = (gpu->vram_d[(256 * line * 2) + (i * 2) + 1] << 8 | gpu->vram_d[(256 * line * 2) + (i * 2)]);
            break;
        }
        framebuffer[(256 * line) + i] = convert_15_to_24(data);
    }
}

// converts a 15 bit rgb colour to a 24 bit rgb colour
u32 GPU2D::convert_15_to_24(u32 colour) {
    u8 b = ((colour & 0x1F) * 255) / 31;
    u8 g = (((colour >> 5) & 0x1F) * 255) / 31;
    u8 r = (((colour >> 10) & 0x1F) * 255) / 31;
    return (b << 16) | (g << 8) | r;
}