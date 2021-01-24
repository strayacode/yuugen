#include <nds/nds.h>

IPC::IPC(NDS *nds) : nds(nds) {

}

void IPC::reset() {
	IPCSYNC7 = IPCSYNC9 = 0;
}

void IPC::write_ipcsync7(u16 data) {
	// mask bits correctly
	IPCSYNC7 = (data & 0x6F00);
	if (get_bit(14, IPCSYNC7)) {
		log_fatal("handle");
	}

	// now store data output specified by bits 8..11 in ipcsync of other cpu for bits 0..3
	IPCSYNC9 = (IPCSYNC9 & ~0xF) | ((data >> 8) & 0xF);
}

void IPC::write_ipcsync9(u16 data) {
	// mask bits correctly
	IPCSYNC9 = (data & 0x6F00);

	if (get_bit(14, IPCSYNC9)) {
		log_fatal("handle");
	}

	// now store data output specified by bits 8..11 in ipcsync of other cpu for bits 0..3
	IPCSYNC7 = (IPCSYNC7 & ~0xF) | ((data >> 8) & 0xF);
}

u16 IPC::read_ipcsync7() {
	return IPCSYNC7 & 0x4F0F;
}

u16 IPC::read_ipcsync9() {
	return IPCSYNC9 & 0x4F0F;
}