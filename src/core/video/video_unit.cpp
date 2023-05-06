#include "common/logger.h"
#include "core/video/video_unit.h"
#include "core/system.h"

namespace core {

VideoUnit::VideoUnit(System& system) : ppu_a(*this, Engine::A), ppu_b(*this, Engine::B), system(system) {}

void VideoUnit::reset() {
    powcnt1.data = 0;
    dispstat7.data = 0;
    dispstat9.data = 0;
    vcount = 0;

    vram.reset();
    ppu_a.reset();
    ppu_b.reset();

    scanline_start_event = system.scheduler.register_event("Scanline Start", [this]() {
        render_scanline_start();
        system.scheduler.add_event(524, &scanline_end_event);
    });

    scanline_end_event = system.scheduler.register_event("Scanline End", [this]() {
        render_scanline_end();
        system.scheduler.add_event(1606, &scanline_start_event);
    });

    system.scheduler.add_event(1606, &scanline_start_event);
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

void VideoUnit::render_scanline_start() {
    if (vcount < 192) {
        ppu_a.render_scanline(vcount);
        ppu_b.render_scanline(vcount);

        // TODO
        // system.dma[1].Trigger(2);
    }

    dispstat7.hblank = true;
    dispstat9.hblank = true;

    // TODO: handle hblank irqs and dmas

    // TODO
    // if (vcount == 215) {
    //     renderer_3d.render();
    // }

    // TODO
    // if ((vcount > 1) && (vcount < 194)) {
    //     system.dma[1].Trigger(3);
    // }
}

void VideoUnit::render_scanline_end() {
    if (++vcount == 263) {
        vcount = 0;
    }

    dispstat7.hblank = false;
    dispstat9.hblank = false;

    switch (vcount) {
    case 192:
        dispstat7.vblank = true;
        dispstat9.vblank = true;

        // TODO: handle vblank irqs and dmas
        break;
    case 262:
        dispstat7.vblank = false;
        dispstat9.vblank = false;
        break;
    }

    if ((dispstat7.lyc_setting | (dispstat7.lyc_setting_msb << 1)) == vcount) {
        dispstat7.lyc = true;

        // TODO: handle vcounter irqs
    } else {
        dispstat7.lyc = false;
    }

    if ((dispstat9.lyc_setting | (dispstat9.lyc_setting_msb << 1)) == vcount) {
        dispstat9.lyc = true;

        // TODO: handle vcounter irqs
    } else {
        dispstat9.lyc = false;
    }
}

} // namespace core