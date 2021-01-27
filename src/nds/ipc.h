#pragma once
#include <common/types.h>
#include <common/arithmetic.h>
#include <common/log.h>
#include <string.h>



class NDS;

// the IPC (inter process communication) allows the arm9 and arm7 to exchange information

class IPC {
public:
	IPC(NDS *nds);

	void reset();	


	void write_ipcsync7(u16 data);
	void write_ipcsync9(u16 data);

	u16 read_ipcsync7();
	u16 read_ipcsync9();

	void write_ipcfifocnt7(u16 data);
	void write_ipcfifocnt9(u16 data);

	u16 IPCFIFOCNT7, IPCFIFOCNT9;
private:
	NDS *nds;


	// each cpu has an ipcsync register
	// when one cpu writes to its own register with data in bits 8..11, that data is recieved in bits 0..3 of the other cpus ipcsync
	// also only 4 bits can be exchanged
	u16 IPCSYNC7, IPCSYNC9;

	

	u32 send_queue[16] = {};
	u32 recieve_queue[16] = {};

};