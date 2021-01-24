#pragma once
#include <common/types.h>


// seems to be only accessible by arm7?

class NDS;

class SPU {
public:
	SPU(NDS *nds);

	u16 get_soundbias();
private:
	NDS *nds;

	u16 SOUNDBIAS;


};