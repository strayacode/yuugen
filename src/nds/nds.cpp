#include <nds/nds.h>
#include <stdio.h>

NDS::NDS() : arm9(this), arm7(this), memory(this) {
    
}

void NDS::reset() {
    running = true;
    memory.load_arm9_bios();
    memory.load_arm7_bios();
    memory.load_firmware();
    arm9.reset();
    arm7.reset();
}

void NDS::run() {
    reset();

    while (running) {
        arm9.step();
        arm7.step();
    }
}