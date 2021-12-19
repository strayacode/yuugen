#include "core/gba/gba.h"

GBA::GBA() : memory(*this), cpu_core(memory, CPUArch::ARMv4, nullptr) {}

void GBA::Reset() {
    cartridge.LoadRom(game_path);
    scheduler.Reset();
    memory.Reset();
    cartridge.Reset();
    cpu_core.Reset();
}

// TODO: use a struct called BootParameters instead when we decide to add more
// parameters
void GBA::Boot(bool direct) {
    if (direct) {
        cpu_core.DirectBoot(0x08000000);
    }
}

void GBA::RunFrame() {
    u64 frame_end_time = scheduler.GetCurrentTime() + 280896;

    while (scheduler.GetCurrentTime() < frame_end_time) {
        cpu_core.RunInterpreter(1);
        scheduler.Tick(1);
        scheduler.RunEvents();
    }
}