#include <emulator/common/types.h>
#include <emulator/common/log.h>
#include <emulator/common/arithmetic.h>
#include <emulator/Emulator.h>

void ARMInterpreter::mov(u32 op2) {
    // get rd
    u8 rd = (opcode >> 12) & 0xF;
    regs.r[rd] = op2;

    

    if (rd == 15) {
        // log_debug("we are now at r15 = 0x%04x", regs.r[15]);
        flush_pipeline(); // shrug idk what im doing lmao
        // log_warn("the behaviour rd = 15 might be handled incorrectly in mov");
    } else {
        regs.r[15] += 4;
    }
    
}

void ARMInterpreter::movs(u32 op2) {
    // get rd
    u8 rd = (opcode >> 12) & 0xF;
    
    set_condition_flag(Z_FLAG, op2 == 0);
	set_condition_flag(N_FLAG, op2 >> 31);
    regs.r[rd] = op2;
    if (rd == 15) {
        regs.cpsr = get_spsr();
        flush_pipeline(); // shrug idk what im doing lmao
    } else {
        regs.r[15] += 4;
    }
    
}

void ARMInterpreter::mvn(u32 op2) {
    // get rd
    u8 rd = (opcode >> 12) & 0xF;
    regs.r[rd] = ~op2;

    if (rd == 15) {
        regs.cpsr = get_spsr();
        flush_pipeline(); // shrug idk what im doing lmao
    } else {
        regs.r[15] += 4;
    }
    
}

u32 ARMInterpreter::imm_data_processing() {
    u32 immediate = opcode & 0xFF;
    u8 shift_amount = ((opcode >> 8) & 0xF) * 2;

    u32 result = (immediate >> shift_amount) | (immediate << (32 - shift_amount));
    return result;
}

u32 ARMInterpreter::imms_data_processing() {
    u32 immediate = opcode & 0xFF;
    u8 shift_amount = ((opcode >> 8) & 0xF) * 2;
    
    u32 result = (immediate >> shift_amount) | (immediate << (32 - shift_amount));
    if (shift_amount != 0) {
        set_condition_flag(C_FLAG, result >> 31);
    }
    return result;
}

void ARMInterpreter::subs(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    u32 result = regs.r[rn] - op2;
    // log_debug("result: %04x", result);

    regs.r[rd] = result;

    if (rd == 15) {
        log_fatal("handle pls");
        // copy the spsr of the current mode to the cpsr
        regs.cpsr = get_spsr();
    } else {
        set_condition_flag(N_FLAG, result >> 31);
        set_condition_flag(Z_FLAG, result == 0);
        set_condition_flag(C_FLAG, op2 >= regs.r[rn]);
        set_condition_flag(V_FLAG, (~(regs.r[rn] ^ op2) & (op2 ^ result)) >> 31);
    }

    regs.r[15] += 4;
}

void ARMInterpreter::sub(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    u32 result = regs.r[rn] - op2;

    regs.r[rd] = result;
    
    if (rd == 15) {
        log_fatal("handle pls");
    }

    regs.r[15] += 4;
}

void ARMInterpreter::cmps(u32 op2) {
    u8 rn = (opcode >> 16) & 0xF;
    
    u32 result = regs.r[rn] - op2;

    set_condition_flag(N_FLAG, result >> 31);
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(C_FLAG, op2 >= regs.r[rn]);
    set_condition_flag(V_FLAG, ((regs.r[rn] ^ op2) & (regs.r[rn] ^ result)) >> 31);

    regs.r[15] += 4;
}

void ARMInterpreter::bics(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] & ~op2;
    regs.r[rd] = result;
    if (rd == 15) {
        // copy the spsr of the current mode to the cpsr
        log_fatal("shit");
        regs.cpsr = get_spsr();
    } else {
        set_condition_flag(N_FLAG, result >> 31);
        set_condition_flag(Z_FLAG, result == 0);
        // C flag is set by carry out of shifter
    }

    regs.r[15] += 4;
}

void ARMInterpreter::bic(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] & ~op2;
    regs.r[rd] = result;
    if (rd == 15) {
        // copy the spsr of the current mode to the cpsr
        log_fatal("shit");
        regs.cpsr = get_spsr();
    } 

    regs.r[15] += 4;
}

void ARMInterpreter::add(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    regs.r[rd] = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle pls");
    }

    regs.r[15] += 4;
    
}



void ARMInterpreter::ands(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] & op2;
    if (rd == 15) {
        log_fatal("handle pls");
        regs.cpsr = get_spsr();
    } else {
        set_condition_flag(N_FLAG, result >> 31);
        set_condition_flag(Z_FLAG, result == 0);
    }

    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::_and(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] & op2;
    if (rd == 15) {
        log_fatal("handle pls");
        regs.cpsr = get_spsr();
    } 

    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::eor(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] ^ op2;
    if (rd == 15) {
        log_fatal("handle pls");
    }

    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::eors(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] ^ op2;
    if (rd == 15) {
        log_fatal("handle pls");
    }
    set_condition_flag(Z_FLAG, result == 0);
	set_condition_flag(N_FLAG, result >> 31);
    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::tsts(u32 op2) {
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] & op2;
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(N_FLAG, result >> 31);

    regs.r[15] += 4;
}

void ARMInterpreter::adds(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u64 result64 = (u64)regs.r[rn] + (u64)op2;
    u32 result32 = (u32)result64;
    regs.r[rd] = result32;
    set_condition_flag(Z_FLAG, result32 == 0);
    set_condition_flag(N_FLAG, result32 >> 31);
    // detect borrow from bit 32
    set_condition_flag(C_FLAG, result64 >> 32);
    // understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
    set_condition_flag(V_FLAG, (~(regs.r[rn] ^ op2) & (op2 ^ result32)) >> 31);
    if (rd == 15) {
        log_fatal("handle pls");
    }

    regs.r[15] += 4;
}

void ARMInterpreter::adcs(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u64 result = (u64)regs.r[rn] + (u64)op2 + (u64)get_condition_flag(C_FLAG);
	u32 result32 = (u32)result;
    set_condition_flag(C_FLAG, result >> 32);

    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(N_FLAG, result32 >> 31);

    set_condition_flag(V_FLAG, (~(regs.r[rn] ^ op2) & (op2 ^ result32)) >> 31);
    regs.r[rd] = result32;

    regs.r[15] += 4;
}

void ARMInterpreter::adc(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] + op2 + get_condition_flag(C_FLAG);
    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::orr(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    regs.r[rd] = regs.r[rn] | op2;

    if (rd == 15) {
        log_fatal("handle pls");
        regs.cpsr = get_spsr();
    } 

    

    regs.r[15] += 4;
}

void ARMInterpreter::orrs(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    regs.r[rd] = regs.r[rn] | op2;
    if (rd == 15) {
        log_fatal("handle pls");
        regs.cpsr = get_spsr();
    } else {
        set_condition_flag(Z_FLAG, regs.r[rd] == 0);
	    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    }
    
    regs.r[15] += 4;
}

void ARMInterpreter::cmns(u32 op2) {
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] + op2;
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(N_FLAG, result >> 31);
    // understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
    set_condition_flag(V_FLAG, (~(regs.r[rn] ^ op2) & (op2 ^ result)) >> 31);

    regs.r[15] += 4;
}

void ARMInterpreter::rscs(u32 op2) {
    // TODO: correct the flag setting later
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = op2 - regs.r[rn] - !get_condition_flag(C_FLAG);
    if (rd == 15) {
        regs.cpsr = get_spsr();
    } else {
        set_condition_flag(N_FLAG, result >> 31);
        set_condition_flag(Z_FLAG, result == 0);
        set_condition_flag(V_FLAG, ((regs.r[rn] ^ op2) & (regs.r[rn] ^ result)) >> 31);
		set_condition_flag(C_FLAG, (u64)op2 >= (u64)regs.r[rn] + (u64)!get_condition_flag(C_FLAG));
    }

    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::sbcs(u32 op2) {
    // TODO: correct the flag setting later
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] - op2 - 1 + get_condition_flag(C_FLAG);

    if (rd == 15) {
        regs.cpsr = get_spsr();
    } else {
        set_condition_flag(N_FLAG, result >> 31);
        set_condition_flag(Z_FLAG, result == 0);
        set_condition_flag(V_FLAG, ((regs.r[rn] ^ op2) & (op2 ^ result)) >> 31);
        // TODO: fix later
		set_condition_flag(C_FLAG, regs.r[rn] >= result);
    }

    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::swp() {
    u8 rm = opcode & 0xF;
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    u32 address = regs.r[rn];
    u32 data = read_word(address);
    if (address & 0x3) {
        data = rotate_right(data, (address & 0x3) * 8);
    }

    write_word(address, regs.r[rm]);
    regs.r[rd] = data;


    regs.r[15] += 4;
}

void ARMInterpreter::swpb() {
    u8 rm = opcode & 0xF;
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    u8 data = read_byte(regs.r[rn]);
    write_byte(regs.r[rn], regs.r[rm] & 0xFF);
    regs.r[rd] = data;

    regs.r[15] += 4;
}



void ARMInterpreter::mlas() {
    u8 rm = opcode & 0xF;
    u8 rs = (opcode >> 8) & 0xF;
    u8 rn = (opcode >> 12) & 0xF;
    u8 rd = (opcode >> 16) & 0xF;

    u32 result = (regs.r[rm] * regs.r[rs]) + regs.r[rn];
    set_condition_flag(N_FLAG, result >> 31);
    set_condition_flag(Z_FLAG, result == 0);

    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::muls() {
    u8 rm = opcode & 0xF;
    u8 rs = (opcode >> 8) & 0xF;
    u8 rd = (opcode >> 16) & 0xF;
    u32 result = regs.r[rm] * regs.r[rs];
    set_condition_flag(N_FLAG, result >> 31);
    set_condition_flag(Z_FLAG, result == 0);

    regs.r[rd] = result;

    regs.r[15] += 4;

}

void ARMInterpreter::umulls() {
    u8 rm = opcode & 0xF;
    u8 rs = (opcode >> 8) & 0xF;
    u8 rdlo = (opcode >> 12) & 0xF;
    u8 rdhi = (opcode >> 16) & 0xF;
    u64 result = (u64)regs.r[rm] * (u64)regs.r[rs];
    set_condition_flag(N_FLAG, result >> 63);
    set_condition_flag(Z_FLAG, result == 0);

    regs.r[rdhi] = result >> 32;
    regs.r[rdlo] = result & 0xFFFFFFFF;

    regs.r[15] += 4;
}

void ARMInterpreter::smulls() {
    u8 rm = opcode & 0xF;
    u8 rs = (opcode >> 8) & 0xF;
    u8 rdlo = (opcode >> 12) & 0xF;
    u8 rdhi = (opcode >> 16) & 0xF;
    s64 result = (s32)(regs.r[rm]);
    result *= (s32)(regs.r[rs]);
    regs.r[rdhi] = result >> 32;
    regs.r[rdlo] = result;
    set_condition_flag(N_FLAG, regs.r[rdhi] >> 31);
    set_condition_flag(Z_FLAG, result == 0);
    

    regs.r[15] += 4;
}

void ARMInterpreter::umlals() {
    u8 rm = opcode & 0xF;
    u8 rs = (opcode >> 8) & 0xF;
    u8 rdlo = (opcode >> 12) & 0xF;
    u8 rdhi = (opcode >> 16) & 0xF;
    u64 result = (u64)regs.r[rm] * (u64)regs.r[rs];
    result += ((u64)regs.r[rdlo] + (u64)regs.r[rdhi]);
    regs.r[rdlo] = result;
    regs.r[rdhi] = result >> 32;
    set_condition_flag(N_FLAG, regs.r[rdhi] >> 31);
    set_condition_flag(Z_FLAG, result == 0);

    regs.r[15] += 4;
}

void ARMInterpreter::smlals() {
    // TODO: check later!
    u8 rm = opcode & 0xF;
    u8 rs = (opcode >> 8) & 0xF;
    u8 rdlo = (opcode >> 12) & 0xF;
    u8 rdhi = (opcode >> 16) & 0xF;
    s64 result = (s32)regs.r[rm];
    result *= (s32)regs.r[rs];
    result += ((s64)regs.r[rdlo] + (s64)regs.r[rdhi]);
    regs.r[rdlo] = result;
    regs.r[rdhi] = result >> 32;
    set_condition_flag(N_FLAG, regs.r[rdhi] >> 31);
    set_condition_flag(Z_FLAG, result == 0);

    regs.r[15] += 4;
}

void ARMInterpreter::clz() {
    // arm9 exclusive
    if (cpu_id == ARMv4) {
        return;
    }
    u8 rm = opcode & 0xF;
    u8 rd = (opcode >> 12) & 0xF;
    if (regs.r[rm] == 0) {
        regs.r[rd] = 32;
        return;
    } else {
        // rm is checked from bit 31 to bit 0 and number found when the bit is 1
        for (int i = 31; i >= 0; i--) {
            if (get_bit(i, regs.r[rm])) {
                regs.r[rd] = 31 - i;
                return;
            }
        }
    }
}

// fine
u32 ARMInterpreter::lli() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    return regs.r[rm] << shift_amount;
}

// fine
u32 ARMInterpreter::lri() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    if (shift_amount == 0) {
        return 0;
    } else {
        return regs.r[rm] >> shift_amount;
    }
}

// fine
u32 ARMInterpreter::lris() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    u32 result;
    if (shift_amount == 0) {
        set_condition_flag(C_FLAG, regs.r[rm] & (1 << 31));
        result = 0;
    } else {
        // means shift_amount > 0
        result = regs.r[rm] >> shift_amount;
        set_condition_flag(C_FLAG, regs.r[rm] & (1 << (shift_amount - 1)));
    }
    return result;
}


// fine
u32 ARMInterpreter::lrrs() {
    u8 shift_amount = regs.r[(opcode >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[opcode & 0xF];
    if ((opcode & 0xF) == 15) {
        // prefetch due to how register shifting executes after the prefetch
        rm += 4;
    }
    u32 result;
    if (shift_amount == 0) {
        result = rm;
    } else if (shift_amount < 32) {
        result = rm >> shift_amount;
        set_condition_flag(C_FLAG, rm & (1 << (shift_amount - 1)));
    } else if (shift_amount == 32) {
        result = 0;
        set_condition_flag(C_FLAG, rm & (1 << 31));
    } else {
        result = 0;
        set_condition_flag(C_FLAG, false);
    }
    return result;
}

// fine
u32 ARMInterpreter::llis() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    
    if (shift_amount > 0) {
        set_condition_flag(C_FLAG, regs.r[rm] & (1 << (32 - shift_amount)));
    }
    return regs.r[rm] << shift_amount;
}


// fine
u32 ARMInterpreter::llrs() {
    u8 shift_amount = regs.r[(opcode >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[opcode & 0xF];
    if ((opcode & 0xF) == 15) {
        // prefetch due to how register shifting executes after the prefetch
        rm += 4;
    }
    u32 result;
    if (shift_amount == 0) {
        result = rm;
    } else if (shift_amount < 32) {
        result = rm << shift_amount;
        set_condition_flag(C_FLAG, rm & (1 << (32 - shift_amount)));
    } else if (shift_amount == 32) {
        result = 0;
        set_condition_flag(C_FLAG, rm & 0x1);
    } else {
        // shift amount > 32
        result = 0;
        set_condition_flag(C_FLAG, false);
    }


    return result;
}

// fine
u32 ARMInterpreter::ari() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    u32 result;
    u8 msb = regs.r[rm] >> 31;

    if (shift_amount == 0) {
        result = 0xFFFFFFFF * msb;
    } else {
        // shift amount > 0
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
    return result;
}

// fine
u32 ARMInterpreter::aris() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    u32 result;
    u8 msb = regs.r[rm] >> 31;

    if (shift_amount == 0) {
        result = 0xFFFFFFFF * msb;
        set_condition_flag(C_FLAG, msb);
    } else {
        // shift amount > 0
        set_condition_flag(C_FLAG, regs.r[rm] & (1 << (shift_amount - 1)));
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
    return result;
}

// fine
u32 ARMInterpreter::arrs() {
    u8 shift_amount = regs.r[(opcode >> 8) & 0xF] & 0xFF;
    u32 rm = regs.r[opcode & 0xF];
    if ((opcode & 0xF) == 15) {
        // prefetch due to how register shifting executes after the prefetch
        rm += 4;
    }
    u32 result;
    u8 msb = rm >> 31;

    if (shift_amount == 0) {
        result = rm;
    } else if (shift_amount < 32) {
        // shift amount > 0
        set_condition_flag(C_FLAG, rm & (1 << (shift_amount - 1)));
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (rm >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    } else {
        // shift amount > 32
        result = 0xFFFFFFFF * msb;
        set_condition_flag(C_FLAG, msb);
    }
    return result;
}

u32 ARMInterpreter::rris() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;

    u32 result;
    if (shift_amount == 0) {
        // perform rotate right extend
        result = (get_condition_flag(C_FLAG) << 31) | (regs.r[rm] >> 1);
        set_condition_flag(C_FLAG, regs.r[rm] & 0x1);
    } else {
        // shift amount > 0
        result = (regs.r[rm] >> shift_amount) | (regs.r[rm] << (32 - shift_amount));
        set_condition_flag(C_FLAG, regs.r[rm] & (1 << (shift_amount - 1)));
    }

    return result;
}

u32 ARMInterpreter::rri() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;

    u32 result;
    if (shift_amount == 0) {
        // perform rotate right extend
        result = (get_condition_flag(C_FLAG) << 31) | (regs.r[rm] >> 1);
    } else {
        // shift amount > 0
        result = (regs.r[rm] >> shift_amount) | (regs.r[rm] << (32 - shift_amount));
    }

    return result;
}