#include "core/video/video_unit.h"

namespace core {

VideoUnit::VideoUnit() : ppu_a(*this, Engine::A), ppu_b(*this, Engine::B) {}

void VideoUnit::reset() {
    powcnt1.data = 0;

    vram.reset();
    ppu_a.reset();
    ppu_b.reset();
}

void VideoUnit::write_powcnt1(u16 value) {
    powcnt1.data = value & 0x820f;
}

const u32* VideoUnit::get_framebuffer(Screen screen) {
    if (powcnt1.display_swap == (screen == Screen::Top)) {
        return ppu_a.get_framebuffer();
    } else {
        return ppu_b.get_framebuffer();
    }
}

} // namespace core