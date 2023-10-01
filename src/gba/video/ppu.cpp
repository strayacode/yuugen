#include "gba/video/ppu.h"
#include "gba/system.h"

namespace gba {

PPU::PPU(System& system) : scheduler(system.scheduler), irq(system.irq) {}

void PPU::reset() {
    vram.fill(0);
    dispcnt.data = 0;
    dispstat.data = 0;
    vcount = 0;
    
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

void PPU::write_dispcnt(u16 value, u32 mask) {
    dispcnt.data = (dispcnt.data & ~mask) | (value & mask);
}

void PPU::render_scanline_start() {
    if (vcount < 160) {
        render_scanline(vcount);

        // TODO: trigger hblank dma
    }

    dispstat.hblank = true;

    if (dispstat.hblank_irq) {
        irq.raise(IRQ::Source::HBlank);
    }
}

void PPU::render_scanline_end() {
    if (++vcount == 229) {
        vcount = 0;
    }

    dispstat.hblank = false;
    
    switch (vcount) {
    case 160:
        dispstat.vblank = true;
        
        if (dispstat.vblank_irq) {
            irq.raise(IRQ::Source::VBlank);
        }

        // TODO: trigger vblank dma
        break;
    case 228:
        dispstat.vblank = false;
        break;
    }

    if (dispstat.lyc_setting == vcount) {
        dispstat.lyc = true;

        if (dispstat.lyc_irq) {
            irq.raise(IRQ::Source::VCounter);
        }
    } else {
        dispstat.lyc = false;
    }
}

void PPU::render_scanline(int line) {

}

} // namespace gba