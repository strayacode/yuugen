#include <emulator/core/arm.h>
#include <emulator/emulator.h>
#include <emulator/common/arithmetic.h>
#include <emulator/common/types.h>

ARM::ARM(Emulator *emulator, int cpu_id): emulator(emulator), cpu_id(cpu_id) {

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
    printf("init pipeline %d\n", cpu_id);
    if (!cpu_id) {
        printf("not good\n");
        return emulator->memory.arm7_read_word(addr);
    }
    return emulator->memory.arm9_read_word(addr);
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
        printf("[ARM] undefined registers access r%d\n", reg);
        return 0;
    }
}

void ARM::execute_instruction() {
    // using http://imrannazar.com/ARM-Opcode-Map
    printf("%04x\n", regs.r15);
    if (is_arm()) {
        u32 index = ((opcode >> 16) & 0xFF0) | ((opcode >> 4) & 0xF);
        // execute arm instruction
        switch (index) {
            // case 0x350: case 0x351: case 0x352: case 0x353:
            // case 0x354: case 0x355: case 0x356: case 0x357:
            // case 0x358: case 0x359: case 0x35A: case 0x35B:
            // case 0x35C: case 0x35D: case 0x35E: case 0x35F:
            //     // cmp(); break;

            // execute arm instruction
            case 0xA00: case 0xA01: case 0xA02: case 0xA03:
            case 0xA04: case 0xA05: case 0xA06: case 0xA07:
            case 0xA08: case 0xA09: case 0xA0A: case 0xA0B:
            case 0xA0C: case 0xA0D: case 0xA0E: case 0xA0F:
            case 0xA10: case 0xA11: case 0xA12: case 0xA13:
            case 0xA14: case 0xA15: case 0xA16: case 0xA17:
            case 0xA18: case 0xA19: case 0xA1A: case 0xA1B:
            case 0xA1C: case 0xA1D: case 0xA1E: case 0xA1F:
            case 0xA20: case 0xA21: case 0xA22: case 0xA23:
            case 0xA24: case 0xA25: case 0xA26: case 0xA27:
            case 0xA28: case 0xA29: case 0xA2A: case 0xA2B:
            case 0xA2C: case 0xA2D: case 0xA2E: case 0xA2F:
            case 0xA30: case 0xA31: case 0xA32: case 0xA33:
            case 0xA34: case 0xA35: case 0xA36: case 0xA37:
            case 0xA38: case 0xA39: case 0xA3A: case 0xA3B:
            case 0xA3C: case 0xA3D: case 0xA3E: case 0xA3F:
            case 0xA40: case 0xA41: case 0xA42: case 0xA43:
            case 0xA44: case 0xA45: case 0xA46: case 0xA47:
            case 0xA48: case 0xA49: case 0xA4A: case 0xA4B:
            case 0xA4C: case 0xA4D: case 0xA4E: case 0xA4F:
            case 0xA50: case 0xA51: case 0xA52: case 0xA53:
            case 0xA54: case 0xA55: case 0xA56: case 0xA57:
            case 0xA58: case 0xA59: case 0xA5A: case 0xA5B:
            case 0xA5C: case 0xA5D: case 0xA5E: case 0xA5F:
            case 0xA60: case 0xA61: case 0xA62: case 0xA63:
            case 0xA64: case 0xA65: case 0xA66: case 0xA67:
            case 0xA68: case 0xA69: case 0xA6A: case 0xA6B:
            case 0xA6C: case 0xA6D: case 0xA6E: case 0xA6F:
            case 0xA70: case 0xA71: case 0xA72: case 0xA73:
            case 0xA74: case 0xA75: case 0xA76: case 0xA77:
            case 0xA78: case 0xA79: case 0xA7A: case 0xA7B:
            case 0xA7C: case 0xA7D: case 0xA7E: case 0xA7F:
            case 0xA80: case 0xA81: case 0xA82: case 0xA83:
            case 0xA84: case 0xA85: case 0xA86: case 0xA87:
            case 0xA88: case 0xA89: case 0xA8A: case 0xA8B:
            case 0xA8C: case 0xA8D: case 0xA8E: case 0xA8F:
            case 0xA90: case 0xA91: case 0xA92: case 0xA93:
            case 0xA94: case 0xA95: case 0xA96: case 0xA97:
            case 0xA98: case 0xA99: case 0xA9A: case 0xA9B:
            case 0xA9C: case 0xA9D: case 0xA9E: case 0xA9F:
            case 0xAA0: case 0xAA1: case 0xAA2: case 0xAA3:
            case 0xAA4: case 0xAA5: case 0xAA6: case 0xAA7:
            case 0xAA8: case 0xAA9: case 0xAAA: case 0xAAB:
            case 0xAAC: case 0xAAD: case 0xAAE: case 0xAAF:
            case 0xAB0: case 0xAB1: case 0xAB2: case 0xAB3:
            case 0xAB4: case 0xAB5: case 0xAB6: case 0xAB7:
            case 0xAB8: case 0xAB9: case 0xABA: case 0xABB:
            case 0xABC: case 0xABD: case 0xABE: case 0xABF:
            case 0xAC0: case 0xAC1: case 0xAC2: case 0xAC3:
            case 0xAC4: case 0xAC5: case 0xAC6: case 0xAC7:
            case 0xAC8: case 0xAC9: case 0xACA: case 0xACB:
            case 0xACC: case 0xACD: case 0xACE: case 0xACF:
            case 0xAD0: case 0xAD1: case 0xAD2: case 0xAD3:
            case 0xAD4: case 0xAD5: case 0xAD6: case 0xAD7:
            case 0xAD8: case 0xAD9: case 0xADA: case 0xADB:
            case 0xADC: case 0xADD: case 0xADE: case 0xADF:
            case 0xAE0: case 0xAE1: case 0xAE2: case 0xAE3:
            case 0xAE4: case 0xAE5: case 0xAE6: case 0xAE7:
            case 0xAE8: case 0xAE9: case 0xAEA: case 0xAEB:
            case 0xAEC: case 0xAED: case 0xAEE: case 0xAEF:
            case 0xAF0: case 0xAF1: case 0xAF2: case 0xAF3:
            case 0xAF4: case 0xAF5: case 0xAF6: case 0xAF7:
            case 0xAF8: case 0xAF9: case 0xAFA: case 0xAFB:
            case 0xAFC: case 0xAFD: case 0xAFE: case 0xAFF:
                b(); break;
        
        default:
            printf("[ARM] undefined arm instruction 0x%04x with identifier 0x%03x\n", opcode, index);
            emulator->running = false;
            break;
        }
        // no need to use get_reg() and set_reg() since pc is not a banked register
        regs.r15 += 4;
    } else {
        // execute thumb instruction
        // no need to use get_reg() and set_reg() since pc is not a banked register
        regs.r15 += 2;
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
        regs.cpsr = 0x0000005F;

    } else {
        // specific to arm9
        regs.r13 = regs.r13_fiq = regs.r13_abt = regs.r13_und = 0x03002F7C;
        regs.r13_svc = 0x03003FC0;
        regs.r13_irq = 0x03003F80;
        regs.r15 = emulator->cartridge.header.arm9_entry_address;
        regs.cpsr = 0x0000005F;
    }
    
    printf("[ARM] successfully initialised direct boot state\n");
    
}

void ARM::firmware_boot() {
    regs.r15 = 0xFFFF0000;
}

void ARM::reset() {
    // for now we will just direct boot
    // do cartridge part first so we can change r15 to entry_address accordingly
    emulator->cartridge.direct_boot();
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
        // pipeline[1] = read_halfword(regs.r15);
    }
    
    execute_instruction();
}

bool ARM::is_arm() {
    return (get_bit(5, regs.cpsr) == 0);
}

bool ARM::get_condition_flag(int condition_flag) {
    return ((regs.cpsr & (1 << condition_flag)) != 0);
}

bool ARM::evaluate_condition() {
    bool n_flag = get_condition_flag(N_FLAG);
    bool z_flag = get_condition_flag(Z_FLAG);
    bool c_flag = get_condition_flag(C_FLAG);
    bool v_flag = get_condition_flag(V_FLAG);
    switch (pipeline[0] >> 28) {
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
    printf("[ARM] instruction to execute: %04x, instruction to decode: %04x\n", pipeline[0], pipeline[1]);
}

