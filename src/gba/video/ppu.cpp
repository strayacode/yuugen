#include "gba/video/ppu.h"
#include "gba/system.h"

namespace gba {

PPU::PPU(System& system) : scheduler(system.scheduler), irq(system.irq) {}

void PPU::reset() {
    vram.fill(0);
    oam.fill(0);
    dispcnt.data = 0;
    dispstat.data = 0;
    vcount = 0;
    framebuffer.fill(0);
    
    scanline_start_event = scheduler.register_event("Scanline Start", [this]() {
        render_scanline_start();
        scheduler.add_event(228, &scanline_end_event);
    });

    scanline_end_event = scheduler.register_event("Scanline End", [this]() {
        render_scanline_end();
        scheduler.add_event(1004, &scanline_start_event);
    });

    scheduler.add_event(1004, &scanline_start_event);
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
    switch (dispcnt.bg_mode) {
    case 0:
        logger.warn("mode 0");
        break;
    case 4:
        render_mode4(line);
        break;
    default:
        logger.todo("PPU: handle bg mode %d", dispcnt.bg_mode);
    }
}

void PPU::render_mode4(int line) {
    auto bitmap_start = dispcnt.display_frame_select * 0xa000;

    for (int x = 0; x < 240; x++) {
        int offset = bitmap_start + (240 * line) + x;
        int palette_index = vram[offset];
        u16 colour = common::read<u16>(palette_ram.data(), palette_index * 2);
        plot(x, line, 0xff000000 | rgb555_to_rgb888(colour));
    }
}

u32 PPU::rgb555_to_rgb888(u32 colour) {
    u8 r = ((colour & 0x1f) * 255) / 31;
    u8 g = (((colour >> 5) & 0x1f) * 255) / 31;
    u8 b = (((colour >> 10) & 0x1f) * 255) / 31;
    return (b << 16) | (g << 8) | r;
}

void PPU::plot(int x, int y, u32 colour) {
    framebuffer[(240 * y) + x] = colour;
}

} // namespace gba