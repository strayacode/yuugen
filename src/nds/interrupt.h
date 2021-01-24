#pragma once

#include <common/types.h>
#include <common/log.h>

class NDS;

class Interrupt {
public:
	Interrupt(NDS *nds);

	void write_ime(u32 value);
	void write_ie9(u32 value);
	void write_ie7(u32 value);

	// interrupt request flags
	u32 IF;

	// interrupt master enable
	// this determines if any interrupts at all can occur
	u32 IME;

private:
	NDS *nds;

	

	// interrupt enable
	// this will allow interrupts to occur but only if enabled in IE
	u32 IE;

	
	
};