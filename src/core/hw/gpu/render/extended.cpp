#include <core/hw/gpu/gpu_2d.h>
#include <core/hw/gpu/gpu.h>

void GPU2D::RenderExtended(int bg_index, u16 line) {
    if (!(BGCNT[bg_index] & (1 << 7))) {
        // 16 bit bgmap entries
        log_fatal("[GPU2D] Handle 16 bit bg map entries");
    } else if ((BGCNT[bg_index] & (1 << 7)) && (BGCNT[bg_index] & (1 << 2))) {
        // direct colour bitmap
        log_fatal("[GPU2D] Handle direct colour bitmap");
    } else {
        // 256 colour bitmap
        u32 screen_base = ((BGCNT[bg_index] >> 8) & 0x1F) * 0x4000;

        log_fatal("[GPU2D] Handle 256 colour bitmap");
        u8 screen_size = (BGCNT[bg_index] >> 14) & 0x3;
        if (screen_size != 1) {
            log_fatal("handle");
        }

        for (int pixel = 0; pixel < 256; pixel++) {

        }
    }
}