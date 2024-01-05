#include "common/logger.h"
#include "nds/video/video_unit.h"
#include "nds/system.h"

namespace nds {

VideoUnit::VideoUnit(System& system) :
    gpu(system.scheduler, system.dma9, system.arm9.get_irq(), vram.texture_data, vram.texture_palette),
    ppu_a(gpu, get_palette_ram(), get_oam(), vram.bga, vram.obja, vram.bga_extended_palette, vram.obja_extended_palette, vram.lcdc),
    ppu_b(gpu, get_palette_ram() + 0x400, get_oam() + 0x400, vram.bgb, vram.objb, vram.bgb_extended_palette, vram.objb_extended_palette, vram.lcdc),
    system(system),
    irq7(system.arm7.get_irq()),
    irq9(system.arm9.get_irq()) {}

void VideoUnit::reset() {
    palette_ram.fill(0);
    oam.fill(0);
    powcnt1.data = 0;
    dispstat7.data = 0;
    dispstat9.data = 0;
    dispcapcnt.data = 0;
    vcount = 0;
    display_capture = false;

    vram.reset();
    gpu.reset();
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

u16 VideoUnit::read_dispstat(arm::Arch arch) {
    if (arch == arm::Arch::ARMv5) {
        return dispstat9.data;
    } else {
        return dispstat7.data;
    }
}

void VideoUnit::write_dispstat(arm::Arch arch, u16 value, u32 mask) {
    mask &= 0xffbf;
    if (arch == arm::Arch::ARMv5) {
        dispstat9.data = (dispstat9.data & ~mask) | (value & mask);
    } else {
        dispstat7.data = (dispstat7.data & ~mask) | (value & mask);
    }
}

void VideoUnit::write_vcount(u16 value, u32 mask) {
    vcount = (vcount & ~mask) | (value & mask);
}

void VideoUnit::write_powcnt1(u16 value, u32 mask) {
    mask &= 0x820f;
    powcnt1.data = (powcnt1.data & ~mask) | (value & mask);
}

void VideoUnit::write_dispcapcnt(u32 value, u32 mask) {
    dispcapcnt.data = (dispcapcnt.data & ~mask) | (value & mask);
}

u32* VideoUnit::fetch_framebuffer(Screen screen) {
    if (powcnt1.display_swap == (screen == Screen::Top)) {
        return ppu_a.fetch_framebuffer();
    } else {
        return ppu_b.fetch_framebuffer();
    }
}

void VideoUnit::render_scanline_start() {
    if (vcount < 192) {
        ppu_a.render_scanline(vcount);
        ppu_b.render_scanline(vcount);
        system.dma9.trigger(DMA::Timing::HBlank);
    }

    if (vcount == 0 && dispcapcnt.capture_enable) {
        display_capture = true;
    }

    if (display_capture) {
        int width = display_capture_dimensions[dispcapcnt.capture_size][0];
        int height = display_capture_dimensions[dispcapcnt.capture_size][1];
        u32 base = 0x06800000 + (dispcapcnt.vram_write_block * 0x20000);
        u32 write_offset = (dispcapcnt.vram_write_offset * 0x8000) + vcount * width * 2;

        switch (dispcapcnt.capture_source) {
        case 0: {
            u32* line;

            // source a
            if (dispcapcnt.source_a) {
                // capture 3d
                line = gpu.fetch_framebuffer() + (vcount * 256);
            } else {
                // capture 2d
                LOG_TODO("VideoUnit: capture 2d");
            }

            for (int i = 0; i < width; i++) {
                vram.write<u16>(base + (write_offset & 0x1ffff), rgb666_to_rgb555(line[i]));
                write_offset += 2;
            }

            break;
        }
        default:
            LOG_TODO("VideoUnit: handle capture source %d", dispcapcnt.capture_source);
        }

        // TODO: see if dispcapcnt busy bit gets cleared at height or 192 scanline
        if (vcount + 1 == height) {
            dispcapcnt.capture_enable = false;
            display_capture = false;
        }
    }

    dispstat7.hblank = true;
    dispstat9.hblank = true;

    if (dispstat7.hblank_irq) {
        irq7.raise(IRQ::Source::HBlank);
    }

    if (dispstat9.hblank_irq) {
        irq9.raise(IRQ::Source::HBlank);
    }

    if (vcount == 215) {
        gpu.render();
    }

    // TODO: is this correctly implemented?
    if (vcount > 1 && vcount < 194) {
        system.dma9.trigger(DMA::Timing::StartOfDisplay);
    }
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

        if (dispstat7.vblank_irq) {
            irq7.raise(IRQ::Source::VBlank);
        }

        if (dispstat9.vblank_irq) {
            irq9.raise(IRQ::Source::VBlank);
        }

        system.dma9.trigger(DMA::Timing::VBlank);
        gpu.do_swap_buffers();
        break;
    case 262:
        dispstat7.vblank = false;
        dispstat9.vblank = false;
        break;
    }

    if ((dispstat7.lyc_setting | (dispstat7.lyc_setting_msb << 1)) == vcount) {
        dispstat7.lyc = true;

        if (dispstat7.lyc_irq) {
            irq7.raise(IRQ::Source::VCounter);
        }
    } else {
        dispstat7.lyc = false;
    }

    if ((dispstat9.lyc_setting | (dispstat9.lyc_setting_msb << 1)) == vcount) {
        dispstat9.lyc = true;

        if (dispstat9.lyc_irq) {
            irq9.raise(IRQ::Source::VCounter);
        }
    } else {
        dispstat9.lyc = false;
    }
}

u16 VideoUnit::rgb666_to_rgb555(u32 colour) {
    u8 r = (colour & 0x3f) / 2;
    u8 g = ((colour >> 6) & 0x3f) / 2;
    u8 b = ((colour >> 12) & 0x3f) / 2;
    return 0x8000 | (b << 10) | (g << 5) | r;
}

} // namespace nds