#include <emulator/emulator.h>
#include <stdio.h>

Emulator::Emulator() : arm9(this), arm7(this), memory(this) {
    
}

void Emulator::reset() {
    memory.load_arm9_bios();
    memory.load_arm7_bios();
    running = true;
    arm9.reset();
    arm7.reset();
}