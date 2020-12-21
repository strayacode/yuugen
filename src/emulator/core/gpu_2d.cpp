#include <emulator/core/gpu_2d.h>
#include <emulator/common/types.h>

const u32* GPU2D::get_framebuffer() {
    return &framebuffer[0]; // get the address of the first item in framebuffer
}