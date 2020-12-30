#include <emulator/core/gpu_2d.h>
#include <emulator/common/types.h>
#include <emulator/core/gpu.h>
#include <emulator/common/arithmetic.h>
#include <emulator/common/log.h>
#include <stdio.h>
#include <string.h>

GPU2D::GPU2D(GPU *gpu, int engine_id) : gpu(gpu), engine_id(engine_id) {

}


const u32* GPU2D::get_framebuffer() {
    return &framebuffer[0]; // get the address of the first item in framebuffer
}

// converts rgb555 to rgb888 for sdl
u32 GPU2D::convert_15_to_24(u32 colour) {
    u8 b = ((colour & 0x1F) * 255) / 31;
    u8 g = (((colour >> 5) & 0x1F) * 255) / 31;
    u8 r = (((colour >> 10) & 0x1F) * 255) / 31;
    return (b << 16) | (g << 8) | r;
}

void GPU2D::render_scanline(int line) {
    
    // we have only done mode 0 and 2 of display mode so for 
    u8 display_mode = (dispcnt >> 16) & 0x3;
    render_blank_screen(line);
    switch (display_mode) {
    case 0:
        // screen becomes white
        render_blank_screen(line);
        break;
    case 2:
        
        // use vram display
        render_vram_display(line);
        break;
    default:
        log_fatal("[GPU2D] 2d display mode %d is not implemented yet!\n", display_mode);
    }
}

void GPU2D::render_blank_screen(int line) {
    memset(&framebuffer[line * 256], 0xFF, 256 * sizeof(u32));
}

void GPU2D::render_vram_display(int line) {
    for (int i = 0; i < 256; i++) {
        u16 data;
        switch (get_vram_bank()) {
        case 0:
            data = (gpu->vram_a[(i * 2) + 1] << 8 | gpu->vram_a[i * 2]);
            break;
        case 1:
            data = (gpu->vram_b[(i * 2) + 1] << 8 | gpu->vram_b[i * 2]);
            break;
        case 2:
            data = (gpu->vram_c[(i * 2) + 1] << 8 | gpu->vram_c[i * 2]);
            break;
        case 3:
            data = (gpu->vram_d[(i * 2) + 1] << 8 | gpu->vram_d[i * 2]);
            break;
        }

        framebuffer[(256 * line) + i] = convert_15_to_24(data);
    }
}

u8 GPU2D::get_vram_bank() {
    return get_bit_range(18, 19, dispcnt);
}


