#include <emulator/core/arm.h>
#include <emulator/emulator.h>

ARM9::ARM9(Emulator *emulator) : emulator(emulator) {
    set_pc(0xFFFF0000);
}

ARM7::ARM7(Emulator *emulator) : emulator(emulator) {
    set_pc(0x00000000);
}

u32 ARM::get_pc() {
    return regs.r[15];
}

void ARM::set_pc(u32 value) {
    regs.r[15] = value;
}

void ARM7::read_opcode() {
    opcode = emulator->memory.arm7_read_word(get_pc());
}

