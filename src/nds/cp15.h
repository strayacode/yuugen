#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/arithmetic.h>
#include <string.h>

class NDS;

class CP15 {
public:
	CP15(NDS *nds);

	u32 read_reg(u32 cn, u32 cm, u32 cp);
	void write_reg(u32 cn, u32 cm, u32 cp, u32 data);

	bool get_dtcm_enabled();
	bool get_itcm_enabled();

	u32 get_dtcm_base_addr();

	u32 get_dtcm_size();
	u32 get_itcm_size();

	void direct_boot();

	// dtcm and itcm mem arrays
	u8 itcm[0x8000] = {};
	u8 dtcm[0x4000] = {};

	// this is used to get bit 13 of the control register to check if the base of exceptions should be 0x00000000 or 0xFFFF0000
	u32 get_exception_base();



private:
	NDS *nds;

	u32 control_register;

	// arm9 specific since only arm has a coprocessor
    // still dont really know the main uses of these but just here because of cp15 write command with cache control
    u8 data_cache[0x1000] = {};
    u8 instruction_cache[0x2000] = {};

    // raw registers
    u32 dtcm_reg;
    u32 itcm_reg;

    u32 dtcm_base_addr = 0;
    // this is always set to 0 as itcm starts at 0
    u32 itcm_base_addr = 0;

    u32 dtcm_size = 0;
    u32 itcm_size = 0;

};