#pragma once

#include <nds/cp15.h>
#include <nds/arm.h>
#include <nds/gpu.h>
#include <nds/memory.h>
#include <nds/cartridge.h>
#include <nds/interrupt.h>
#include <nds/ipc.h>
#include <nds/rtc.h>
#include <nds/timers.h>
#include <nds/spi.h>
#include <nds/spu.h>
#include <nds/dma.h>
#include <nds/input.h>
#include <nds/maths_unit.h>
#include <string>
#include <common/log.h>
#include <common/emu_core.h>


class NDS : virtual public EmuCore {
public:
	NDS();
	void firmware_boot();
	void direct_boot(std::string rom_path);
	void run_frame();
	void reset();
	void handle_input();
	const u32* get_framebuffer(int screen);
	CP15 cp15;
	Memory memory;
	NDS_Cartridge cartridge;

	// 2 sets of interrupt registers. 0 is for arm7 and 1 is for arm9
	Interrupt interrupt[2];
	IPC ipc;
	NDS_GPU gpu;
	RTC rtc;
	// 2 timers. 0 is for arm7 and 1 is for arm9
	Timers timers[2];

	// each arm cpu gets 4 dma channels
	// 0 is for arm7 and 1 is for arm9
	DMA dma[2];

	SPI spi;

	NDS_ARM arm9, arm7;

	SPU spu;
	
	NDS_Input input;

	MathsUnit maths_unit;

private:
	
	

	


};