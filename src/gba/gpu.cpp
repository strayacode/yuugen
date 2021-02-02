#include <gba/gpu.h>

const u32* GBA_GPU::get_framebuffer(int screen) {
	return &framebuffer[0];
}