#include "gba/video/ppu.h"

namespace gba {

PPU::PPU(common::Scheduler& scheduler) : scheduler(scheduler) {}

void PPU::reset() {
    scanline_start_event = scheduler.register_event("Scanline Start", [this]() {
        render_scanline_start();
        scheduler.add_event(272, &scanline_end_event);
    });

    scanline_end_event = scheduler.register_event("Scanline End", [this]() {
        render_scanline_end();
        scheduler.add_event(960, &scanline_start_event);
    });

    scheduler.add_event(960, &scanline_start_event);
}

void PPU::render_scanline_start() {
    // logger.warn("render scanline start %d", scheduler.get_current_time());
}

void PPU::render_scanline_end() {
    // logger.warn("render scanline end %d", scheduler.get_current_time());
}

} // namespace gba