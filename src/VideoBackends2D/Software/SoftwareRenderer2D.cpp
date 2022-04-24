#include "Common/Log.h"
#include "Common/Memory.h"
#include "VideoCommon/GPU.h"
#include "VideoCommon/Colour.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"

SoftwareRenderer2D::SoftwareRenderer2D(GPU& gpu, Engine engine) : Renderer2D(gpu, engine) {}

void SoftwareRenderer2D::render_scanline(int line) {
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
            log_fatal("handle");
            // RenderAffine(3, line);
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
    for (int x = 0; x < 256; x++) {
        u16 data = 0;
        u32 offset = ((256 * line) + x) * 2;
        
        switch ((dispcnt >> 18) & 0x3) {
        case 0:
            data = Common::read<u16>(gpu.bank_a.data(), offset);
            break;
        case 1:
            data = Common::read<u16>(gpu.bank_b.data(), offset);
            break;
        case 2:
            data = Common::read<u16>(gpu.bank_c.data(), offset);
            break;
        case 3:
            data = Common::read<u16>(gpu.bank_d.data(), offset);
            break;
        }

        draw_pixel(x, line, rgb555_to_rgb888(data) | 0xFF000000);
    }
}