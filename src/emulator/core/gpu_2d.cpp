#include <emulator/core/gpu_2d.h>
#include <emulator/common/types.h>
// #include <emulator/core/gpu.h>
#include <stdio.h>



const u32* GPU2D::get_framebuffer() {
    return &framebuffer[0]; // get the address of the first item in framebuffer
}

// converts rgb555 to rgb888 for sdl
u32 GPU2D::convert_15_to_24(u32 colour) {
    // printf("%04x\n", colour);
    u8 b = ((colour & 0x1F) * 255) / 31;
    u8 g = (((colour >> 5) & 0x1F) * 255) / 31;
    u8 r = (((colour >> 10) & 0x1F) * 255) / 31;
    
    return (b << 16) | (g << 8) | r;
}


