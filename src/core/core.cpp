#include <core/core.h>

Core::Core() :
    cartridge(this),
    arm9(this, ARMv5), 
    arm7(this, ARMv4),
    gpu(this),
    dma {DMA(this, 0), DMA(this, 1)},
    cp15(this),
    interrupt {Interrupt(this, 0), Interrupt(this, 1)},
    ipc(this),
    timers {Timers(this, 0), Timers(this, 1)},
    rtc(this),
    spu(this),
    spi(this),
    memory(this) {
    
}

void Core::Reset() {
    cartridge.Reset();
    arm9.Reset();
    arm7.Reset();
    gpu.Reset();
    memory.Reset();

    dma[0].Reset();
    dma[1].Reset();

    cp15.Reset();

    input.Reset();

    interrupt[0].Reset();
    interrupt[1].Reset();

    ipc.Reset();

    rtc.Reset();

    spu.Reset();

    spi.Reset();
}

void Core::DirectBoot() {
    cartridge.LoadROM(rom_path);
    cartridge.DirectBoot();

    arm9.DirectBoot();
    arm7.DirectBoot();

    cp15.DirectBoot();
    spi.DirectBoot();
}

void Core::FirmwareBoot() {
    
}

void Core::RunFrame() {
    // quick sidenote
    // in 1 frame of the nds executing
    // there are 263 scanlines with 192 visible and 71 for vblank
    // in each scanline there are 355 dots in total with 256 visible and 99 for hblank
    // 3 cycles of the arm7 occurs per dot and 6 cycles of the arm9 occurs per dot
    for (int i = 0; i < 263; i++) {
        for (int j = 0; j < 355 * 3; j++) {
            // since the arm9 runs at double the clock rate of the arm7 we step the arm9 cpu 2 times and the arm7 only once
            for (int k = 0; k < 2; k++) {
                arm9.Step();
                if (dma[1].enabled) {
                    dma[1].Transfer();
                }
            }
            

            arm7.Step();
            if (dma[0].enabled) {
                dma[0].Transfer();
            }

            if (j == 768) {
                gpu.RenderScanlineStart();
            }
        }
        gpu.RenderScanlineFinish();
    }
}

void Core::SetRomPath(const char* path) {
    rom_path = strdup(path);
}
