#include <emulator/common/types.h>
#include <emulator/common/log.h>
#include <emulator/common/arithmetic.h>
#include <emulator/Emulator.h>

void ARMInterpreter::arm_mov(u32 op2) {
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

void ARMInterpreter::arm_movs(u32 op2) {
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

void ARMInterpreter::arm_mvn(u32 op2) {
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

u32 ARMInterpreter::arm_imm_data_processing() {
    u32 immediate = opcode & 0xFF;
    u8 shift_amount = ((opcode >> 8) & 0xF) * 2;

    u32 result = (immediate >> shift_amount) | (immediate << (32 - shift_amount));
    return result;
}

u32 ARMInterpreter::arm_imms_data_processing() {
    u32 immediate = opcode & 0xFF;
    u8 shift_amount = ((opcode >> 8) & 0xF) * 2;
    
    u32 result = (immediate >> shift_amount) | (immediate << (32 - shift_amount));
    if (shift_amount != 0) {
        set_condition_flag(C_FLAG, result >> 31);
    }
    return result;
}

void ARMInterpreter::arm_subs(u32 op2) {
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

void ARMInterpreter::arm_sub(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    u32 result = regs.r[rn] - op2;

    regs.r[rd] = result;
    
    if (rd == 15) {
        log_fatal("handle pls");
    }

    regs.r[15] += 4;
}

void ARMInterpreter::arm_cmps(u32 op2) {
    u8 rn = (opcode >> 16) & 0xF;
    
    u32 result = regs.r[rn] - op2;

    set_condition_flag(N_FLAG, result >> 31);
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(C_FLAG, op2 >= regs.r[rn]);
    set_condition_flag(V_FLAG, ((regs.r[rn] ^ op2) & (regs.r[rn] ^ result)) >> 31);

    regs.r[15] += 4;
}

void ARMInterpreter::arm_bics(u32 op2) {
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

void ARMInterpreter::arm_bic(u32 op2) {
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

void ARMInterpreter::arm_add(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    regs.r[rd] = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle pls");
    }

    regs.r[15] += 4;
    
}



void ARMInterpreter::arm_ands(u32 op2) {
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

void ARMInterpreter::arm_and(u32 op2) {
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

void ARMInterpreter::arm_eor(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] ^ op2;
    if (rd == 15) {
        log_fatal("handle pls");
    }

    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::arm_eors(u32 op2) {
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

void ARMInterpreter::arm_tsts(u32 op2) {
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] & op2;
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(N_FLAG, result >> 31);

    regs.r[15] += 4;
}

void ARMInterpreter::arm_adds(u32 op2) {
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

void ARMInterpreter::arm_adcs(u32 op2) {
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

void ARMInterpreter::arm_adc(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] + op2 + get_condition_flag(C_FLAG);
    regs.r[rd] = result;

    regs.r[15] += 4;
}

void ARMInterpreter::arm_orr(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    regs.r[rd] = regs.r[rn] | op2;

    if (rd == 15) {
        log_fatal("handle pls");
        regs.cpsr = get_spsr();
    } 

    

    regs.r[15] += 4;
}

void ARMInterpreter::arm_orrs(u32 op2) {
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

void ARMInterpreter::arm_cmns(u32 op2) {
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] + op2;
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(N_FLAG, result >> 31);
    // understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
    set_condition_flag(V_FLAG, (~(regs.r[rn] ^ op2) & (op2 ^ result)) >> 31);

    regs.r[15] += 4;
}

void ARMInterpreter::arm_rscs(u32 op2) {
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

void ARMInterpreter::arm_sbcs(u32 op2) {
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

void ARMInterpreter::arm_swp() {
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

void ARMInterpreter::arm_swpb() {
    u8 rm = opcode & 0xF;
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    u8 data = read_byte(regs.r[rn]);
    write_byte(regs.r[rn], regs.r[rm] & 0xFF);
    regs.r[rd] = data;

    regs.r[15] += 4;
}



void ARMInterpreter::arm_mlas() {
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

void ARMInterpreter::arm_muls() {
    u8 rm = opcode & 0xF;
    u8 rs = (opcode >> 8) & 0xF;
    u8 rd = (opcode >> 16) & 0xF;
    u32 result = regs.r[rm] * regs.r[rs];
    set_condition_flag(N_FLAG, result >> 31);
    set_condition_flag(Z_FLAG, result == 0);

    regs.r[rd] = result;

    regs.r[15] += 4;

}

void ARMInterpreter::arm_umulls() {
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

void ARMInterpreter::arm_smulls() {
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

void ARMInterpreter::arm_umlals() {
    u8 rm = opcode & 0xF;
    u8 rs = (opcode >> 8) & 0xF;
    u8 rdlo = (opcode >> 12) & 0xF;
    u8 rdhi = (opcode >> 16) & 0xF;
    u64 rdhilo = ((u64)regs.r[rdhi] << 32) | ((u64)regs.r[rdlo]);
    u64 result = (u64)regs.r[rm] * (u64)regs.r[rs] + rdhilo;
    set_condition_flag(N_FLAG, result >> 63);
    set_condition_flag(Z_FLAG, result == 0);
    regs.r[rdlo] = result & 0xFFFFFFFF;

    regs.r[rdhi] = result >> 32;


    regs.r[15] += 4;
}

void ARMInterpreter::arm_smlals() {
    // TODO: check later!
    u8 rm = opcode & 0xF;
    u8 rs = (opcode >> 8) & 0xF;
    u8 rdlo = (opcode >> 12) & 0xF;
    u8 rdhi = (opcode >> 16) & 0xF;
    s64 rdhilo = (s64)(((u64)regs.r[rdhi] << 32) | ((u64)regs.r[rdlo]));
    s64 result = ((s64)(s32)regs.r[rm] * (s64)(s32)regs.r[rs]);
    result += rdhilo;
    regs.r[rdlo] = result;
    regs.r[rdhi] = result >> 32;
    set_condition_flag(N_FLAG, regs.r[rdhi] >> 31);
    set_condition_flag(Z_FLAG, result == 0);

    regs.r[15] += 4;
}

void ARMInterpreter::arm_clz() {
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
u32 ARMInterpreter::arm_lli() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    return regs.r[rm] << shift_amount;
}

// fine
u32 ARMInterpreter::arm_lri() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    if (shift_amount == 0) {
        return 0;
    } else {
        return regs.r[rm] >> shift_amount;
    }
}

// fine
u32 ARMInterpreter::arm_lris() {
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
u32 ARMInterpreter::arm_lrrs() {
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
u32 ARMInterpreter::arm_llis() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    
    if (shift_amount > 0) {
        set_condition_flag(C_FLAG, regs.r[rm] & (1 << (32 - shift_amount)));
    }
    return regs.r[rm] << shift_amount;
}


// fine
u32 ARMInterpreter::arm_llrs() {
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
u32 ARMInterpreter::arm_ari() {
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
u32 ARMInterpreter::arm_aris() {
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
u32 ARMInterpreter::arm_arrs() {
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

u32 ARMInterpreter::arm_rris() {
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

u32 ARMInterpreter::arm_rri() {
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


// start of thumb instructions

void ARMInterpreter::thumb_mov_imm() {
    u8 rd = (opcode >> 8) & 0x7;

    regs.r[rd] = opcode & 0xFF;
    
    // set flags
    set_condition_flag(N_FLAG, regs.r[0] >> 31);
    set_condition_flag(Z_FLAG, regs.r[0] == 0);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_movh() {
    u8 rd = (get_bit(7, opcode) << 3) | (opcode & 0x7);

    u8 rm = (get_bit(6, opcode) << 3) | ((opcode >> 3) & 0x7);  
    regs.r[rd] = regs.r[rm];

    regs.r[15] += 2; 
}

void ARMInterpreter::thumb_cmp_imm() {
    u8 rn = (opcode >> 8) & 0x7;
    u32 immediate = opcode & 0xFF;
    u32 result = regs.r[rn] - immediate;


    // set flags
    set_condition_flag(N_FLAG, result >> 31);
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(C_FLAG, immediate >= regs.r[rn]);
    set_condition_flag(V_FLAG, ((regs.r[rn] ^ immediate) & (regs.r[rn] ^ result)) >> 31);


    regs.r[15] += 2;  
}

void ARMInterpreter::thumb_cmp_reg() {
    u8 rn = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;
    u32 result = regs.r[rn] - regs.r[rm];


    // set flags
    set_condition_flag(N_FLAG, result >> 31);
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(C_FLAG, regs.r[rm] >= regs.r[rn]);
    set_condition_flag(V_FLAG, ((regs.r[rn] ^ regs.r[rm]) & (regs.r[rn] ^ result)) >> 31);


    regs.r[15] += 2;  
}

void ARMInterpreter::thumb_tst_reg() {
    u8 rn = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;

    u32 result = regs.r[rn] & regs.r[rm];

    set_condition_flag(N_FLAG, result >> 31);
    set_condition_flag(Z_FLAG, result == 0);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_lsr_imm() {
    u8 rd = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;
    u32 immediate_5 = (opcode >> 6) & 0x1F;
    if (immediate_5 == 0) {
        set_condition_flag(C_FLAG, regs.r[rd] >> 31);
        regs.r[rd] = 0;
    } else {
        // immediate_5 > 0
        set_condition_flag(C_FLAG, regs.r[rd] & (1 << (immediate_5 - 1)));
        regs.r[rd] = regs.r[rm] >> immediate_5;
        
    }
    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);
    regs.r[15] += 2;
}

void ARMInterpreter::thumb_asr_imm() {
    u8 rd = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;

    u32 immediate = (opcode >> 6) & 0x1F;

    u32 msb = regs.r[rm] >> 31;

    if (immediate == 0) {
        set_condition_flag(C_FLAG, msb);
        regs.r[rd] = 0xFFFFFFFF * msb;

        
    } else {
        // immediate > 0
        set_condition_flag(C_FLAG, regs.r[rm] & (1 << (immediate - 1)));
        regs.r[rd] = (regs.r[rm] >> immediate) | ((0xFFFFFFFF * msb) << (32 - immediate));
    }

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_lsl_reg() {
    u8 rd = opcode & 0x7;
    u8 rs = (opcode >> 3) & 0x7;
    u8 shift_amount = regs.r[rs] & 0xFF;
    // TODO: optimise later
    if (shift_amount == 0) {
        // do nothing lol
    } else if (shift_amount < 32) {
        set_condition_flag(C_FLAG, regs.r[rd] & (1 << (32 - shift_amount)));
        regs.r[rd] <<= shift_amount;
    } else if (shift_amount == 32) {
        set_condition_flag(C_FLAG, regs.r[rd] & 0x1);
        regs.r[rd] = 0;
    } else {
        // shift_amount > 32
        set_condition_flag(C_FLAG, false);
        regs.r[rd] = 0;
    }

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_add_imm() {
    u8 immediate = opcode & 0xFF;
    u8 rd = (opcode >> 8) & 0x7;
    u64 result64 = (u64)regs.r[rd] + (u64)immediate;
    u32 result32 = (u32)result64;
    regs.r[rd] = result32;

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);
    // detect borrow from bit 32
    set_condition_flag(C_FLAG, result64 >> 32);
    // understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
    set_condition_flag(V_FLAG, (~(regs.r[rd] ^ immediate) & (immediate ^ result32)) >> 31);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_add_reg() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;
    u8 rm = (opcode >> 6) & 0x7;
    u64 result64 = (u64)regs.r[rn] + (u64)regs.r[rm];
    u32 result32 = (u32)result64;
    regs.r[rd] = regs.r[rn] + regs.r[rm];
    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);
    // detect borrow from bit 32
    set_condition_flag(C_FLAG, result64 >> 32);
    // understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
    set_condition_flag(V_FLAG, (~(regs.r[rm] ^ regs.r[rn]) & (regs.r[rn] ^ result32)) >> 31);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_sub_imm() {
    u32 immediate = opcode & 0xFF;
    u8 rd = (opcode >> 8) & 0x7;
    
    u32 result = regs.r[rd] - immediate;
    regs.r[rd] = result;

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);
    set_condition_flag(C_FLAG, immediate >= regs.r[rd]);
    set_condition_flag(V_FLAG, (~(regs.r[rd] ^ immediate) & (immediate ^ result)) >> 31);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_lsl_imm() {
    u8 rd = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;
    u32 immediate = (opcode >> 6) & 0x1F;
    // TODO: optimise later

    if (immediate > 0) {
        set_condition_flag(C_FLAG, regs.r[rm] & (1 << (32 - immediate)));
    }
    
    regs.r[rd] = regs.r[rm] << immediate;
    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_orr() {
    u8 rd = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;
    regs.r[rd] |= regs.r[rm];

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);


    regs.r[15] += 2;
}

void ARMInterpreter::thumb_and() {
    u8 rd = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;

    regs.r[rd] &= regs.r[rm];

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_add_imm3() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;
    u32 immediate = (opcode >> 6) & 0x7;
    u64 result64 = (u64)regs.r[rn] + (u64)immediate;
    u32 result32 = (u32)result64;
    regs.r[rd] = regs.r[rn] + immediate;

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);
    // detect borrow from bit 32
    set_condition_flag(C_FLAG, result64 >> 32);
    // understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
    set_condition_flag(V_FLAG, (~(immediate ^ regs.r[rn]) & (regs.r[rn] ^ result32)) >> 31);

    regs.r[15] += 2;

}

void ARMInterpreter::thumb_addh() {
    u8 rd = (get_bit(7, opcode) << 3) | (opcode & 0x7);
    u8 rm = (opcode >> 3) & 0xF;

    regs.r[rd] += regs.r[rm];

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_addpc_reg() {
    u32 immediate = opcode & 0xFF;
    u8 rd = (opcode >> 8) & 0x7;

    regs.r[rd] = (regs.r[15] & 0xFFFFFFFC) + (immediate << 2);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_mul_reg() {
    u8 rd = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;

    regs.r[rd] *= regs.r[rm];
    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_mvn_reg() {
    u8 rd = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;


    regs.r[rd] = ~regs.r[rm];

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);

    regs.r[15] += 2;

}

void ARMInterpreter::thumb_neg_reg() {
    u8 rd = opcode & 0x7;
    u8 rm = (opcode >> 3) & 0x7;
    u8 zero = 0;

    regs.r[rd] = zero - regs.r[rm];

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);

    // TODO: fix later
    set_condition_flag(C_FLAG, zero >= regs.r[rd]);
    set_condition_flag(V_FLAG, ((zero ^ regs.r[rm]) & (zero ^ regs.r[rd])) >> 31);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_ror_reg() {
    u8 rd = opcode & 0x7;
    u8 rs = (opcode >> 3) & 0x7;

    u8 shift_amount = regs.r[rs] & 0xFF;
    // TODO: optimise this shit

    
    if (shift_amount > 0) {
        set_condition_flag(C_FLAG, regs.r[rd] & (1 << (shift_amount - 1)));
    }

    regs.r[rd] = (regs.r[rd] >> shift_amount) | (regs.r[rd] << (32 - shift_amount));

    set_condition_flag(N_FLAG, regs.r[rd] >> 31);
    set_condition_flag(Z_FLAG, regs.r[rd] == 0);

    

    regs.r[15] += 2;
}