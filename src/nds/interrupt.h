#pragma once

#include <common/types.h>
#include <common/log.h>

class NDS;

class Interrupt {
public:
	Interrupt(NDS *nds, bool cpu_id);

	void write_ime(u32 value);
	void write_ie9(u32 value);
	void write_ie7(u32 value);
	void write_if(u32 value);

	void request_interrupt(u8 bit);

	// interrupt request flags
	u32 IF;	

	// interrupt master enable
	// this determines if any interrupts at all can occur
	u32 IME;

	// interrupt enable
	// this will allow interrupts to occur but only if enabled in IE
	u32 IE;

private:
	NDS *nds;

	bool cpu_id;

	

	

	
	
};