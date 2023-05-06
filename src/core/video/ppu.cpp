#include "core/video/video_unit.h"
#include "core/video/ppu.h"

namespace core {

PPU::PPU(VideoUnit& video_unit, Engine engine) : video_unit(video_unit), engine(engine) {}

void PPU::reset() {
    dispcnt.data = 0;
    framebuffer.fill(0xff00ff00);
}

void PPU::render_scanline(int line) {

}

void PPU::write_dispcnt(u32 value, u32 mask) {
    dispcnt.data = (dispcnt.data & ~mask) | (value & mask);
}

} // namespace core