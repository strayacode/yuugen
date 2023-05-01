#include "core/nds/video/ppu.h"

namespace core::nds {

void PPU::reset() {
    dispcnt.data = 0;
}

void PPU::write_dispcnt(u32 value, u32 mask) {
    dispcnt.data = (dispcnt.data & ~mask) | (value & mask);
}

} // namespace core::nds