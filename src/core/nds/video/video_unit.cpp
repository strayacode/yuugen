#include "core/nds/video/video_unit.h"

namespace core::nds {

void VideoUnit::reset() {
    powcnt1.data = 0;

    vram.reset();
    ppu_a.reset();
    ppu_b.reset();
}

void VideoUnit::write_powcnt1(u16 value) {
    powcnt1.data = value & 0x820f;
}

} // namespace core::nds