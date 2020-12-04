#include <emulator/core/arm9.h>
#include <emulator/emulator.h>
#include <stdio.h>

ARM9::ARM9(Emulator *emulator) : emulator(emulator) {
    
}

u32 ARM9::get_reg(u32 reg) {
    switch (reg) {
    case 0:
        return regs.r0;
    case 1:
        return regs.r1;
    case 2:
        return regs.r2;
    case 3:
        return regs.r3;
    case 4:
        return regs.r4;
    case 5:
        return regs.r5;
    case 6:
        return regs.r6;
    case 7:
        return regs.r7;
    case 8:
        switch (cpu_mode) {
        case FIQ:
            return regs.r8_fiq;
        default:
            return regs.r8;
        }
    case 9:
        switch (cpu_mode) {
        case FIQ:
            return regs.r9_fiq;
        default:
            return regs.r9;
        }
    case 10:
        switch (cpu_mode) {
        case FIQ:
            return regs.r10_fiq;
        default:
            return regs.r10;
        }
    case 11:
        switch (cpu_mode) {
        case FIQ:
            return regs.r11_fiq;
        default:
            return regs.r11;
        }
    case 12:
        switch (cpu_mode) {
        case FIQ:
            return regs.r12_fiq;
        default:
            return regs.r12;
        }
    case 13:
        switch (cpu_mode) {
        case FIQ:
            return regs.r13_fiq;
        case SVC:
            return regs.r13_svc;
        case ABT:
            return regs.r13_abt;
        case IRQ:
            return regs.r13_irq;
        case UND:
            return regs.r13_und;
        default:
            return regs.r13;
        }
    case 14:
        switch (cpu_mode) {
        case FIQ:
            return regs.r14_fiq;
        case SVC:
            return regs.r14_svc;
        case ABT:
            return regs.r14_abt;
        case IRQ:
            return regs.r14_irq;
        case UND:
            return regs.r14_und;
        default:
            return regs.r14;
        }
    case 15:
        return regs.r15;
    default:
        printf("[ARM9] undefined registers access r%d\n", reg);
        return 0;
    }
}

void ARM9::read_instruction() {
    opcode = emulator->memory.arm9_read_word(get_reg(15));
}

void ARM9::execute_instruction() {
    // implement proper code
    switch (opcode) {
    default:
        printf("[ARM9] undefined instruction 0x%04x\n", opcode);
        emulator->running = false;
        break;
    }
}

void ARM9::direct_boot() {
    // common between arm7 and arm9
    regs.r0 = regs.r1 = regs.r2 = regs.r3 = regs.r4 = regs.r5 = regs.r6 = regs.r7 = regs.r8 = regs.r9 = regs.r10 = regs.r11 = regs.r12 = regs.r14 = 0;

    regs.r8_fiq = regs.r9_fiq = regs.r10_fiq = regs.r11_fiq = regs.r12_fiq = regs.r14_fiq = regs.spsr_fiq = 0;
	regs.r14_svc = regs.spsr_svc = 0;
	regs.r14_abt = regs.spsr_abt = 0;
	regs.r14_irq = regs.spsr_irq = 0;
	regs.r14_und = regs.spsr_und = 0;

    // specific to arm9
    regs.r13 = regs.r13_fiq = regs.r13_abt = regs.r13_und = 0x03002F7C;
    regs.r13_svc = 0x03003FC0;
    regs.r13_irq = 0x03003F80;
    regs.cpsr = 0x0000005F;
    regs.r15 = 0;
    cpu_mode = SYS;

    printf("[ARM9] successfully initialised direct boot state\n");
}

void ARM9::reset() {
    #ifdef DIRECT_BOOT
        direct_boot();
    #else
        firmware_boot();
    #endif
}

void ARM9::step() {
    read_instruction();
    execute_instruction();
}