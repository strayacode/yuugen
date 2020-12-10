#include <dmg/core/sm83.h>
#include <stdio.h>
#include <stdlib.h>

SM83::SM83() {
    regs.a = regs.b = regs.c = regs.d = regs.e = regs.f = regs.h = regs.l = regs.pc = regs.sp = 0;
}

void SM83::step() {
    read_instruction();
    execute_instruction();
    debug_registers();
}

void SM83::tick() {
    cycles += 4;
}

void SM83::read_instruction() {
    opcode = memory.read_byte(regs.pc);
    tick();
    regs.pc++;
}

void SM83::execute_instruction() {
    switch (opcode) {
    case 0x31:
        ld_sp_u16(); break;
    case 0xAF:
        regs.a = xor_byte(regs.a, regs.a); 
        break;
    default:
        printf("[SM83] instruction 0x%02x is undefined!\n", opcode);
        exit(1);
    }
}

void SM83::debug_registers() {
    printf("[SM83] A: %02x F: %02x B: %02x C: %02x D: %02x E: %02x H: %02x L: %02x PC: %04x SP: %04x\n", 
    regs.a, regs.f, regs.b, regs.c, regs.d, regs.e, regs.h, regs.l, regs.pc, regs.sp);
}

bool SM83::get_flag(u8 index) {
    return (regs.f & (1 << index)) != 0;
}

void SM83::set_flag(u8 index, u8 value) {
    if (value) {
        regs.f |= (1 << index);
    } else {
        regs.f &= ~(1 << index);
    }
}