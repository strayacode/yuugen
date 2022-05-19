#include "Common/Memory.h"
#include "VideoCommon/Colour.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"

void SoftwareRenderer2D::compose_scanline(int line) {
    for (int x = 0; x < 256; x++) {
        compose_pixel(x, line);
    }
}

void SoftwareRenderer2D::compose_pixel(int x, int line) {
    u8 enabled = dispcnt >> 8;

    if ((dispcnt >> 13) & 0x7) {
        u8 win0_x1 = winh[0] >> 8;
        u8 win0_x2 = winh[0] & 0xFF;
        u8 win0_y1 = winv[0] >> 8;
        u8 win0_y2 = winv[0] & 0xFF;
        
        u8 win1_x1 = winh[1] >> 8;
        u8 win1_x2 = winh[1] & 0xFF;
        u8 win1_y1 = winv[1] >> 8;
        u8 win1_y2 = winv[1] & 0xFF;

        // TODO: add object window when we doing object rendering
        if ((dispcnt & (1 << 13)) && x > win0_x1 && x <= win0_x2 && line > win0_y1 && line <= win0_y2) {
            enabled &= (winin & 0xF);
        } else if ((dispcnt & (1 << 14)) && x > win1_x1 && x <= win1_x2 && line > win1_y1 && line <= win1_y2) {
            enabled &= ((winin >> 8) & 0xF);
        } else {
            enabled &= (winout & 0xF);  
        }
    }

    // set the initial pixel to be the colour at palette index 0 (the backdrop colour)
    u16 pixel = Common::read<u16>(palette_ram, 0);

    u8 priority = 3;
    for (int i = 3; i >= 0; i--) {
        // only check priority if the bg layer is enabled as indicated by the variable enabled, which has taken into account window logic
        if ((enabled & (1 << i)) && (bg_layers[i][(256 * line) + x] != 0x8000)) {
            if ((bgcnt[i] & 0x3) <= priority) {
                priority = bgcnt[i] & 0x3;
                pixel = bg_layers[i][(256 * line) + x];
            }
        }
    }
    
    if ((dispcnt & (1 << 12)) && (obj_colour[(256 * line) + x] != 0x8000)) {
        // only draw an obj pixel instead of a bg pixel if it has a higher priority or same priority than the relative bg priority index
        if (obj_priority[(256 * line) + x] <= priority) {
            pixel = obj_colour[(256 * line) + x];
        }
    }

    draw_pixel(x, line, rgb555_to_rgb888(pixel) | 0xFF000000);
}