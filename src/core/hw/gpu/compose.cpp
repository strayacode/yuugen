#include <core/hw/gpu/gpu.h>
#include <core/hw/gpu/gpu_2d.h>

void GPU2D::ComposeScanline(u16 line) {
    for (int i = 0; i < 256; i++) {
        u8 priority = 0;
        u8 priority_bg_index = 3;
        for (int i = 3; i >= 0; i--) {
            if ((BGCNT[i] & 0x3) >= priority) {
                priority_bg_index = i;
                priority = (BGCNT[i] & 0x3);
            } 
        }

        framebuffer[(256 * line) + i] = layers[priority_bg_index][(256 * line) + i];
    }
}