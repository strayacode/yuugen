#include <emulator/core/arm.h>
#include <emulator/emulator.h>
#include <emulator/common/arithmetic.h>
#include <emulator/common/types.h>
#include <emulator/core/disassembler.h>
#include <emulator/common/log.h>
#include <functional>

ARM::ARM(Emulator *emulator, int cpu_id): emulator(emulator), cpu_id(cpu_id) {
    fill_arm_lut_table();
    #ifdef FILE_LOG
    // file = fopen("armwrestler2.log", "w");
    #endif
    // do thumb later lol
}

// memory handlers
u8 ARM::read_byte(u32 addr) {
    if (!cpu_id) {
        return emulator->memory.arm7_read_byte(addr);
    }
    return emulator->memory.arm9_read_byte(addr);
}

u16 ARM::read_halfword(u32 addr) {
    if (!cpu_id) {
        return emulator->memory.arm7_read_halfword(addr);
    }
    return emulator->memory.arm9_read_halfword(addr);
}

u32 ARM::read_word(u32 addr) {
    if (!cpu_id) {
        return emulator->memory.arm7_read_word(addr);
    }
    return emulator->memory.arm9_read_word(addr);
}

void ARM::write_byte(u32 addr, u8 data) {
    if (!cpu_id) {
        emulator->memory.arm7_write_byte(addr, data);
    } else {
        emulator->memory.arm9_write_byte(addr, data);
    }
}

void ARM::write_halfword(u32 addr, u16 data) {
    if (!cpu_id) {
        emulator->memory.arm7_write_halfword(addr, data);
    } else {
        emulator->memory.arm9_write_halfword(addr, data);
    }
}

void ARM::write_word(u32 addr, u32 data) {
    if (!cpu_id) {
        emulator->memory.arm7_write_word(addr, data);
    } else {
        emulator->memory.arm9_write_word(addr, data);
    }
}

u32 ARM::get_reg(u8 reg) {
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
        log_fatal("[ARM] undefined register read r%d", reg);
        emulator->running = false;
        return 0;
    }
}

void ARM::set_reg(u8 reg, u32 data) {
    u32 cpu_mode = get_bit_range(0, 4, regs.cpsr);
    switch (reg) {
    case 0:
        regs.r0 = data; break;
    case 1: 
        regs.r1 = data; break;
    case 2:
        regs.r2 = data; break;
    case 3:
        regs.r3 = data; break;
    case 4:
        regs.r4 = data; break;
    case 5:
        regs.r5 = data; break;
    case 6:
        regs.r6 = data; break;
    case 7:
        regs.r7 = data; break;
    case 8:
        switch (cpu_mode) {
        case FIQ:
            regs.r8_fiq = data; break;
        default:
            regs.r8 = data; break;
        }
        break;
    case 9:
        switch (cpu_mode) {
        case FIQ:
            regs.r9_fiq = data; break;
        default:
            regs.r9 = data; break;
        }
        break;
    case 10:
        switch (cpu_mode) {
        case FIQ:
            regs.r10_fiq = data; break;
        default:
            regs.r10 = data; break;
        }
        break;
    case 11:
        switch (cpu_mode) {
        case FIQ:
            regs.r11_fiq = data; break;
        default:
            regs.r11 = data; break;
        }
        break;
    case 12:
        switch (cpu_mode) {
        case FIQ:
            regs.r12_fiq = data; break;
        default:
            regs.r12 = data; break;
        }
        break;
    case 13:
        switch (cpu_mode) {
        case FIQ:
            regs.r13_fiq = data; break;
        case SVC:
            regs.r13_svc = data; break;
        case ABT:
            regs.r13_abt = data; break;
        case IRQ:
            regs.r13_irq = data; break;
        case UND:
            regs.r13_und = data; break;
        default:
            regs.r13 = data; break;
        }
        break;
    case 14:
        switch (cpu_mode) {
        case FIQ:
            regs.r14_fiq = data; break;
        case SVC:
            regs.r14_svc = data; break;
        case ABT:
            regs.r14_abt = data; break;
        case IRQ:
            regs.r14_irq = data; break;
        case UND:
            regs.r14_und = data; break;
        default:
            regs.r14 = data; break;
        }
        break;
    case 15:
        regs.r15 = data; break;
    default:
        log_fatal("[ARM] undefined register write r%d", reg);
        emulator->running = false;
    }
}

void ARM::fill_arm_lut_table() {
    for (int i = 0; i < 4096; i++) {
        if ((i & 0b111100000000) == 0b111100000000) {
            // software interrupt still need to implement
            arm_lut_table[i] = &ARM::arm_undefined;
        } else if ((i & 0b111000000001) == 0b111000000001) {
            // coprocessor register transfer (on arm9 only i think)
            arm_lut_table[i] = &ARM::arm_undefined;
        } else if ((i & 0b111000000001) == 0b111000000000) {
            // coprocessor data operation (on arm9 only i think)
            arm_lut_table[i] = &ARM::arm_undefined;
        } else if ((i & 0b110000000000) == 0b110000000000) {
            // coprocessor data transfer (on arm9 only i think)
            arm_lut_table[i] = &ARM::arm_undefined;
        } else if ((i & 0b111000000000) == 0b101000000000) {
            arm_lut_table[i] = &ARM::arm_branch;
        } else if ((i & 0b111000000000) == 0b100000000000) {
            // block data transfer still need to implement
            arm_lut_table[i] = &ARM::arm_block_data_transfer;
        } else if ((i & 0b111000000001) == 0b011000000001) {
            // undefined instruction
            arm_lut_table[i] = &ARM::arm_undefined;
        } else if ((i & 0b110000000000) == 0b010000000000) {
            // single data transfer still need to implement
            arm_lut_table[i] = &ARM::arm_single_data_transfer;
        } else if ((i & 0b111001001001) == 0b000001001001) {
            // halfword data transfer: immediate offset still need to implement
            arm_lut_table[i] = &ARM::arm_halfword_data_transfer_immediate;
        } else if ((i & 0b111001001001) == 0b000000001001) {
            // halfword data transfer: register offset still need to implement
            arm_lut_table[i] = &ARM::arm_undefined;
        } else if (i == 0b000100100001) {
            // branch with exchange still need to implement
            arm_lut_table[i] = &ARM::arm_branch_exchange;
        } else if ((i & 0b111110111111) == 0b000100001001) {
            // single data swap still need to implement
            arm_lut_table[i] = &ARM::arm_undefined;
        } else if ((i & 0b111110001111) == 0b000010001001) {
            // multiply long still need to implement
            arm_lut_table[i] = &ARM::arm_undefined;
        } else if ((i & 0b111111001111) == 0b000000001001) {
            // multiply still need to implement
            arm_lut_table[i] = &ARM::arm_undefined;
        } else if ((i & 0b110000000000) == 0b000000000000) {
            // data processing / psr transfer still need to implement
            arm_lut_table[i] = &ARM::arm_data_processing;
        } else {
            arm_lut_table[i] = &ARM::arm_undefined;
        }
    }
}

void ARM::execute_instruction() {
    #ifdef FILE_LOG
    if (counter == 1000000) {
        exit(1);
    }
    // fprintf(file, "%d [ARM] r0: %08x r1: %08x r2: %08x r3: %08x r4: %08x: r5: %08x r6: %08x: r7: %08x r8: %08x r9: %08x r10: %08x r11: %08x r12: %08x r13: %08x r14: %08x r15: %08x\n", counter, regs.r0, regs.r1, regs.r2, regs.r3, regs.r4, regs.r5, regs.r6, regs.r7, regs.r8, regs.r9, regs.r10, regs.r11, regs.r12, regs.r13, regs.r14, regs.r15);
    counter++;
    #endif
    if (is_arm()) {
        if (condition_evaluate()) {
            u32 index = ((opcode >> 16) & 0xFF0) | ((opcode >> 4) & 0xF);
            std::invoke(arm_lut_table[index], this);
        } else {
            regs.r15 += 4;
        }
    } else {
        // execute thumb instruction
        log_fatal("i have not implemented thumb yet for anything lol");
    }
    
}

void ARM::direct_boot() {
    // common between arm7 and arm9
    regs.r0 = regs.r1 = regs.r2 = regs.r3 = regs.r4 = regs.r5 = regs.r6 = regs.r7 = regs.r8 = regs.r9 = regs.r10 = regs.r11 = 0;

    // are changed to entry point later through cartridge header
    regs.r12 = regs.r14 = 0;

    regs.r8_fiq = regs.r9_fiq = regs.r10_fiq = regs.r11_fiq = regs.r12_fiq = regs.r14_fiq = regs.spsr_fiq = 0;
	regs.r14_svc = regs.spsr_svc = 0;
	regs.r14_abt = regs.spsr_abt = 0;
	regs.r14_irq = regs.spsr_irq = 0;
	regs.r14_und = regs.spsr_und = 0;

    if (!cpu_id) {
        // arm7 specific
        regs.r13 = regs.r13_fiq = regs.r13_abt = regs.r13_und = 0x0380FD80;
        regs.r13_svc = 0x0380FFC0;
        regs.r13_irq = 0x0380FF80;
        regs.r15 = emulator->cartridge.header.arm7_entry_address;
        regs.cpsr = 0x0000001F;

    } else {
        // specific to arm9
        regs.r13 = regs.r13_fiq = regs.r13_abt = regs.r13_und = 0x03002F7C;
        regs.r13_svc = 0x03003FC0;
        regs.r13_irq = 0x03003F80;
        regs.r15 = emulator->cartridge.header.arm9_entry_address;
        regs.cpsr = 0x0000001F;
    }
    log_debug("[ARM] successfully initialised direct boot state");
}

void ARM::firmware_boot() {
    regs.r15 = 0xFFFF0000;
}

void ARM::reset() {
    // for now we will just direct boot
    direct_boot();
    flush_pipeline();
}

void ARM::step() {
    // stepping the pipeline must happen before an instruction is executed incase the instruction is a branch which would flush and then step the pipeline (not correct)
    opcode = pipeline[0]; // store the current executing instruction 
    // shift the pipeline
    pipeline[0] = pipeline[1];
    // fill the 2nd item with the new instruction to be read
    if (is_arm()) {
        pipeline[1] = read_word(regs.r15);
    } else {
        log_fatal("still need to implement thumb");
        // pipeline[1] = read_halfword(regs.r15);
    }
    // disassemble_instruction(opcode);
    // printf("execute arm9 instruction lol\n");
    execute_instruction();
    // printf("arm9 pc: %04x\n", regs.r15);
}

bool ARM::is_arm() {
    return (get_bit(5, regs.cpsr) == 0);
}

bool ARM::get_condition_flag(int condition_flag) {
    return ((regs.cpsr & (1 << condition_flag)) != 0);
}

void ARM::set_condition_flag(int condition_flag, bool data) {
    if (data) {
        regs.cpsr |= (1 << condition_flag);
    } else {
        regs.cpsr &= ~(1 << condition_flag);
    }
}

bool ARM::condition_evaluate() {
    bool n_flag = get_condition_flag(N_FLAG);
    bool z_flag = get_condition_flag(Z_FLAG);
    bool c_flag = get_condition_flag(C_FLAG);
    bool v_flag = get_condition_flag(V_FLAG);
    // printf("%d\n", opcode >> 28);
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
            printf("[ARM] condition code %d is not valid!\n", pipeline[0] >> 28);
            emulator->running = false;
            return false;
    }
}

void ARM::flush_pipeline() {
    for (int i = 0; i < 2; i++) {
        if (is_arm()) {
            pipeline[i] = read_word(regs.r15);
            regs.r15 += 4;
        } else {
            pipeline[i] = read_halfword(regs.r15);
            regs.r15 += 2;
        }
    }
    // printf("[ARM] instruction to execute: %04x, instruction to decode: %04x\n", pipeline[0], pipeline[1]);
}

void ARM::debug_regs() {
    log_debug("[ARM] r0: %08x r1: %08x r2: %08x r3: %08x r4: %08x: r5: %08x r6: %08x: r7: %08x r8: %08x r9: %08x r10: %08x r11: %08x r12: %08x r13: %08x r14: %08x r15: %08x", regs.r0, regs.r1, regs.r2, regs.r3, regs.r4, regs.r5, regs.r6, regs.r7, regs.r8, regs.r9, regs.r10, regs.r11, regs.r12, regs.r13, regs.r14, regs.r15);
}



