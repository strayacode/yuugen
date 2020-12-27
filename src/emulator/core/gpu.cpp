#include <emulator/emulator.h>
#include <emulator/core/gpu.h>
#include <emulator/common/log.h>

GPU::GPU(Emulator *emulator) : emulator(emulator) {
    // printf("24 bit value: 0x%04x\n", engine_a.sdljew(0x1F));
}

// for just mode 2 lol
// move to gpu_2d later
void GPU::fill_framebuffer() {
    for (int i = 0; i < (256 * 192); i++) { 
        engine_a.framebuffer[i] = engine_a.convert_15_to_24(emulator->memory.lcdc_vram[(i * 2) + 1] << 8 | emulator->memory.lcdc_vram[i * 2]); 
    }
}

void GPU::draw_scanline(int line) {
    engine_a.draw_scanline(line);
    engine_b.draw_scanline(line);
}