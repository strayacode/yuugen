#include "common/logger.h"
#include "common/bits.h"
#include "core/video/ppu/ppu.h"

namespace core {

PPU::PPU(u8* palette_ram, u8* oam, VRAMRegion& bg, VRAMRegion& obj, VRAMRegion& bg_extended_palette, VRAMRegion& lcdc) : 
    palette_ram(palette_ram),
    oam(oam),
    bg(bg),
    obj(obj),
    bg_extended_palette(bg_extended_palette),
    lcdc(lcdc) {}

void PPU::reset() {
    dispcnt.data = 0;
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
    winh.fill(0);
    winv.fill(0);
    winin = 0;
    winout = 0;
    mosaic = 0;
    bldcnt.data = 0;
    bldalpha = 0;
    bldy = 0;
    master_bright = 0;

    framebuffer.fill(0);
    reset_layers();
}

void PPU::render_scanline(int line) {
    reset_layers();

    if (line == 0) {
        internal_x[0] = bgx[0];
        internal_y[0] = bgy[0];
        internal_x[1] = bgx[1];
        internal_y[1] = bgy[1];
    }

    switch (dispcnt.display_mode) {
    case 0:
        render_blank_screen(line);
        break;
    case 1:
        render_graphics_display(line);
        break;
    case 2:
        render_vram_display(line);
        break;
    case 3:
        logger.error("PPU: handle main memory display");
        break;
    }
}

void PPU::write_dispcnt(u32 value, u32 mask) {
    dispcnt.data = (dispcnt.data & ~mask) | (value & mask);
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
    mosaic = (mosaic & ~mask) | (value & mask);
}

void PPU::write_bldcnt(u16 value, u32 mask) {
    bldcnt.data = (bldcnt.data & ~mask) | (value & mask);
}

void PPU::write_bldalpha(u16 value, u32 mask) {
    bldalpha = (bldalpha & ~mask) | (value & mask);
}

void PPU::write_bldy(u16 value, u32 mask) {
    bldy = (bldy & ~mask) | (value & mask);
}

void PPU::render_blank_screen(int line) {
    for (int x = 0; x < 256; x++) {
        render_pixel(x, line, 0xffffffff);
    }
}

void PPU::render_graphics_display(int line) {
    if (dispcnt.enable_bg0) {
        if (dispcnt.bg0_3d || dispcnt.bg_mode == 6) {
            logger.error("PPU: handle 3d rendering");
        } else {
            render_text(0, line);
        }
    }

    if (dispcnt.enable_bg1) {
        if (dispcnt.bg_mode != 6) {
            render_text(1, line);
        }
    }

    if (dispcnt.enable_bg2) {
        switch (dispcnt.bg_mode) {
        case 0:
        case 1:
        case 3:
            render_text(2, line);
            break;
        case 2:
        case 4:
            render_affine(2, line);
            break;
        case 5:
            render_extended(2, line);
            break;
        case 6:
            render_large(2, line);
            break;
        }
    }

    if (dispcnt.enable_bg3) {
        switch (dispcnt.bg_mode) {
        case 0:
            render_text(3, line);
            break;
        case 1:
        case 2:
            render_affine(3, line);
            break;
        case 3:
        case 4:
        case 5:
            render_extended(3, line);
            break;
        }
    }

    if (dispcnt.enable_obj) {
        render_objects(line);
    }

    compose_scanline(line);
}

void PPU::render_vram_display(int line) {
    for (int x = 0; x < 256; x++) {
        u32 addr = (dispcnt.vram_block * 0x20000) + ((256 * line) + x) * 2;
        u16 data = lcdc.read<u16>(addr);
        render_pixel(x, line, rgb555_to_rgb888(data) | 0xff000000);
    }
}

u32 PPU::rgb555_to_rgb888(u32 colour) {
    u8 b = ((colour & 0x1f) * 255) / 31;
    u8 g = (((colour >> 5) & 0x1f) * 255) / 31;
    u8 r = (((colour >> 10) & 0x1f) * 255) / 31;
    return (r << 16) | (g << 8) | b;
}

void PPU::render_pixel(int x, int y, u32 colour) {
    framebuffer[(256 * y) + x] = colour;
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
            logger.todo("PPU: handle object window");
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

void PPU::reset_layers() {
    for (int i = 0; i < 4; i++) {
        bg_layers[i].fill(0);
    }

    obj_priority.fill(4);
    obj_priority.fill(0x8000);
}

} // namespace core