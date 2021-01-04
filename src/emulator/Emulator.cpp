#include <emulator/Emulator.h>
#include <stdio.h>
#include <string>
#include <SDL2/SDL.h>
#include <emulator/common/log.h>

Emulator::Emulator() : arm9(this, 1), arm7(this, 0), memory(this), cartridge(this), gpu(this), dma {DMA(this, 0), DMA(this, 1)} {

}

void Emulator::reset() {
    running = true;
    memory.load_arm9_bios();
    memory.load_arm7_bios();
    memory.load_firmware();
    arm9.reset();
    arm7.reset();
}

void Emulator::run_nds_frame() {
    // quick sidenote
    // in 1 frame of the nds executing
    // there are 263 scanlines with 192 visible and 71 for vblank
    // in each scanline there are 355 dots in total with 256 visible and 99 for hblank
    // 3 cycles of the arm7 occurs per dot and 6 cycles of the arm9 occurs per dot
    for (int i = 0; i < 263; i++) {
        for (int j = 0; j < 355 * 3; j++) {
            // run arm9 and arm7 stuff
            // since arm9 runs at the twice the clock speed of arm7 we run it 2 times instead of 1
            
            arm9.step();
            arm9.step();
            arm7.step();
            
        }
        gpu.render_scanline(i);
    }
}



