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

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] = address;
    }

    write_word(address, regs.r[rd]);

    

    regs.r[15] += 4;
}

// fine
void ARMInterpreter::str_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    write_word(regs.r[rn], regs.r[rd]);

    // always writeback in post transfer
    regs.r[rn] += op2;

    regs.r[15] += 4;
}

// fine
void ARMInterpreter::ldr_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    regs.r[rd] = read_word(regs.r[rn]);

    // always writeback in post transfer
    regs.r[rn] += op2;

    regs.r[15] += 4;
}

// fine
void ARMInterpreter::ldr_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    u32 data = read_word(address);
    // TODO: check this out later more
    // data = rotate_right(data, (regs.r[rn] & 0x3) * 8);
    
    if (get_bit(21, opcode)) {
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
    
    write_halfword(regs.r[rn], regs.r[rd]);

    regs.r[15] += 4;

    // always write back to base register in post indexing
    regs.r[rn] += op2;

    
}

// fine
void ARMInterpreter::ldrh_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    
    regs.r[15] += 4;

    regs.r[rd] = read_halfword(regs.r[rn]);

    // always write back to base register in post indexing
    regs.r[rn] += op2;

    
}

// fine
void ARMInterpreter::ldrh_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    

    regs.r[15] += 4;

    regs.r[rd] = read_halfword(regs.r[rn] + op2);

    
    // check for writeback

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] += op2;
    }

    
}

// fine
void ARMInterpreter::ldrb_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    regs.r[rd] = read_byte(regs.r[rn]);

    // always writeback in post transfer
    regs.r[rn] += op2;

    regs.r[15] += 4;
}

// fine
void ARMInterpreter::ldrb_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    u32 data = read_byte(address);
    // TODO: check this out later more
    // data = rotate_right(data, (regs.r[rn] & 0x3) * 8);
    
    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] = address;
    }

    regs.r[rd] = data;


    regs.r[15] += 4;

    
}

// fine
void ARMInterpreter::strb_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    write_word(regs.r[rn], (u8)regs.r[rd]);

    // always writeback in post transfer
    regs.r[rn] += op2;

    regs.r[15] += 4;
}

// fine
void ARMInterpreter::strb_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    write_word(regs.r[rn] + op2, (u8)regs.r[rd]);

    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] += op2;
    }

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

void ARMInterpreter::ldmiaw() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    regs.r[15] += 4;

    for (int i = 0; i < 16; i++) {
        if (get_bit(i, opcode)) {
            regs.r[i] = read_word(address);
            address += 4;
        }
    }


    regs.r[rn] = address;
       
}