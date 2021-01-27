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

	// check if other cpu allows for irq to be sent for fifo and if the current cpu will send an irq
	if (get_bit(13, IPCSYNC9) && get_bit(14, IPCSYNC7)) {
		nds->interrupt[1].request_interrupt(16);
	}

	// now store data output specified by bits 8..11 in ipcsync of other cpu for bits 0..3
	IPCSYNC7 = (IPCSYNC7 & ~0xF) | ((data >> 8) & 0xF);
}

void IPC::write_ipcfifocnt7(u16 data) {
	// check if send fifo clear bit gets set (means clear send queue)
	if (get_bit(3, data)) {
		memset(send_queue, 0, 16);
	}

	// write to ipcfifocnt
	// TODO: only write to used bits
	IPCFIFOCNT7 = data;
}

void IPC::write_ipcfifocnt9(u16 data) {
	// check if send fifo clear bit gets set (means clear send queue)
	if (get_bit(3, data)) {
		memset(send_queue, 0, 16);
	}

	// write to ipcfifocnt
	// TODO: only write to used bits
	IPCFIFOCNT9 = data;
}

u16 IPC::read_ipcsync7() {
	return IPCSYNC7 & 0x4F0F;
}

u16 IPC::read_ipcsync9() {
	return IPCSYNC9 & 0x4F0F;
}