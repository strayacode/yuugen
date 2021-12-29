#include "core/gba/gpu/gpu.h"

void GBAGPU::ComposeScanline(u16 line) {
    for (int x = 0; x < 240; x++) {
        ComposePixel(line, x);
    }
}

void GBAGPU::ComposePixel(u16 line, u16 x) {
    // set the initial colour as the backdrop colour (palette index 0)
    u16 pixel = 0;
    memcpy(&pixel, &palette_ram[0], 2);

    if (bg_layers[2][(240 * line) + x] != 0x8000) {
        pixel = bg_layers[2][(240 * line) + x];
    }

    framebuffer[(240 * line) + x] = Convert15To24(pixel);
}