#include <emulator/core/ARMInterpreter.h>
#include <emulator/Emulator.h>
#include <emulator/common/log.h>
#include <emulator/common/arithmetic.h>
#include <stdio.h>

ARMInterpreter::ARMInterpreter(Emulator *emulator, int cpu_id): emulator(emulator), cpu_id(cpu_id) {
    
    // #ifdef FILE_LOG
    
    // file = fopen("armwrestler2.log", "w");
    // #endif
}

// memory handlers
u8 ARMInterpreter::read_byte(u32 addr) {
    if (cpu_id == ARMv5) {
        return emulator->memory.arm9_read_byte(addr);
    }
    return emulator->memory.arm7_read_byte(addr);
}

u16 ARMInterpreter::read_halfword(u32 addr) {
    if (cpu_id == ARMv5) {
        return emulator->memory.arm9_read_halfword(addr);
    }
    return emulator->memory.arm7_read_halfword(addr);
}

u32 ARMInterpreter::read_word(u32 addr) {
    if (cpu_id == ARMv5) {
        return emulator->memory.arm9_read_word(addr);
    }
    return emulator->memory.arm7_read_word(addr);
}

void ARMInterpreter::write_byte(u32 addr, u8 data) {
    if (cpu_id == ARMv5) {
        emulator->memory.arm9_write_byte(addr, data);
    } else {
        emulator->memory.arm7_write_byte(addr, data);
    }
}

void ARMInterpreter::write_halfword(u32 addr, u16 data) {
    if (cpu_id == ARMv5) {
        emulator->memory.arm9_write_halfword(addr, data);
    } else {
        emulator->memory.arm7_write_halfword(addr, data);
    }
}

void ARMInterpreter::write_word(u32 addr, u32 data) {
    if (cpu_id == ARMv5) {
        emulator->memory.arm9_write_word(addr, data);
    } else {
        emulator->memory.arm7_write_word(addr, data);
    }
}

void ARMInterpreter::direct_boot() {
    // common between arm7 and arm9
    regs.r[0] = regs.r[1] = regs.r[2] = regs.r[3] = regs.r[4] = regs.r[5] = regs.r[6] = regs.r[7] = regs.r[8] = regs.r[9] = regs.r[10] = regs.r[11] = regs.r[12] = regs.r[14] = 0;

    regs.r8_fiq = regs.r9_fiq = regs.r10_fiq = regs.r11_fiq = regs.r12_fiq = regs.r14_fiq = regs.spsr_fiq = 0;
	regs.r14_svc = regs.spsr_svc = 0;
	regs.r14_abt = regs.spsr_abt = 0;
	regs.r14_irq = regs.spsr_irq = 0;
	regs.r14_und = regs.spsr_und = 0;
    // switch to system mode
    regs.cpsr = 0x0000001F;

    if (cpu_id == ARMv5) {
        // specific to arm9
        regs.r[13] = regs.r13_fiq = regs.r13_abt = regs.r13_und = 0x03002F7C;
        regs.r13_svc = 0x03003FC0;
        regs.r13_irq = 0x03003F80;
        regs.r[15] = emulator->cartridge.header.arm9_entry_address;
    } else {
        // arm7 specific
        regs.r[13] = regs.r13_fiq = regs.r13_abt = regs.r13_und = 0x0380FD80;
        regs.r13_svc = 0x0380FFC0;
        regs.r13_irq = 0x0380FF80;
        regs.r[15] = emulator->cartridge.header.arm7_entry_address;
    }
    log_debug("sucessfully initialised direct boot state");
}

void ARMInterpreter::reset() {
    // for now we will just direct boot
    direct_boot();
    flush_pipeline();
}


bool ARMInterpreter::is_arm() {
    return (get_bit(5, regs.cpsr) == 0);
}

void ARMInterpreter::flush_pipeline() {
    for (int i = 0; i < 2; i++) {
        if (is_arm()) {
            pipeline[i] = read_word(regs.r[15]);
            regs.r[15] += 4;
        } else {
            pipeline[i] = read_halfword(regs.r[15]);
            regs.r[15] += 2;
        }
    }
}

bool ARMInterpreter::get_condition_flag(int condition_flag) {
    return ((regs.cpsr & (1 << condition_flag)) != 0);
}

void ARMInterpreter::set_condition_flag(int condition_flag, bool data) {
    if (data) {
        regs.cpsr |= (1 << condition_flag);
    } else {
        regs.cpsr &= ~(1 << condition_flag);
    }
}

bool ARMInterpreter::condition_evaluate() {
    bool n_flag = get_condition_flag(N_FLAG);
    bool z_flag = get_condition_flag(Z_FLAG);
    bool c_flag = get_condition_flag(C_FLAG);
    bool v_flag = get_condition_flag(V_FLAG);
    switch (opcode >> 28) {
        case 0:
            return z_flag; // EQ 
        case 1:
            return !z_flag; // NE
        case 2:
            return c_flag; // CS
        case 3:
            return !c_flag; // CC
        case 4:
            return n_flag; // MI
        case 5:
            return !n_flag; // PL
        case 6:
            return v_flag; // VS
        case 7:
            return !v_flag; // VC
        case 8:
            return (c_flag && !z_flag); // HI
        case 9:
            return (!c_flag || z_flag); // LS
        case 10:
            return (n_flag == v_flag); // GE
        case 11:
            return (n_flag != v_flag); // LT
        case 12:
            return (!z_flag && (n_flag == v_flag)); // GT
        case 13:
            return (z_flag || (n_flag != v_flag)); // LE
        case 14:
            return true; // AL
        default:
            log_fatal("[ARM] condition code %d is not valid!", opcode >> 28);
    }
}

u32 ARMInterpreter::get_spsr() {
    u32 cpu_mode = get_bit_range(0, 4, regs.cpsr);
    switch (cpu_mode) {
    case FIQ:
        return regs.spsr_fiq;
    case SVC:
        return regs.spsr_svc;
    case ABT:
        return regs.spsr_abt;
    case IRQ:
        return regs.spsr_irq;
    case UND:
        return regs.spsr_und;
    default:
        log_fatal("[ARM] undefined psr read");
    }
}

// TODO: optmise this stuff
void ARMInterpreter::set_spsr(u32 data) {
    u32 cpu_mode = get_bit_range(0, 4, regs.cpsr);
    switch (cpu_mode) {
    case FIQ:
        regs.spsr_fiq = data;
        break;
    case SVC:
        regs.spsr_svc = data;
        break;
    case ABT:
        regs.spsr_abt = data;
        break;
    case IRQ:
        regs.spsr_irq = data;
        break;
    case UND:
        regs.spsr_und = data;
        break;
    default:
        log_fatal("[ARM] undefined psr write");
    }
    // TODO: probably want to investigate which bits are preserved
}

void ARMInterpreter::set_mode(u8 mode) {
    // not sure if i should load banked spsr into cpsr :thinking:
    log_warn("mode 0x%04x", mode);
    switch (mode & 0x1F) {
    case USR: case SYS:
        regs.r[8] = regs.r_usr[8];
        regs.r[9] = regs.r_usr[9];
        regs.r[10] = regs.r_usr[10];
        regs.r[11] = regs.r_usr[11];
        regs.r[12] = regs.r_usr[12];
        regs.r[13] = regs.r_usr[13];
        regs.r[14] = regs.r_usr[14];
        break;
    case FIQ:
        regs.r[8] = regs.r8_fiq;
        regs.r[9] = regs.r9_fiq;
        regs.r[10] = regs.r10_fiq;
        regs.r[11] = regs.r11_fiq;
        regs.r[12] = regs.r12_fiq;
        regs.r[13] = regs.r13_fiq;
        regs.r[14] = regs.r14_fiq;
        break;
    case SVC:
        regs.r[13] = regs.r13_svc;
        regs.r[14] = regs.r14_svc;
        break;
    case ABT:
        regs.r[13] = regs.r13_abt;
        regs.r[14] = regs.r14_abt;
        break;
    case IRQ:
        regs.r[13] = regs.r13_irq;
        regs.r[14] = regs.r14_irq;
        break;
    case UND:
        regs.r[13] = regs.r13_und;
        regs.r[14] = regs.r14_und;
        break;
    default:
        log_fatal("mode switch to mode 0x%04x has not been implemented yet", mode & 0x1F);
    }
    // finally actually set the mode in cpsr
    regs.cpsr = (regs.cpsr & ~0x1F) | (mode & 0x1F);
}

void ARMInterpreter::step() {
    // stepping the pipeline must happen before an instruction is executed incase the instruction is a branch which would flush and then step the pipeline (not correct)
    opcode = pipeline[0]; // store the current executing instruction 
    // shift the pipeline
    pipeline[0] = pipeline[1];
    // fill the 2nd item with the new instruction to be read
    if (is_arm()) {
        pipeline[1] = read_word(regs.r[15]);
    } else {
        log_fatal("still need to implement thumb");
        // pipeline[1] = read_halfword(regs.r15);
    }
    execute_instruction();
}

void ARMInterpreter::execute_instruction() {
    // we will return the function results in void function as we dont have to type break then lmao
    
    if (is_arm()) {
        // using http://imrannazar.com/ARM-Opcode-Map
        if (condition_evaluate()) {
            u32 index = ((opcode >> 16) & 0xFF0) | ((opcode >> 4) & 0xF);
            switch (index) {
            case 0x012: case 0x01A:
                return ands(lris());
            case 0x014: case 0x01C:
                return ands(aris());
            case 0x019:
                return muls();
            case 0x020: case 0x028:
                return eor(lli());
            case 0x030: case 0x038:
                return eors(llis());
            case 0x39:
                return mlas();
            case 0x040: case 0x048:
                return sub(lli());
            case 0x050: case 0x058:
                return subs(lli());
            case 0x080: case 0x088:
                return add(lli());
            case 0x090: case 0x098:
                return adds(lli());
            case 0x099:
                return umulls();
            case 0x0A0: case 0x0A8:
                return adc(lli());
            case 0x0A2: case 0x0AA:
                return adc(lri());
            case 0x0B0: case 0x0B8:
                return adcs(lli());
            case 0x0CB: case 0x0EB:
                return strh_post(imm_halfword_signed_data_transfer());
            case 0x0D0: case 0x0D8:
                return sbcs(lli());
            case 0x0D9:
                return smulls();
            case 0x0F0: case 0x0F8:
                return rscs(lli());
            case 0x120:
                return msr_reg();
            case 0x121:
                return bx();
            case 0x150: case 0x158:
                return cmps(lli());
            case 0x15B:
                return ldrh_pre(-imm_halfword_signed_data_transfer());
            case 0x170: case 0x178:
                return cmns(lli());
            case 0x17B:
                return ldrh_pre(-imm_halfword_signed_data_transfer());
            case 0x196: case 0x19E:
                return orrs(rris());
            case 0x19B:
                return ldrh_pre(reg_halfword_signed_data_transfer());
            case 0x1A0: case 0x1A8:
                return mov(lli());
            case 0x1A2: case 0x1AA:
                return mov(lri());
            case 0x1A6: case 0x1AE:
                return mov(rri());
            case 0x1B1:
                return movs(llrs());
            case 0x1B2: case 0x1BA:
                return movs(lris());
            case 0x1B5:
                return movs(arrs());
            case 0x1CB:
                return strh_pre(imm_halfword_signed_data_transfer());
            case 0x1D0: case 0x1D8:
                return bics(llis());
            case 0x1D4: case 0x1DC:
                return bics(aris());
            case 0x1DB:
                return ldrh_pre(imm_halfword_signed_data_transfer());
            case 0x1E0: case 0x1E8:
                return mvn(lli());
            case 0x1FB:
                return ldrh_pre(imm_halfword_signed_data_transfer());
            case 0x200: case 0x201: case 0x202: case 0x203: 
            case 0x204: case 0x205: case 0x206: case 0x207: 
            case 0x208: case 0x209: case 0x20A: case 0x20B: 
            case 0x20C: case 0x20D: case 0x20E: case 0x20F:
                return _and(imm_data_processing());
            case 0x210: case 0x211: case 0x212: case 0x213: 
            case 0x214: case 0x215: case 0x216: case 0x217: 
            case 0x218: case 0x219: case 0x21A: case 0x21B: 
            case 0x21C: case 0x21D: case 0x21E: case 0x21F:
                return ands(imms_data_processing());
            case 0x220: case 0x221: case 0x222: case 0x223: 
            case 0x224: case 0x225: case 0x226: case 0x227: 
            case 0x228: case 0x229: case 0x22A: case 0x22B: 
            case 0x22C: case 0x22D: case 0x22E: case 0x22F:
                return eor(imm_data_processing());
            case 0x240: case 0x241: case 0x242: case 0x243: 
            case 0x244: case 0x245: case 0x246: case 0x247: 
            case 0x248: case 0x249: case 0x24A: case 0x24B: 
            case 0x24C: case 0x24D: case 0x24E: case 0x24F:
                return sub(imm_data_processing());
            case 0x250: case 0x251: case 0x252: case 0x253: 
            case 0x254: case 0x255: case 0x256: case 0x257: 
            case 0x258: case 0x259: case 0x25A: case 0x25B: 
            case 0x25C: case 0x25D: case 0x25E: case 0x25F:
                return subs(imms_data_processing());
            case 0x280: case 0x281: case 0x282: case 0x283: 
            case 0x284: case 0x285: case 0x286: case 0x287: 
            case 0x288: case 0x289: case 0x28A: case 0x28B: 
            case 0x28C: case 0x28D: case 0x28E: case 0x28F:
                return add(imm_data_processing());
            case 0x2D0: case 0x2D1: case 0x2D2: case 0x2D3: 
            case 0x2D4: case 0x2D5: case 0x2D6: case 0x2D7: 
            case 0x2D8: case 0x2D9: case 0x2DA: case 0x2DB: 
            case 0x2DC: case 0x2DD: case 0x2DE: case 0x2DF:
                return sbcs(imms_data_processing());
            case 0x310: case 0x311: case 0x312: case 0x313: 
            case 0x314: case 0x315: case 0x316: case 0x317: 
            case 0x318: case 0x319: case 0x31A: case 0x31B: 
            case 0x31C: case 0x31D: case 0x31E: case 0x31F:
                return tsts(imms_data_processing());
            case 0x350: case 0x351: case 0x352: case 0x353: 
            case 0x354: case 0x355: case 0x356: case 0x357: 
            case 0x358: case 0x359: case 0x35A: case 0x35B: 
            case 0x35C: case 0x35D: case 0x35E: case 0x35F:
                return cmps(imms_data_processing());
            case 0x380: case 0x381: case 0x382: case 0x383: 
            case 0x384: case 0x385: case 0x386: case 0x387: 
            case 0x388: case 0x389: case 0x38A: case 0x38B: 
            case 0x38C: case 0x38D: case 0x38E: case 0x38F:
                return orr(imm_data_processing());
            case 0x3A0: case 0x3A1: case 0x3A2: case 0x3A3: 
            case 0x3A4: case 0x3A5: case 0x3A6: case 0x3A7: 
            case 0x3A8: case 0x3A9: case 0x3AA: case 0x3AB: 
            case 0x3AC: case 0x3AD: case 0x3AE: case 0x3AF:
                return mov(imm_data_processing());
            case 0x3E0: case 0x3E1: case 0x3E2: case 0x3E3: 
            case 0x3E4: case 0x3E5: case 0x3E6: case 0x3E7: 
            case 0x3E8: case 0x3E9: case 0x3EA: case 0x3EB: 
            case 0x3EC: case 0x3ED: case 0x3EE: case 0x3EF:
                return mvn(imm_data_processing());
            case 0x400: case 0x401: case 0x402: case 0x403: 
            case 0x404: case 0x405: case 0x406: case 0x407: 
            case 0x408: case 0x409: case 0x40A: case 0x40B: 
            case 0x40C: case 0x40D: case 0x40E: case 0x40F:
                return str_post(-imm_single_data_transfer());
            case 0x410: case 0x411: case 0x412: case 0x413: 
            case 0x414: case 0x415: case 0x416: case 0x417: 
            case 0x418: case 0x419: case 0x41A: case 0x41B: 
            case 0x41C: case 0x41D: case 0x41E: case 0x41F:
                return ldr_post(-imm_single_data_transfer());
            case 0x480: case 0x481: case 0x482: case 0x483: 
            case 0x484: case 0x485: case 0x486: case 0x487: 
            case 0x488: case 0x489: case 0x48A: case 0x48B: 
            case 0x48C: case 0x48D: case 0x48E: case 0x48F:
                return str_post(imm_single_data_transfer());
            case 0x490: case 0x491: case 0x492: case 0x493: 
            case 0x494: case 0x495: case 0x496: case 0x497: 
            case 0x498: case 0x499: case 0x49A: case 0x49B: 
            case 0x49C: case 0x49D: case 0x49E: case 0x49F:
                return ldr_post(imm_single_data_transfer());
            case 0x4C0: case 0x4C1: case 0x4C2: case 0x4C3: 
            case 0x4C4: case 0x4C5: case 0x4C6: case 0x4C7: 
            case 0x4C8: case 0x4C9: case 0x4CA: case 0x4CB: 
            case 0x4CC: case 0x4CD: case 0x4CE: case 0x4CF:
                return strb_post(imm_single_data_transfer());
            case 0x4D0: case 0x4D1: case 0x4D2: case 0x4D3: 
            case 0x4D4: case 0x4D5: case 0x4D6: case 0x4D7: 
            case 0x4D8: case 0x4D9: case 0x4DA: case 0x4DB: 
            case 0x4DC: case 0x4DD: case 0x4DE: case 0x4DF:
                return ldrb_post(imm_single_data_transfer());
            case 0x510: case 0x511: case 0x512: case 0x513:
            case 0x514: case 0x515: case 0x516: case 0x517:
            case 0x518: case 0x519: case 0x51A: case 0x51B:
            case 0x51C: case 0x51D: case 0x51E: case 0x51F:
                return ldr_pre(-imm_single_data_transfer());
            case 0x530: case 0x531: case 0x532: case 0x533:
            case 0x534: case 0x535: case 0x536: case 0x537:
            case 0x538: case 0x539: case 0x53A: case 0x53B:
            case 0x53C: case 0x53D: case 0x53E: case 0x53F:
                // TODO: later separate ldr and str into separate instructions where if statement is not required for writeback check
                return ldr_pre(-imm_single_data_transfer());
            case 0x580: case 0x581: case 0x582: case 0x583:
            case 0x584: case 0x585: case 0x586: case 0x587:
            case 0x588: case 0x589: case 0x58A: case 0x58B:
            case 0x58C: case 0x58D: case 0x58E: case 0x58F:
                return str_pre(imm_single_data_transfer());
            case 0x590: case 0x591: case 0x592: case 0x593:
            case 0x594: case 0x595: case 0x596: case 0x597:
            case 0x598: case 0x599: case 0x59A: case 0x59B:
            case 0x59C: case 0x59D: case 0x59E: case 0x59F:
                return ldr_pre(imm_single_data_transfer());
            case 0x5B0: case 0x5B1: case 0x5B2: case 0x5B3:
            case 0x5B4: case 0x5B5: case 0x5B6: case 0x5B7:
            case 0x5B8: case 0x5B9: case 0x5BA: case 0x5BB:
            case 0x5BC: case 0x5BD: case 0x5BE: case 0x5BF:
                // TODO: later separate ldr and str into separate instructions where if statement is not required for writeback check
                return ldr_pre(imm_single_data_transfer());
            case 0x5C0: case 0x5C1: case 0x5C2: case 0x5C3: 
            case 0x5C4: case 0x5C5: case 0x5C6: case 0x5C7: 
            case 0x5C8: case 0x5C9: case 0x5CA: case 0x5CB: 
            case 0x5CC: case 0x5CD: case 0x5CE: case 0x5CF:
                return strb_pre(imm_single_data_transfer());
            case 0x5D0: case 0x5D1: case 0x5D2: case 0x5D3: 
            case 0x5D4: case 0x5D5: case 0x5D6: case 0x5D7: 
            case 0x5D8: case 0x5D9: case 0x5DA: case 0x5DB: 
            case 0x5DC: case 0x5DD: case 0x5DE: case 0x5DF:
                return ldrb_pre(imm_single_data_transfer());
            case 0x610: case 0x618:
                return ldr_post(-rpll());
            case 0x612: case 0x61A:
                return ldr_post(-rplr());
            case 0x614: case 0x61C:
                return ldr_post(-rpar());
            case 0x616: case 0x61E:
                return ldr_post(-rprr());
            case 0x690: case 0x698:
                return ldr_post(rpll());
            case 0x692: case 0x69A:
                return ldr_post(rplr());
            case 0x694: case 0x69C:
                return ldr_post(rpar());
            case 0x696: case 0x69E:
                return ldr_post(rprr());
            case 0x710: case 0x718:
                return ldr_pre(-rpll());
            case 0x712: case 0x71A:
                return ldr_pre(-rplr());
            case 0x714: case 0x71C:
                return ldr_pre(-rpar());
            case 0x716: case 0x71E:
                return ldr_pre(-rprr());
            case 0x730: case 0x738:
                // TODO: split into more functions later
                return ldr_pre(-rpll());
            case 0x732: case 0x73A:
                return ldr_pre(-rplr());
            case 0x734: case 0x73C:
                return ldr_pre(-rpar());
            case 0x736: case 0x73E:
                return ldr_pre(-rprr());
            case 0x790: case 0x798:
                return ldr_pre(rpll());
            case 0x792: case 0x79A:
                // TODO: check again later
                return ldr_pre(rplr());
            case 0x794: case 0x79C:
                return ldr_pre(rpar());
            case 0x796: case 0x79E:
                return ldr_pre(rprr());
            case 0x7B0: case 0x7B8:
                return ldr_pre(rpll());
            case 0x7B2: case 0x7BA:
                return ldr_pre(rplr());
            case 0x7B4: case 0x7BC:
                return ldr_pre(rpar());
            case 0x7B6: case 0x7BE:
                return ldr_pre(rprr());
            case 0x820: case 0x821: case 0x822: case 0x823:
            case 0x824: case 0x825: case 0x826: case 0x827:
            case 0x828: case 0x829: case 0x82A: case 0x82B:
            case 0x82C: case 0x82D: case 0x82E: case 0x82F:
                return stmdaw();
            case 0x830: case 0x831: case 0x832: case 0x833:
            case 0x834: case 0x835: case 0x836: case 0x837:
            case 0x838: case 0x839: case 0x83A: case 0x83B:
            case 0x83C: case 0x83D: case 0x83E: case 0x83F:
                return ldmdaw();
            case 0x870: case 0x871: case 0x872: case 0x873:
            case 0x874: case 0x875: case 0x876: case 0x877:
            case 0x878: case 0x879: case 0x87A: case 0x87B:
            case 0x87C: case 0x87D: case 0x87E: case 0x87F:
                // TODO: make more accurate later
                return ldmdaw();
            case 0x8A0: case 0x8A1: case 0x8A2: case 0x8A3:
            case 0x8A4: case 0x8A5: case 0x8A6: case 0x8A7:
            case 0x8A8: case 0x8A9: case 0x8AA: case 0x8AB:
            case 0x8AC: case 0x8AD: case 0x8AE: case 0x8AF:
                return stmiaw();
            case 0x8B0: case 0x8B1: case 0x8B2: case 0x8B3:
            case 0x8B4: case 0x8B5: case 0x8B6: case 0x8B7:
            case 0x8B8: case 0x8B9: case 0x8BA: case 0x8BB:
            case 0x8BC: case 0x8BD: case 0x8BE: case 0x8BF:
                return ldmiaw();
            case 0x8F0: case 0x8F1: case 0x8F2: case 0x8F3:
            case 0x8F4: case 0x8F5: case 0x8F6: case 0x8F7:
            case 0x8F8: case 0x8F9: case 0x8FA: case 0x8FB:
            case 0x8FC: case 0x8FD: case 0x8FE: case 0x8FF:
                // TODO: make more accurate later
                return ldmiaw();
            case 0x920: case 0x921: case 0x922: case 0x923:
            case 0x924: case 0x925: case 0x926: case 0x927:
            case 0x928: case 0x929: case 0x92A: case 0x92B:
            case 0x92C: case 0x92D: case 0x92E: case 0x92F:
                return stmdbw();
            case 0x930: case 0x931: case 0x932: case 0x933:
            case 0x934: case 0x935: case 0x936: case 0x937:
            case 0x938: case 0x939: case 0x93A: case 0x93B:
            case 0x93C: case 0x93D: case 0x93E: case 0x93F:
                return ldmdbw();
            case 0x970: case 0x971: case 0x972: case 0x973:
            case 0x974: case 0x975: case 0x976: case 0x977:
            case 0x978: case 0x979: case 0x97A: case 0x97B:
            case 0x97C: case 0x97D: case 0x97E: case 0x97F:
                // TODO: make more accurate later
                return ldmdbw();
            case 0x9A0: case 0x9A1: case 0x9A2: case 0x9A3:
            case 0x9A4: case 0x9A5: case 0x9A6: case 0x9A7:
            case 0x9A8: case 0x9A9: case 0x9AA: case 0x9AB:
            case 0x9AC: case 0x9AD: case 0x9AE: case 0x9AF:
                return stmibw();
            case 0x9B0: case 0x9B1: case 0x9B2: case 0x9B3:
            case 0x9B4: case 0x9B5: case 0x9B6: case 0x9B7:
            case 0x9B8: case 0x9B9: case 0x9BA: case 0x9BB:
            case 0x9BC: case 0x9BD: case 0x9BE: case 0x9BF:
                return ldmibw();
            case 0x9F0: case 0x9F1: case 0x9F2: case 0x9F3:
            case 0x9F4: case 0x9F5: case 0x9F6: case 0x9F7:
            case 0x9F8: case 0x9F9: case 0x9FA: case 0x9FB:
            case 0x9FC: case 0x9FD: case 0x9FE: case 0x9FF:
                // TODO: make this more accurate later by using uw correctly
                return ldmibw();
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
                return b();
            case 0xB00: case 0xB01: case 0xB02: case 0xB03:
            case 0xB04: case 0xB05: case 0xB06: case 0xB07:
            case 0xB08: case 0xB09: case 0xB0A: case 0xB0B:
            case 0xB0C: case 0xB0D: case 0xB0E: case 0xB0F:
            case 0xB10: case 0xB11: case 0xB12: case 0xB13:
            case 0xB14: case 0xB15: case 0xB16: case 0xB17:
            case 0xB18: case 0xB19: case 0xB1A: case 0xB1B:
            case 0xB1C: case 0xB1D: case 0xB1E: case 0xB1F:
            case 0xB20: case 0xB21: case 0xB22: case 0xB23:
            case 0xB24: case 0xB25: case 0xB26: case 0xB27:
            case 0xB28: case 0xB29: case 0xB2A: case 0xB2B:
            case 0xB2C: case 0xB2D: case 0xB2E: case 0xB2F:
            case 0xB30: case 0xB31: case 0xB32: case 0xB33:
            case 0xB34: case 0xB35: case 0xB36: case 0xB37:
            case 0xB38: case 0xB39: case 0xB3A: case 0xB3B:
            case 0xB3C: case 0xB3D: case 0xB3E: case 0xB3F:
            case 0xB40: case 0xB41: case 0xB42: case 0xB43:
            case 0xB44: case 0xB45: case 0xB46: case 0xB47:
            case 0xB48: case 0xB49: case 0xB4A: case 0xB4B:
            case 0xB4C: case 0xB4D: case 0xB4E: case 0xB4F:
            case 0xB50: case 0xB51: case 0xB52: case 0xB53:
            case 0xB54: case 0xB55: case 0xB56: case 0xB57:
            case 0xB58: case 0xB59: case 0xB5A: case 0xB5B:
            case 0xB5C: case 0xB5D: case 0xB5E: case 0xB5F:
            case 0xB60: case 0xB61: case 0xB62: case 0xB63:
            case 0xB64: case 0xB65: case 0xB66: case 0xB67:
            case 0xB68: case 0xB69: case 0xB6A: case 0xB6B:
            case 0xB6C: case 0xB6D: case 0xB6E: case 0xB6F:
            case 0xB70: case 0xB71: case 0xB72: case 0xB73:
            case 0xB74: case 0xB75: case 0xB76: case 0xB77:
            case 0xB78: case 0xB79: case 0xB7A: case 0xB7B:
            case 0xB7C: case 0xB7D: case 0xB7E: case 0xB7F:
            case 0xB80: case 0xB81: case 0xB82: case 0xB83:
            case 0xB84: case 0xB85: case 0xB86: case 0xB87:
            case 0xB88: case 0xB89: case 0xB8A: case 0xB8B:
            case 0xB8C: case 0xB8D: case 0xB8E: case 0xB8F:
            case 0xB90: case 0xB91: case 0xB92: case 0xB93:
            case 0xB94: case 0xB95: case 0xB96: case 0xB97:
            case 0xB98: case 0xB99: case 0xB9A: case 0xB9B:
            case 0xB9C: case 0xB9D: case 0xB9E: case 0xB9F:
            case 0xBA0: case 0xBA1: case 0xBA2: case 0xBA3:
            case 0xBA4: case 0xBA5: case 0xBA6: case 0xBA7:
            case 0xBA8: case 0xBA9: case 0xBAA: case 0xBAB:
            case 0xBAC: case 0xBAD: case 0xBAE: case 0xBAF:
            case 0xBB0: case 0xBB1: case 0xBB2: case 0xBB3:
            case 0xBB4: case 0xBB5: case 0xBB6: case 0xBB7:
            case 0xBB8: case 0xBB9: case 0xBBA: case 0xBBB:
            case 0xBBC: case 0xBBD: case 0xBBE: case 0xBBF:
            case 0xBC0: case 0xBC1: case 0xBC2: case 0xBC3:
            case 0xBC4: case 0xBC5: case 0xBC6: case 0xBC7:
            case 0xBC8: case 0xBC9: case 0xBCA: case 0xBCB:
            case 0xBCC: case 0xBCD: case 0xBCE: case 0xBCF:
            case 0xBD0: case 0xBD1: case 0xBD2: case 0xBD3:
            case 0xBD4: case 0xBD5: case 0xBD6: case 0xBD7:
            case 0xBD8: case 0xBD9: case 0xBDA: case 0xBDB:
            case 0xBDC: case 0xBDD: case 0xBDE: case 0xBDF:
            case 0xBE0: case 0xBE1: case 0xBE2: case 0xBE3:
            case 0xBE4: case 0xBE5: case 0xBE6: case 0xBE7:
            case 0xBE8: case 0xBE9: case 0xBEA: case 0xBEB:
            case 0xBEC: case 0xBED: case 0xBEE: case 0xBEF:
            case 0xBF0: case 0xBF1: case 0xBF2: case 0xBF3:
            case 0xBF4: case 0xBF5: case 0xBF6: case 0xBF7:
            case 0xBF8: case 0xBF9: case 0xBFA: case 0xBFB:
            case 0xBFC: case 0xBFD: case 0xBFE: case 0xBFF:
                return bl();
            default:
                log_fatal("opcode 0x%08x is unimplemented with identifier 0x%03x", opcode, index);
            }
        } else {
            regs.r[15] += 4;
        }
    } else {
        log_fatal("thumb instruction set not implemented yet");
    }
}
