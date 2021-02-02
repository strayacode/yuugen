#pragma once

#include <gba/arm.h>
#include <gba/input.h>
#include <gba/gpu.h>
#include <gba/cartridge.h>
#include <common/emu_core.h>
#include <string>

class GBA : virtual public EmuCore {
public:
	void direct_boot(std::string rom_path);
	void run_frame();
	void reset();

	const u32* get_framebuffer(int screen);

	void handle_input();
private:
	GBA_ARM arm7;
	GBA_GPU gpu;
	GBA_Cartridge cartridge;
	GBA_Input input;
};