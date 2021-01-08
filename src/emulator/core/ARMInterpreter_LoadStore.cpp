#include <emulator/core/ARMInterpreter.h>
#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>
#include <emulator/common/log.h>

// fine
u32 ARMInterpreter::arm_imm_single_data_transfer() {
    return opcode & 0xFFF;
}

// fine
void ARMInterpreter::arm_str_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle");
    }

    

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] = address;
    }

    write_word(address, regs.r[rd]);

    
    regs.r[15] += 4;
    
}

// fine
void ARMInterpreter::arm_str_post(u32 op2) {
    
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    u32 address = regs.r[rn] + op2;

    write_word(regs.r[rn], regs.r[rd]);

    
    // always writeback in post transfer
    regs.r[rn] += op2;
    
    regs.r[15] += 4;
    
}

// fine
void ARMInterpreter::arm_ldr_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];
    regs.r[15] += 4;
    u32 data = read_word(regs.r[rn]);
    
    if (rd == 15) {
        log_fatal("handle");
    }
    // TODO: make sure this is correct later
    // i.e. not word aligned
    if (address & 0x3) {
        u8 shift_amount = (address & 0x3) * 8;
        data = rotate_right(data, shift_amount);
    }

    regs.r[rd] = data;  

    if (rd != rn) {
        // always writeback in post transfer
        regs.r[rn] += op2;
    }

    
}

// fine
void ARMInterpreter::arm_ldr_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;

    u32 data = read_word(address);

    if (address & 0x3) {
        u8 shift_amount = (address & 0x3) * 8;
        data = rotate_right(data, shift_amount);
    }

    
    if (get_bit(21, opcode) && (rd != rn)) {
        // write back to rn
        regs.r[rn] = address;
    }

    regs.r[rd] = data;

    regs.r[15] += 4;
    
}


// fine
u32 ARMInterpreter::arm_reg_halfword_signed_data_transfer() {
    return regs.r[opcode & 0xF];
}

// fine
u32 ARMInterpreter::arm_imm_halfword_signed_data_transfer() {
    return ((opcode >> 4) & 0xF0) | (opcode & 0xF);
}

// fine
void ARMInterpreter::arm_strh_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle");
    }
    regs.r[15] += 4;
    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] = address;
        write_halfword(address, regs.r[rd]);
    } else {
        write_halfword(address, regs.r[rd]);
    }
    

    // TODO: change later!
}

// fine
void ARMInterpreter::arm_strh_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    if (rd == 15) {
        log_fatal("handle");
    }
    // always write back to base register in post indexing
    regs.r[rn] = address;

    

    write_halfword(regs.r[rn], regs.r[rd]);

    
    regs.r[15] += 4;
    

    
}

// fine
void ARMInterpreter::arm_ldrh_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];
    u32 data = read_halfword(address);
    regs.r[15] += 4;

    // always write back to base register in post indexing
    regs.r[rn] += op2;

    regs.r[rd] = data;
    
}

// fine
void ARMInterpreter::arm_ldrh_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    
    // check for writeback

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] += op2;
        regs.r[rd] = read_halfword(regs.r[rn]);
    } else {
        regs.r[rd] = read_halfword(regs.r[rn] + op2);
    }

    

    regs.r[15] += 4;
}

// fine
void ARMInterpreter::arm_ldrb_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    regs.r[rd] = read_byte(regs.r[rn]);
    regs.r[rn] += op2;

    if (rd == 15) {
        log_fatal("hmmmm");
    }

    regs.r[15] += 4;
 
}

// fine
void ARMInterpreter::arm_ldrb_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] += op2;
        regs.r[rd] = read_byte(regs.r[rn]);
    } else {
        regs.r[rd] = read_byte(regs.r[rn] + op2);
    }

    

    
    // TODO: maybe ldrb and strb use the rn == rd edgecase ill check later
    
    

    regs.r[15] += 4;

    
}

// fine
void ARMInterpreter::arm_strb_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    write_word(regs.r[rn], regs.r[rd]);

    // always writeback in post transfer
    regs.r[rn] += op2;

    regs.r[15] += 4;

    
}

// fine
void ARMInterpreter::arm_strb_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] += op2;
        write_word(regs.r[rn], regs.r[rd]);
    } else {
        write_word(regs.r[rn] + op2, regs.r[rd]);
    }

    

    regs.r[15] += 4;

    
}

// TODO: look at writeback later
void ARMInterpreter::ldrsb_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    u32 address = regs.r[rn] + op2;
    // first we cast to s8 to read the data as a signed byte and then cast to s32 to sign extend to a 32 bit integer
    u32 data = (s32)(s8)read_byte(address);

    regs.r[rd] = data;

    regs.r[15] += 4;
}

// fine
void ARMInterpreter::stmiaw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    regs.r[15] += 4;

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            write_word(address, regs.r[i]);
            address += 4;
        }
    }

    regs.r[rn] = address;
    

    
}

// fine
void ARMInterpreter::stmibw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    regs.r[15] += 4;

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address += 4;
            write_word(address, regs.r[i]);
            
        }
    }

    regs.r[rn] = address;
    

    
}

// fine
void ARMInterpreter::stmdbw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    regs.r[15] += 4;

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    

    // subtract offset from base
    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) { 
            // write register to address
            write_word(address, regs.r[i]);
            // pre decrement the address
            address += 4;
        }
    }

    // writeback to base register
    regs.r[rn] = writeback;
   
    
}

// fine
void ARMInterpreter::stmdaw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    
    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }

    u32 writeback = address;
    
    // subtract offset from base
    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) { 
            // post decrement the address
            address += 4;
            // write register to address
            write_word(address, regs.r[i]);
            
        }
    }

    

    // writeback to base register
    regs.r[rn] = writeback;

    regs.r[15] += 4;
}

void ARMInterpreter::ldmiaw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (opcode & (1 << i)) {
            regs.r[i] = read_word(address);
            address += 4;
        }
    }


    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if (!(opcode & (1 << rn)) ||(cpu_id == ARMv5 && ((opcode & 0xFFFF) == (1 << rn)) || !(((opcode & 0xFFFF) >> rn) == 1))) {
        regs.r[rn] = address;
    }
    
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}



void ARMInterpreter::ldmibw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (opcode & (1 << i)) {
            address += 4;
            regs.r[i] = read_word(address);
        }
    }


    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if (!(opcode & (1 << rn)) ||(cpu_id == ARMv5 && ((opcode & 0xFFFF) == (1 << rn)) || !(((opcode & 0xFFFF) >> rn) == 1))) {
        regs.r[rn] = address;
    }
    
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}



void ARMInterpreter::ldmdbw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    for (int i = 0; i < 16; i++) {
        if (opcode & (1 << i)) {
            regs.r[i] = read_word(address);
            address += 4;
        }
    }

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if (!(opcode & (1 << rn)) ||(cpu_id == ARMv5 && ((opcode & 0xFFFF) == (1 << rn)) || !(((opcode & 0xFFFF) >> rn) == 1))) {
        regs.r[rn] = writeback;
    }
    
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

void ARMInterpreter::ldmdaw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    for (int i = 0; i < 16; i++) {
        if (opcode & (1 << i)) {
            address += 4;
            regs.r[i] = read_word(address);
            
        }
    }

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if (!(opcode & (1 << rn)) ||(cpu_id == ARMv5 && ((opcode & 0xFFFF) == (1 << rn)) || !(((opcode & 0xFFFF) >> rn) == 1))) {
        regs.r[rn] = writeback;
    }
    
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

void ARMInterpreter::ldmiauw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];
    
    u8 old_mode = regs.cpsr & 0x1F;
    
    // first we must switch to user mode so that we can change the values of usr mode registers
    update_mode(0x1F);

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            
            regs.r[i] = read_word(address);
            address += 4;
        }
    }

    // switching back to to old mode is my guess
    update_mode(old_mode);

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    if (!(opcode & (1 << rn)) ||(cpu_id == ARMv5 && ((opcode & 0xFFFF) == (1 << rn)) || !(((opcode & 0xFFFF) >> rn) == 1))) {
        regs.r[rn] = address;
    }

    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
    
}

void ARMInterpreter::ldmdauw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    u8 old_mode = regs.cpsr & 0x1F;

    update_mode(0x1F);

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    for (int i = 0; i < 16; i++) {
        if (opcode & (1 << i)) {
            address += 4;
            regs.r[i] = read_word(address);
            
        }
    }

    update_mode(old_mode);

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if (!(opcode & (1 << rn)) || (cpu_id == ARMv5 && ((opcode & 0xFFFF) == (1 << rn)) || !(((opcode & 0xFFFF) >> rn) == 1))) {
        regs.r[rn] = writeback;
    }
    
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

void ARMInterpreter::ldmibuw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    u8 old_mode = regs.cpsr & 0x1F;

    update_mode(0x1F);

    for (int i = 0; i < 16; i++) {
        if (opcode & (1 << i)) {
            address += 4;
            regs.r[i] = read_word(address);
        }
    }

    update_mode(old_mode);


    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if (!(opcode & (1 << rn)) ||((cpu_id == ARMv5) && ((opcode & 0xFFFF) == (1 << rn)) || !(((opcode & 0xFFFF) >> rn) == 1))) {
        regs.r[rn] = address;
    }
    
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

void ARMInterpreter::ldmdbuw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    u8 old_mode = regs.cpsr & 0x1F;

    update_mode(0x1F);

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    for (int i = 0; i < 16; i++) {
        if (opcode & (1 << i)) {
            regs.r[i] = read_word(address);
            address += 4;
        }
    }

    update_mode(old_mode);

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if (!(opcode & (1 << rn)) ||((cpu_id == ARMv5) && ((opcode & 0xFFFF) == (1 << rn)) || !(((opcode & 0xFFFF) >> rn) == 1))) {
        regs.r[rn] = writeback;
    }
    
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

u32 ARMInterpreter::rpll() {
    u8 rm = opcode & 0xF;
    u8 shift_amount = (opcode >> 7) & 0x1F;
    
    return regs.r[rm] << shift_amount;
}

u32 ARMInterpreter::rplr() {
    u8 rm = opcode & 0xF;
    u8 shift_amount = (opcode >> 7) & 0x1F;
    if (shift_amount == 0) {
        return 0;
    } else {
        return regs.r[rm] >> shift_amount;
    }
}

u32 ARMInterpreter::rpar() {
    u8 rm = opcode & 0xF;
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 msb = opcode >> 31;

    if (shift_amount == 0) {
        return 0xFFFFFFFF * msb;
    } else {
        return (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
}

u32 ARMInterpreter::rprr() {
    u8 rm = opcode & 0xF;
    u8 shift_amount = (opcode >> 7) & 0x1F;

    if (shift_amount == 0) {
        // rotate right extended
        return (get_condition_flag(C_FLAG) << 31) | (regs.r[rm] >> 1);
    } else {
        // rotate right
        return (regs.r[rm] >> shift_amount) | (regs.r[rm] << (32 - shift_amount));
    }
}