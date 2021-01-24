#include <nds/nds.h>
#include <nds/interrupt.h>

Interrupt::Interrupt(NDS *nds) : nds(nds) {

}

void Interrupt::write_ime(u32 value) {
	log_debug("value: %d", value);
	IME = value & 0x1;
	if (IME != 0) {
		log_debug("interrupt time");
	}
}

void Interrupt::write_ie9(u32 value) {
	// set the appropriate bits
	IE = value & 0x3F3F7F;
}

void Interrupt::write_ie7(u32 value) {
	// set the appropriate bits
	IE = value & 0x1DF3FFF;
}