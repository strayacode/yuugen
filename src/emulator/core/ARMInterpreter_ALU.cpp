#include <emulator/common/types.h>
#include <emulator/common/log.h>
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
        // log_debug("we are now at r15 = 0x%04x", regs.r[15]);
        flush_pipeline(); // shrug idk what im doing lmao
        // log_warn("the behaviour rd = 15 might be handled incorrectly in mov");
    } else {
        regs.r[15] += 4;
    }
    
}

void ARMInterpreter::mvn(u32 op2) {
    // get rd
    u8 rd = (opcode >> 12) & 0xF;
    regs.r[rd] = ~op2;

    if (rd == 15) {
        // log_debug("we are now at r15 = 0x%04x", regs.r[15]);
        flush_pipeline(); // shrug idk what im doing lmao
        // log_warn("the behaviour rd = 15 might be handled incorrectly in mov");
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
    // log_debug("result: %04x", result);

    set_condition_flag(N_FLAG, result >> 31);
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(C_FLAG, op2 >= regs.r[rn]);
    set_condition_flag(V_FLAG, (~(regs.r[rn] ^ op2) & (op2 ^ result)) >> 31);

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
    u32 result = regs.r[rn] + op2;
    regs.r[rd] = result;
    set_condition_flag(Z_FLAG, result == 0);
    set_condition_flag(N_FLAG, result >> 31);
    // understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
    set_condition_flag(V_FLAG, (~(regs.r[rn] ^ op2) & (op2 ^ result)) >> 31);
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
    set_condition_flag(N_FLAG, result >> 31);

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

    regs.r[15] += 4;
}

void ARMInterpreter::sbcs(u32 op2) {
    // TODO: correct the flag setting later
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 result = regs.r[rn] - op2 - !get_condition_flag(C_FLAG);
    if (rd == 15) {
        regs.cpsr = get_spsr();
    } else {
        set_condition_flag(N_FLAG, result >> 31);
        set_condition_flag(Z_FLAG, result == 0);
        set_condition_flag(V_FLAG, ((regs.r[rn] ^ op2) & (op2 ^ result)) >> 31);
		set_condition_flag(C_FLAG, (u64)regs.r[rn] >= (u64)op2 + (u64)!get_condition_flag(C_FLAG));
    }

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

u32 ARMInterpreter::lli() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    return regs.r[rm] << shift_amount;
}

u32 ARMInterpreter::lri() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    if (rm == 15) {
        log_fatal("ok");
    }
    return regs.r[rm] >> shift_amount;
}

u32 ARMInterpreter::lris() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    if (rm == 15) {
        log_fatal("ok");
    }
    if (shift_amount == 0) {
        set_condition_flag(C_FLAG, regs.r[rm] >> 31);
    } else {
        // means shift_amount > 0
        set_condition_flag(C_FLAG, (regs.r[rm] >> (shift_amount - 1)) & 0x1);
    }
    return regs.r[rm] >> shift_amount;
}

u32 ARMInterpreter::lrrs() {
    u8 shift_amount = regs.r[(opcode >> 8) & 0xF] & 0xFF;
    u8 rm = opcode & 0xF;
    if (rm == 15) {
        log_fatal("ok");
    }
    if (shift_amount == 0) {
        set_condition_flag(C_FLAG, regs.r[rm] >> 31);
    } else {
        // means shift_amount > 0
        set_condition_flag(C_FLAG, (regs.r[rm] >> (shift_amount - 1)) & 0x1);
    }
    return regs.r[rm] >> shift_amount;
}


u32 ARMInterpreter::llis() {
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 rm = opcode & 0xF;
    if (rm == 15) {
        log_fatal("ok");
    }
    if (shift_amount > 0) {
        set_condition_flag(C_FLAG, (regs.r[rm] >> (32 - shift_amount)) & 0x1);
    }
    return regs.r[rm] << shift_amount;
}

u32 ARMInterpreter::llrs() {
    u8 shift_amount = regs.r[(opcode >> 8) & 0xF] & 0xFF;
    u8 rm = opcode & 0xF;
    if (shift_amount > 0) {
        set_condition_flag(C_FLAG, (regs.r[rm] >> (32 - shift_amount)) & 0x1);
    }
    return regs.r[rm] << shift_amount;
}

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
        set_condition_flag(C_FLAG, (regs.r[rm] >> (shift_amount - 1) & 0x1));
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
    return result;
}

u32 ARMInterpreter::arrs() {
    u8 shift_amount = regs.r[(opcode >> 8) & 0xF] & 0xFF;
    u8 rm = opcode & 0xF;
    u32 result;
    u8 msb = regs.r[rm] >> 31;

    if (shift_amount == 0) {
        result = 0xFFFFFFFF * msb;
        set_condition_flag(C_FLAG, msb);
    } else {
        // shift amount > 0
        set_condition_flag(C_FLAG, (regs.r[rm] >> (shift_amount - 1) & 0x1));
        // perform asr
        // what happens is that rm gets shifted normally to the right but then the bits not set are set with the msb
        result = (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
    return result;
}