#include "Common/Log.h"
#include "Common/Memory.h"
#include "VideoCommon/GPU.h"
#include "VideoCommon/Colour.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"

SoftwareRenderer2D::SoftwareRenderer2D(GPU& gpu, Engine engine) : Renderer2D(gpu, engine) {}

void SoftwareRenderer2D::render_scanline(int line) {
    for (int i = 0; i < 4; i++) {
        bg_layers[i][(256 * line) + i] = 0;
    }

    for (int i = 0; i < 256; i++) {
        obj_priority[(256 * line) + i] = 4;
        obj_colour[(256 * line) + i] = 0x8000;
    }

    // reload the internal registers
    if (line == 0) {
        internal_x[0] = bgx[0];
        internal_y[0] = bgy[0];
        internal_x[1] = bgx[1];
        internal_y[1] = bgy[1];
    }

    switch ((dispcnt >> 16) & 0x3) {
    case 0:
        render_blank_screen(line);
        break;
    case 1:
        render_graphics_display(line);
        break;
    case 2:
        render_vram_display(line);
        break;
    default:
        log_fatal("SoftwareRenderer2D: display mode %d not implemented", (dispcnt >> 16) & 0x3);
    }
}

void SoftwareRenderer2D::render_blank_screen(int line) {
    for (int x = 0; x < 256; x++) {
        draw_pixel(x, line, 0xFFFFFFFF);
    }
}

void SoftwareRenderer2D::render_graphics_display(int line) {
    u8 bg_mode = dispcnt & 0x7;

    if (dispcnt & (1 << 8)) {
        if ((dispcnt & (1 << 3)) || (bg_mode == 6)) {
            for (int i = 0; i < 256; i++) {
                bg_layers[0][(256 * line) + i] = gpu.renderer_3d->framebuffer[(256 * line) + i];
            }
        } else {
            render_text(0, line);
        }
    }

    if (dispcnt & (1 << 9)) {
        if (bg_mode != 6) {
            render_text(1, line);
        }
    }

    if (dispcnt & (1 << 10)) {
        switch (bg_mode) {
        case 0:
        case 1:
        case 3:
            render_text(2, line);
            break;
        case 2:
        case 4:
            log_fatal("handle");
            // RenderAffine(2, line);
            break;
        case 5:
            render_extended(2, line);
            break;
        case 6:
            log_fatal("handle");
            // RenderLarge(2, line);
            break;
        }
    }

    if (dispcnt & (1 << 11)) {
        switch (bg_mode) {
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

    if (dispcnt & (1 << 12)) {
        render_objects(line);
    }

    compose_scanline(line);
}

void SoftwareRenderer2D::render_vram_display(int line) {
    u8 vram_block = (dispcnt >> 18) & 0x3;

    for (int x = 0; x < 256; x++) {
        u32 addr = 0x06800000 + (vram_block * 0x20000) + ((256 * line) + x) * 2;
        u16 data = gpu.vram.read_vram<u16>(addr);
        
        draw_pixel(x, line, rgb555_to_rgb888(data) | 0xFF000000);
    }
}