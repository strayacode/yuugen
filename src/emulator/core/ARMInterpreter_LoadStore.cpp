#include <emulator/core/ARMInterpreter.h>
#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>
#include <emulator/common/log.h>

// fine
u32 ARMInterpreter::imm_single_data_transfer() {
    return opcode & 0xFFF;
}

// fine
void ARMInterpreter::str_pre(u32 op2) {
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
    }

    write_word(address, regs.r[rd]);

    

    
}

// fine
void ARMInterpreter::str_post(u32 op2) {
    
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;

    write_word(regs.r[rn], regs.r[rd]);

    
    // always writeback in post transfer
    regs.r[rn] += op2;
    

    
}

// fine
void ARMInterpreter::ldr_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    // if (regs.r[rn] & 0x3 != 0) {
    //     log_warn("another thing to handle");
    // }
    u32 address = regs.r[rn];
    regs.r[15] += 4;
    u32 data = read_word(regs.r[rn]);
    

    // TODO: make sure this is correct later
    // i.e. not word aligned
    if (address & 0x3) {
        u8 shift_amount = (address & 0x3) * 8;
        data = rotate_right(data, shift_amount);
    }

    if (rd != rn) {
        // always writeback in post transfer
        regs.r[rn] += op2;
    }

    regs.r[rd] = data;
    

    
}

// fine
void ARMInterpreter::ldr_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    // if (regs.r[rn] & 0x3 != 0) {
    //     log_warn("another thing to handle");
    // }
    // address = rotate_right(address, (address & 0x3) * 8);

    

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
u32 ARMInterpreter::reg_halfword_signed_data_transfer() {
    return regs.r[opcode & 0xF];
}

// fine
u32 ARMInterpreter::imm_halfword_signed_data_transfer() {
    return ((opcode >> 4) & 0xF0) | (opcode & 0xF);
}

// fine
void ARMInterpreter::strh_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    
    if (rd == 15) {
        log_fatal("handle");
    }
    regs.r[15] += 4;

    write_halfword(regs.r[rn] + op2, regs.r[rd]);

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] += op2;
    }
}

// fine
void ARMInterpreter::strh_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;

    write_halfword(regs.r[rn], regs.r[rd]);

    

    // always write back to base register in post indexing
    regs.r[rn] += op2;

    
}

// fine
void ARMInterpreter::ldrh_post(u32 op2) {
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
void ARMInterpreter::ldrh_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    
    u32 address = read_halfword(regs.r[rn] + op2);

    regs.r[15] += 4;

    

    
    // check for writeback

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] += op2;
    }

    regs.r[rd] = address;

    
}

// fine
void ARMInterpreter::ldrb_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = read_byte(regs.r[rn]);
    if (rd == rn) {
        log_fatal("hmmmm");
    }

    regs.r[15] += 4;

    // always writeback in post transfer
    regs.r[rn] += op2;

    regs.r[rd] = address;

    
}

// fine
void ARMInterpreter::ldrb_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    u32 data = read_byte(address);
    // TODO: check this out later more
    // data = rotate_right(data, (regs.r[rn] & 0x3) * 8);

    regs.r[15] += 4;

    if (rd == rn) {
        log_fatal("hmmmm");
    }
    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] = address;
    }

    regs.r[rd] = data;


    

    
}

// fine
void ARMInterpreter::strb_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;

    write_word(regs.r[rn], (u8)regs.r[rd]);

    // always writeback in post transfer
    regs.r[rn] += op2;

    
}

// fine
void ARMInterpreter::strb_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    regs.r[15] += 4;

    write_word(regs.r[rn] + op2, (u8)regs.r[rd]);

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] += op2;
    }

    
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

    

    // subtract offset from base
    for (int i = 15; i >= 0; i--) {
        if (get_bit(i, opcode)) { 
            // pre decrement the address
            address -= 4;
            // write register to address
            write_word(address, regs.r[i]);
        }
    }
   

    // writeback to base register
    regs.r[rn] = address;
}

// fine
void ARMInterpreter::stmdaw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    regs.r[15] += 4;

    

    // subtract offset from base
    for (int i = 15; i >= 0; i--) {
        if (get_bit(i, opcode)) { 
            // write register to address
            write_word(address, regs.r[i]);
            // post decrement the address
            address -= 4;
        }
    }

    // writeback to base register
    regs.r[rn] = address;
}

void ARMInterpreter::ldmiaw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
    

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            regs.r[i] = read_word(address);
            address += 4;
        }
    }


    regs.r[rn] = address;

    
}



void ARMInterpreter::ldmibw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;

    

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address += 4;
            regs.r[i] = read_word(address);
            
        }
    }


    regs.r[rn] = address;

    
}



void ARMInterpreter::ldmdbw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;


    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }    

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            
            regs.r[i] = read_word(address);
            address += 4;
            
        }
    }


    regs.r[rn] = address;

    
}

void ARMInterpreter::ldmdaw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }    

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address += 4;
            regs.r[i] = read_word(address);
            
            
        }
    }


    regs.r[rn] = address;

    
}

void ARMInterpreter::ldmiauw() {
    log_debug("1");
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];
    
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
    
    // first we must switch to user mode so that we can change the values of usr mode registers
    update_mode(0x1F);

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            
            regs.r[i] = read_word(address);
            address += 4;
        }
    }

    // writeback cant occur when usr regs are written to
    regs.r[rn] = address;

    
}

void ARMInterpreter::ldmdauw() {
    // log_warn("jew");
    log_debug("2");
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;

    // first we must switch to user mode so that we can change the values of usr mode registers
    update_mode(0x1F);

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }    

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address += 4;
            // load the values into user registers
            regs.r[i] = read_word(address);
        }
    }

    regs.r[rn] = address;


}

void ARMInterpreter::ldmibuw() {
    log_debug("3");
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];
    log_debug("in ldmibuw: address we are reading from is 0x%04x", address);
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;

    // first we must switch to user mode so that we can change the values of usr mode registers
    update_mode(0x1F);

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address += 4;
            regs.r[i] = read_word(address);
            
        }
    }

    regs.r[rn] = address;

    
}

void ARMInterpreter::ldmdbuw() {
    log_debug("4");
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;

    // first we must switch to user mode so that we can change the values of usr mode registers
    update_mode(0x1F);


    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }    

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            
            regs.r[i] = read_word(address);
            address += 4;
            
        }
    }


    regs.r[rn] = address;

    
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