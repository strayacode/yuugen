#include <core/core.h>

Core::Core() : 
    cartridge(this),
    memory(this),
    arm7(this, ARMv4),
    arm9(this, ARMv5),
    spi(this),
    cp15(this),
    gpu(this),
    dma {DMA(this, 0), DMA(this, 1)},
    ipc(this),
    interrupt {Interrupt(this, 0), Interrupt(this, 1)},
    timers {Timers(this, 0), Timers(this, 1)},
    spu(this),
    rtc(this) {

}

void Core::Reset() {
    cartridge.Reset();
    cartridge.LoadRom(rom_path);

    arm9.Reset();
    arm7.Reset();
    gpu.Reset();
    memory.Reset();

    dma[0].Reset();
    dma[1].Reset();

    timers[0].Reset();
    timers[1].Reset();

    cp15.Reset();

    spi.Reset();

    input.Reset();

    ipc.Reset();

    interrupt[0].Reset();
    interrupt[1].Reset();

    maths_unit.Reset();

    wifi.Reset();
}

void Core::DirectBoot() {
    cp15.DirectBoot();
    memory.DirectBoot();
    cartridge.DirectBoot();
    arm9.DirectBoot();
    arm7.DirectBoot();
    spi.DirectBoot();
}

void Core::FirmwareBoot() {
    arm9.FirmwareBoot();
    arm7.FirmwareBoot();
}

void Core::SetRomPath(std::string path) {
    rom_path = path;
}

void Core::RunFrame() {
    // quick sidenote
    // in 1 frame of the nds executing
    // there are 263 scanlines with 192 visible and 71 for vblank
    // in each scanline there are 355 dots in total with 256 visible and 99 for hblank
    // 3 cycles of the arm7 occurs per dot and 6 cycles of the arm9 occurs per dot
    // so thus each frame consists of 263 * 355 * 6 cycles based on arm9 clock speed
    // which is = 560190

    u64 frame_end_time = scheduler.GetCurrentTime() + 560190;

    // run frame for total of 560190 arm9 cycles
    while (scheduler.GetCurrentTime() < frame_end_time) {
        // determine the number of cycles to run the arm9 and arm7 for
        u32 cycles = std::min(frame_end_time, scheduler.GetEventTime()) - scheduler.GetCurrentTime();

        for (u32 i = 0; i < cycles; i++) {
            arm9.Step();

            if (timers[1].enabled) {
                timers[1].Tick(1);
            }

            arm9.Step();

            if (timers[1].enabled) {
                timers[1].Tick(1);
            }

            arm7.Step();

            if (timers[0].enabled) {
                timers[0].Tick(2);
            }
        }

        // make sure to tick the scheduler by cycles
        scheduler.Tick(cycles);

        // do any events
        scheduler.RunEvents();
    }
}