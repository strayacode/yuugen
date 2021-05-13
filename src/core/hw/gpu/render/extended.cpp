#include <core/hw/gpu/gpu_2d.h>
#include <core/hw/gpu/gpu.h>

void GPU2D::RenderExtended(int bg_index, u16 line) {
    u32 data_base = vram_addr + ((BGCNT[bg_index] >> 8) & 0x1F) * 0x4000;
    u8 screen_size = (BGCNT[bg_index] >> 14) & 0x3;
    if (screen_size != 1) {
        log_fatal("handle");
    }

    if (!(BGCNT[bg_index] & (1 << 7))) {
        // 16 bit bgmap entries
        log_fatal("[GPU2D] Handle 16 bit bg map entries");
    } else if ((BGCNT[bg_index] & (1 << 7)) && (BGCNT[bg_index] & (1 << 2))) {
        // direct colour bitmap
        for (int pixel = 0; pixel < 256; pixel++) {
            u32 data_addr = data_base + (pixel * 2);
            u16 colour;
            if (engine_id == 1) {
                colour = gpu->ReadBGA(data_addr);
            } else {
                colour = gpu->ReadBGB(data_addr);
            }
            layers[bg_index][(256 * line) + pixel] = Convert15To24(colour);
        }
    } else {
        // 256 colour bitmap
        log_fatal("[GPU2D] Handle 256 colour bitmap");
        

        for (int pixel = 0; pixel < 256; pixel++) {

        }
    }
}