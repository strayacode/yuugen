#include <gba/arm.h>

void GBA_ARM::direct_boot() {
	regs.r[0] = 0x08000000;
	regs.r[1] = 0xEA;

	regs.r[13] = 0x03007F00;

	
}