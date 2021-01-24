#include <nds/spu.h>
#include <nds/nds.h>

SPU::SPU(NDS *nds) : nds(nds) {

}

u16 SPU::get_soundbias() {
	// only bits 0..9 are used in soundbias
	return SOUNDBIAS & 0x3FF;
}