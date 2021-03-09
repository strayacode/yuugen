#include <core/core.h>
#include <core/spu.h>

SPU::SPU(Core* core) : core(core) {

}

void SPU::Reset() {
    SOUNDCNT = 0;
}