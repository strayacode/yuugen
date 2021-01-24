#pragma once

#include <common/types.h>
#include <stdio.h>
#include <string.h>

class NDS;

class Memory {
public:
	Memory(NDS *nds);


	u8 arm7_read_byte(u32 addr);
	u16 arm7_read_halfword(u32 addr);
	u32 arm7_read_word(u32 addr);

	void arm7_write_byte(u32 addr, u8 data);
	void arm7_write_halfword(u32 addr, u16 data);
	void arm7_write_word(u32 addr, u32 data);

	u8 arm9_read_byte(u32 addr);
	u16 arm9_read_halfword(u32 addr);
	u32 arm9_read_word(u32 addr);

	void arm9_write_byte(u32 addr, u8 data);
	void arm9_write_halfword(u32 addr, u16 data);
	void arm9_write_word(u32 addr, u32 data);

	void load_arm9_bios();
	void load_arm7_bios();

	void reset();

	// io registers
    u8 POSTFLG7, POSTFLG9;

    // bits 0..1 are used
    u8 WRAMCNT;

private:
	NDS *nds;

	u8 main_memory[0x400000] = {};

	// the way this is read/written depends on wramcnt
	u8 shared_wram[0x8000] = {};

	// arm9 specific
	u8 arm9_bios[0x8000] = {}; // 32kb arm9 bios only 3kb used

	// arm7 specific
    u8 arm7_bios[0x4000] = {}; // 16kb

    

    // for arm7
    u8 arm7_wram[0x10000] = {};

    


    // bits 0..6 can be changed by both cpus
    // bits 7..15 can be changed by arm9 only
    u16 EXMEMCNT;
};