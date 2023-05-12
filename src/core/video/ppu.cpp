#include "core/video/video_unit.h"
#include "core/video/ppu.h"

namespace core {

PPU::PPU(VideoUnit& video_unit, Engine engine) : video_unit(video_unit), engine(engine) {}

void PPU::reset() {
    dispcnt.data = 0;
    framebuffer.fill(0);
}

void PPU::render_scanline(int line) {
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

void PPU::render_blank_screen(int line) {
    for (int x = 0; x < 256; x++) {
        render_pixel(x, line, 0xffffffff);
    }
}

void PPU::render_graphics_display(int line) {
    // TODO: handle graphics display rendering later
    // u8 bg_mode = dispcnt & 0x7;

    // if (dispcnt & (1 << 8)) {
    //     if ((dispcnt & (1 << 3)) || (bg_mode == 6)) {
    //         for (int i = 0; i < 256; i++) {
    //             bg_layers[0][(256 * line) + i] = video_unit.renderer_3d.framebuffer[(256 * line) + i];
    //         }
    //     } else {
    //         render_text(0, line);
    //     }
    // }

    // if (dispcnt & (1 << 9)) {
    //     if (bg_mode != 6) {
    //         render_text(1, line);
    //     }
    // }

    // if (dispcnt & (1 << 10)) {
    //     switch (bg_mode) {
    //     case 0:
    //     case 1:
    //     case 3:
    //         render_text(2, line);
    //         break;
    //     case 2:
    //     case 4:
    //         log_fatal("handle");
    //         // RenderAffine(2, line);
    //         break;
    //     case 5:
    //         render_extended(2, line);
    //         break;
    //     case 6:
    //         log_fatal("handle");
    //         // RenderLarge(2, line);
    //         break;
    //     }
    // }

    // if (dispcnt & (1 << 11)) {
    //     switch (bg_mode) {
    //     case 0:
    //         render_text(3, line);
    //         break;
    //     case 1:
    //     case 2:
    //         render_affine(3, line);
    //         break;
    //     case 3:
    //     case 4:
    //     case 5:
    //         render_extended(3, line);
    //         break;
    //     }
    // }

    // if (dispcnt & (1 << 12)) {
    //     render_objects(line);
    // }

    // compose_scanline(line);
}

void PPU::render_vram_display(int line) {
    for (int x = 0; x < 256; x++) {
        u32 addr = 0x06800000 + (dispcnt.vram_block * 0x20000) + ((256 * line) + x) * 2;
        u16 data = video_unit.vram.read<u16>(addr);
        render_pixel(x, line, rgb555_to_rgb888(data) | 0xff000000);
    }
}

u32 PPU::rgb555_to_rgb888(u32 colour) {
    u8 b = ((colour & 0x1F) * 255) / 31;
    u8 g = (((colour >> 5) & 0x1F) * 255) / 31;
    u8 r = (((colour >> 10) & 0x1F) * 255) / 31;
    return (r << 16) | (g << 8) | b;
}

void PPU::render_pixel(int x, int y, u32 colour) {
    framebuffer[(256 * y) + x] = colour;
}

} // namespace core