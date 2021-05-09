#include <core/hw/gpu/gpu.h>
#include <core/hw/gpu/gpu_2d.h>

void GPU2D::ComposeScanline(u16 line) {
    for (int i = 0; i < 256; i++) {
        // just copy bg layer 0 for now
        framebuffer[(256 * line) + i] = layers[0][(256 * line) + i];
    }
}