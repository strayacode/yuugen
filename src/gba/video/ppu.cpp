#include "common/bits.h"
#include "gba/video/ppu.h"
#include "gba/system.h"

namespace gba {

PPU::PPU(System& system) : scheduler(system.scheduler), irq(system.irq), dma(system.dma) {}

void PPU::reset() {
    vram.fill(0);
    oam.fill(0);
    dispcnt.data = 0;
    dispstat.data = 0;
    bgcnt.fill(BGCNT{});
    bghofs.fill(0);
    bgvofs.fill(0);
    bgpa.fill(0);
    bgpb.fill(0);
    bgpc.fill(0);
    bgpd.fill(0);
    bgx.fill(0);
    bgy.fill(0);
    internal_x.fill(0);
    internal_y.fill(0);
    vcount = 0;
    winh.fill(0);
    winv.fill(0);
    winin = 0;
    winout = 0;
    mosaic.data = 0;
    bldcnt.data = 0;
    bldalpha.data = 0;
    bldy.data = 0;
    framebuffer.fill(0xff000000);
    
    scanline_start_event = scheduler.register_event("Scanline Start", [this]() {
        render_scanline_start();
        scheduler.add_event(228, &scanline_end_event);
    });

    scanline_end_event = scheduler.register_event("Scanline End", [this]() {
        render_scanline_end();
        scheduler.add_event(1004, &scanline_start_event);
    });

    scheduler.add_event(1004, &scanline_start_event);

    reset_layers();
}

void PPU::write_dispcnt(u16 value, u32 mask) {
    dispcnt.data = (dispcnt.data & ~mask) | (value & mask);
}

void PPU::write_dispstat(u16 value, u32 mask) {
    mask &= 0xffbf;
    dispstat.data = (dispstat.data & ~mask) | (value & mask);
}

void PPU::write_bgcnt(int id, u16 value, u32 mask) {
    bgcnt[id].data = (bgcnt[id].data & ~mask) | (value & mask);
}

void PPU::write_bghofs(int id, u16 value, u32 mask) {
    bghofs[id] = (bghofs[id] & ~mask) | (value & mask);
}

void PPU::write_bgvofs(int id, u16 value, u32 mask) {
    bgvofs[id] = (bgvofs[id] & ~mask) | (value & mask);
}

void PPU::write_bgpa(int id, u16 value, u32 mask) {
    bgpa[id] = (bgpa[id] & ~mask) | (value & mask);
}

void PPU::write_bgpb(int id, u16 value, u32 mask) {
    bgpb[id] = (bgpb[id] & ~mask) | (value & mask);
}

void PPU::write_bgpc(int id, u16 value, u32 mask) {
    bgpc[id] = (bgpc[id] & ~mask) | (value & mask);
}

void PPU::write_bgpd(int id, u16 value, u32 mask) {
    bgpd[id] = (bgpd[id] & ~mask) | (value & mask);
}

void PPU::write_bgx(int id, u32 value, u32 mask) {
    mask &= 0xfffffff;
    bgx[id] = common::sign_extend<u32, 28>((bgx[id] & ~mask) | (value & mask));
    internal_x[id] = bgx[id];
}

void PPU::write_bgy(int id, u32 value, u32 mask) {
    mask &= 0xfffffff;
    bgy[id] = common::sign_extend<u32, 28>((bgy[id] & ~mask) | (value & mask));
    internal_y[id] = bgy[id];
}

void PPU::write_winh(int id, u16 value, u32 mask) {
    winh[id] = (winh[id] & ~mask) | (value & mask);
}

void PPU::write_winv(int id, u16 value, u32 mask) {
    winv[id] = (winv[id] & ~mask) | (value & mask);
}

void PPU::write_winin(u16 value, u32 mask) {
    winin = (winin & ~mask) | (value & mask);
}

void PPU::write_winout(u16 value, u32 mask) {
    winout = (winout & ~mask) | (value & mask);
}

void PPU::write_mosaic(u16 value, u32 mask) {
    mosaic.data = (mosaic.data & ~mask) | (value & mask);
}

void PPU::write_bldcnt(u16 value, u32 mask) {
    bldcnt.data = (bldcnt.data & ~mask) | (value & mask);
}

void PPU::write_bldalpha(u16 value, u32 mask) {
    bldalpha.data = (bldalpha.data & ~mask) | (value & mask);
}

void PPU::write_bldy(u16 value, u32 mask) {
    bldy.data = (bldy.data & ~mask) | (value & mask);
}

void PPU::render_scanline_start() {
    if (vcount < 160) {
        render_scanline(vcount);
        dma.trigger(DMA::Timing::HBlank);
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

        dma.trigger(DMA::Timing::VBlank);
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
    reset_layers();

    if (line == 0) {
        internal_x[0] = bgx[0];
        internal_y[0] = bgy[0];
        internal_x[1] = bgx[1];
        internal_y[1] = bgy[1];
    }

    switch (dispcnt.bg_mode) {
    case 0:
        if (dispcnt.enable_bg0) {
            render_background(0, line);
        }

        if (dispcnt.enable_bg1) {
            render_background(1, line);
        }

        if (dispcnt.enable_bg2) {
            render_background(2, line);
        }

        if (dispcnt.enable_bg3) {
            render_background(3, line);
        }

        break;
    case 1:
        if (dispcnt.enable_bg0) {
            render_background(0, line);
        }

        if (dispcnt.enable_bg1) {
            render_background(1, line);
        }

        if (dispcnt.enable_bg2) {
            render_affine(2);
        }

        break;
    case 2:
        if (dispcnt.enable_bg0) {
            render_background(0, line);
        }

        if (dispcnt.enable_bg1) {
            render_background(1, line);
        }

        if (dispcnt.enable_bg2) {
            render_affine(2);
        }

        if (dispcnt.enable_bg3) {
            render_affine(3);
        }
        
        break;
    case 3:
        if (dispcnt.enable_bg2) {
            render_mode3(2, line);
        }

        break;
    case 4:
        if (dispcnt.enable_bg2) {
            render_mode4(2, line);
        }

        break;
    case 5:
        if (dispcnt.enable_bg2) {
            render_mode5(2, line);
        }

        break;
    }

    if (dispcnt.enable_obj) {
        render_objects(line);
    }

    compose_scanline(line);

    // update internal affine registers
    for (int i = 0; i < 2; i++) {
        internal_x[i] += bgpb[i];
        internal_y[i] += bgpd[i];
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

void PPU::reset_layers() {
    for (int i = 0; i < 4; i++) {
        bg_layers[i].fill(0);
    }

    for (int i = 0; i < 256; i++) {
        obj_buffer[i].priority = 4;
        obj_buffer[i].colour = colour_transparent;
    }
}

u8 PPU::calculate_enabled_layers(int x, int line) {
    u8 enabled = common::get_field<8, 5>(dispcnt.data);
    u8 window = common::get_field<13, 3>(dispcnt.data);

    if (window) {
        u8 win0_x1 = winh[0] >> 8;
        u8 win0_x2 = winh[0] & 0xff;
        u8 win0_y1 = winv[0] >> 8;
        u8 win0_y2 = winv[0] & 0xff;
        u8 win1_x1 = winh[1] >> 8;
        u8 win1_x2 = winh[1] & 0xff;
        u8 win1_y1 = winv[1] >> 8;
        u8 win1_y2 = winv[1] & 0xff;

        if (dispcnt.enable_win0 && in_window_bounds(x, win0_x1, win0_x2) && in_window_bounds(line, win0_y1, win0_y2)) {
            enabled &= winin & 0xf;
        } else if (dispcnt.enable_win1 && in_window_bounds(x, win1_x1, win1_x2) && in_window_bounds(line, win1_y1, win1_y2)) {
            enabled &= (winin >> 8) & 0xf;
        } else if (dispcnt.enable_objwin) {
            // LOG_WARN("PPU: handle object window");
        } else {
            enabled &= winout & 0xf;  
        }
    }

    return enabled;
}

bool PPU::in_window_bounds(int coord, int start, int end) {
    if (start <= end) {
        return coord >= start && coord < end;
    } else {
        return coord >= start || coord < end;
    }
}

} // namespace gba