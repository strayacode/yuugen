#include <emulator/core/arm7.h>
#include <emulator/emulator.h>
#include <stdio.h>
#include <emulator/common/arithmetic.h>


ARM7::ARM7(Emulator *emulator) : emulator(emulator) {
    
}

void ARM7::read_instruction() {
    opcode = emulator->memory.arm7_read_word(get_reg(15));
}

u32 ARM7::get_reg(u32 reg) {
    u32 cpu_mode = get_bit_range(0, 4, regs.cpsr);
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

void ARM7::direct_boot() {
    // common between arm7 and arm9
    regs.r0 = regs.r1 = regs.r2 = regs.r3 = regs.r4 = regs.r5 = regs.r6 = regs.r7 = regs.r8 = regs.r9 = regs.r10 = regs.r11 = regs.r12 = regs.r14 = 0;

    regs.r8_fiq = regs.r9_fiq = regs.r10_fiq = regs.r11_fiq = regs.r12_fiq = regs.r14_fiq = regs.spsr_fiq = 0;
	regs.r14_svc = regs.spsr_svc = 0;
	regs.r14_abt = regs.spsr_abt = 0;
	regs.r14_irq = regs.spsr_irq = 0;
	regs.r14_und = regs.spsr_und = 0;

    // arm7 specific
    regs.r13 = regs.r13_fiq = regs.r13_abt = regs.r13_und = 0x0380FD80;
    regs.r13_svc = 0x0380FFC0;
    regs.r13_irq = 0x0380FF80;
    regs.r15 = 0x08000000;
    regs.cpsr = 0x0000005F;

    printf("[ARM7] successfully initialised direct boot state\n");
}

void ARM7::firmware_boot() {
    regs.r15 = 0x00000000;
}

void ARM7::reset() {
    #ifdef DIRECT_BOOT
        direct_boot();
    #else
        firmware_boot();
    #endif
}

void ARM7::execute_instruction() {
    // implement proper code
    switch (opcode) {
    default:
        printf("[ARM7] undefined instruction 0x%04x\n", opcode);
        emulator->running = false;
        break;
    }
}

void ARM7::step() {
    read_instruction();
    execute_instruction();
}

bool ARM7::is_arm() {
    return (get_bit(5, regs.cpsr) == 0);
}

bool ARM7::get_condition_flag(int condition_flag) {
    return (regs.cpsr & (1 << condition_flag) != 0);
}

bool ARM7::evaluate_condition() {
    bool n_flag = get_condition_flag(N_FLAG);
    bool z_flag = get_condition_flag(Z_FLAG);
    bool c_flag = get_condition_flag(C_FLAG);
    bool v_flag = get_condition_flag(V_FLAG);
    switch (opcode >> 28) {
        case 0:
            return z_flag;
        case 1:
            return !z_flag;
        case 2:
            return c_flag;
        case 3:
            return !c_flag;
        case 4:
            return n_flag;
        case 5:
            return !n_flag;
        case 6:
            return v_flag;
        case 7:
            return !v_flag;
        case 8:
            return (c_flag && !z_flag);
        case 9:
            return (!c_flag && z_flag);
        case 10:
            return (n_flag == v_flag);
        case 11:
            return (n_flag != v_flag);
        case 12:
            return (!z_flag && (n_flag == v_flag));
        case 13:
            return (z_flag && (n_flag != v_flag));
        case 14:
            return true;
        default:
            printf("[ARM7] condition code %d is not valid!\n", opcode >> 28);
            emulator->running = false;
    }
}