#include <emulator/core/arm.h>
#include <emulator/emulator.h>
#include <emulator/core/memory.h>
#include <stdio.h>

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

void ARM7::read_instruction() {
    opcode = emulator->memory.arm7_read_word(get_pc());
}

void ARM9::read_instruction() {
    opcode = emulator->memory.arm9_read_word(get_pc());
}

void ARM7::execute_instruction() {
    switch (opcode) {
        default:
            printf("[ARM7] opcode 0x%04x has not been implemented yet!\n", opcode);
            emulator->running = false;
    }
}

void ARM9::execute_instruction() {
    switch (opcode) {
        default:
            printf("[ARM9] opcode 0x%04x has not been implemented yet!\n", opcode);
            emulator->running = false;
    }
}



void ARM7::step() {
    read_instruction();
    execute_instruction();
}

void ARM9::step() {
    read_instruction();
    execute_instruction();
}

