#include <emulator/emulator.h>
#include <emulator/core/gpu.h>
#include <emulator/common/log.h>
#include <emulator/common/arithmetic.h>

GPU::GPU(Emulator *emulator) : emulator(emulator), engine_a(emulator, 1), engine_b(emulator, 0) {
    // printf("24 bit value: 0x%04x\n", engine_a.sdljew(0x1F));
}

const u32* GPU::get_framebuffer(int screen) {
    switch (screen) {
    case top_screen:
        return (get_bit(15, powcnt1) ? engine_a.get_framebuffer() : engine_b.get_framebuffer());
    case bottom_screen:
        return (get_bit(15, powcnt1) ? engine_b.get_framebuffer() : engine_a.get_framebuffer());
    }
}

void GPU::render_scanline(int line) {
    if (line < 192) {
        engine_a.render_scanline(line);
        engine_b.render_scanline(line);
    }
}