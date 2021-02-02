#pragma once

#include <common/types.h>

class GBA_GPU {
public:
	const u32* get_framebuffer(int screen);
private:
	u32 framebuffer[240 * 160] = {};
};