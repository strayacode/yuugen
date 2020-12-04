#include <emulator/emulator.h>

int main() {
    Emulator emulator;
    emulator.reset();
    while (emulator.running) {
        emulator.arm9.step();
        emulator.arm7.step();
    }
    
    return 0;
}