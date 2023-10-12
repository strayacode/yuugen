#include "gba/hardware/apu.h"

namespace gba {

void APU::reset() {
    soundbias.data = 0x200;
}

void APU::write_soundbias(u16 value, u32 mask) {
    soundbias.data = (soundbias.data & ~mask) | (value & mask);
}

} // namespace gba