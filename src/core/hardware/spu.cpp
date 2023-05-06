#include "core/hardware/spu.h"

namespace core {

void SPU::reset() {
    soundbias = 0;
}

void SPU::write_soundbias(u32 value) {
    soundbias = value & 0x3ff;
}

} // namespace core