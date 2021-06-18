#include <core/hw/gpu/engine_2d/gpu_2d.h>
#include <core/hw/gpu/gpu.h>

void GPU2D::RenderExtended(int bg_index, u16 line) {
    u32 data_base = vram_addr + ((BGCNT[bg_index] >> 8) & 0x1F) * 0x4000;
    u8 screen_size = (BGCNT[bg_index] >> 14) & 0x3;

    u16 size_x = 0;
    u16 size_y = 0;

    switch (screen_size) {
    case 0:
        size_x = 128;
        size_y = 128;
        break;
    case 1:
        size_x = 256;
        size_y = 256;
        break;
    case 2:
        size_x = 512;
        size_y = 256;
        break;
    case 3:
        size_x = 512;
        size_y = 512;
        break;
    }

    printf("extended rendering\n");

    if (!(BGCNT[bg_index] & (1 << 7))) {
        // 16 bit bgmap entries
        // log_warn("[GPU2D] Handle 16 bit bg map entries");
    } else if ((BGCNT[bg_index] & (1 << 7)) && (BGCNT[bg_index] & (1 << 2))) {
        // direct colour bitmap
        // log_warn("[GPU2D] Handle direct colour bitmap");
        for (int pixel = 0; pixel < 256; pixel++) {
            u32 coord_x = (internal_x[bg_index - 2] + BGPA[bg_index - 2] * pixel) >> 8;
            u32 coord_y = (internal_y[bg_index - 2] + BGPC[bg_index - 2] * pixel) >> 8;

            u32 data_addr = data_base + (coord_y * size_x + coord_x) * 2;

            u16 colour = gpu->ReadVRAM<u16>(data_addr);
            
            bg_layers[bg_index][(256 * line) + pixel] = colour;
        }
    } else {
        // 256 colour bitmap
        // log_warn("[GPU2D] Handle 256 colour bitmap");
        for (int pixel = 0; pixel < 256; pixel++) {
            u32 coord_x = (internal_x[bg_index - 2] + BGPA[bg_index - 2] * pixel) >> 8;
            u32 coord_y = (internal_y[bg_index - 2] + BGPC[bg_index - 2] * pixel) >> 8;

            u32 data_addr = data_base + (coord_y * size_x + coord_x);

            u8 palette_index = gpu->ReadVRAM<u8>(data_addr);
            u16 colour = ReadPaletteRAM<u16>(palette_index * 2);

            bg_layers[bg_index][(256 * line) + pixel] = colour;
        }
    }

    // increment the internal registers
    internal_x[bg_index - 2] += BGPB[bg_index - 2];
    internal_y[bg_index - 2] += BGPD[bg_index - 2];
}