#include <nds/nds.h>
#include <nds/interrupt.h>

Interrupt::Interrupt(NDS *nds, bool cpu_id) : nds(nds), cpu_id(cpu_id) {

}

void Interrupt::write_ime(u32 value) {
	IME = value & 0x1;
}

void Interrupt::write_ie9(u32 value) {
	// set the appropriate bits
	IE = (IE & ~0x3F3F7F) | (value & 0x3F3F7F);
}

void Interrupt::write_ie7(u32 value) {
	// set the appropriate bits
	IE = (IE & ~0x1DF3FFF) | (value & 0x1DF3FFF);
}

void Interrupt::write_if(u32 value) {
	// when writing to IF a value of 1 actually resets the bit to acknowledge the interrupt while writing 0 has no change
	IF &= ~(value);
}

void Interrupt::request_interrupt(u8 bit) {
	IF |= (1 << bit);
}