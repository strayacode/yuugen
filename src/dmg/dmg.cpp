#include <dmg/dmg.h>
#include <stdio.h>

DMG::DMG() {
    running = true;
}

void DMG::run() {
    printf("[DMG] running!\n");
    sm83.memory.cartridge.load_bootrom();
    while (running) {
        sm83.step();
    }
}