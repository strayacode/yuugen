#include <common/types.h>
#include <nds/arm.h>
#include <nds/nds.h>

u32 ARM::arm_imm_single_data_transfer() {
	return opcode & 0xFFF;
}


void ARM::arm_ldrb_pre(u32 op2) {
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
// this didnt work it seems lol

// void ARM::arm_ldrb_pre(u32 op2) {
// 	u8 rd = (opcode >> 12) & 0xF;
//     u8 rn = (opcode >> 16) & 0xF;

//     if (get_bit(21, opcode)) {
//     	// writeback to rn
//     	regs.r[rn] += op2;
//     }

//     regs.r[rd] = read_byte(regs.r[rn] + op2);

//     if (rd == 15) {
//     	if ((cpu_id == ARMv5) && (opcode & 0x1)) {
//     		// switch to thumb
//     		regs.r[15] &= ~1;

//     		// set t bit
//     		regs.cpsr |= (1 << 5);

//     		thumb_flush_pipeline();
//     	} else {
//     		// just flush normally
//     		regs.r[15] &= ~3;
//     		arm_flush_pipeline();
//     	}
//     }
    
//     regs.r[15] += 4;
// }

void ARM::arm_ldrb_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    regs.r[rd] = read_byte(regs.r[rn]);
    regs.r[rn] += op2;

    if (rd == 15) {
        log_fatal("hmmmm");
    }

    regs.r[15] += 4;
 
}

// URGENT: something was wrong with this instruction before ill investigate later

// void ARM::arm_ldr_pre(u32 op2) {
//     u8 rd = (opcode >> 12) & 0xF;
//     u8 rn = (opcode >> 16) & 0xF;
//     u32 address = regs.r[rn] + op2;
//     if (get_bit(21, opcode)) {
//         // writeback to rn
//         regs.r[rn] += op2;
//     }
//     regs.r[rd] = read_word(address);

//     if (regs.r[rn] & 0x3) {
//         u8 shift_amount = (regs.r[rn] & 0x3) * 8;
//         regs.r[rd] = rotate_right(regs.r[rd], shift_amount);
//     }

//     if (rd == 15) {
//         log_fatal("handle");
//         if ((cpu_id == ARMv5) && (opcode & 0x1)) {
//             // switch to thumb
//             regs.r[15] &= ~1;

//             // set t bit
//             regs.cpsr |= (1 << 5);

//             thumb_flush_pipeline();
//         } else {
//             // just flush normally
//             regs.r[15] &= ~3;
//             arm_flush_pipeline();
//         }
//     }
    
//     regs.r[15] += 4;
// }

void ARM::arm_ldrsb_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;

    u32 address = regs.r[rn] + op2;
    // first we cast to s8 to read the data as a signed byte and then cast to s32 to sign extend to a 32 bit integer
    u32 data = (s32)(s8)read_byte(address);

    regs.r[rd] = data;

    regs.r[15] += 4;
}

void ARM::arm_ldr_pre(u32 op2) {
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

void ARM::arm_ldr_post(u32 op2) {
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

void ARM::arm_strb_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    write_byte(regs.r[rn] + op2, regs.r[rd]);



    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] += op2;
    }

    

    regs.r[15] += 4;
}

void ARM::arm_strb_post(u32 op2) {
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

void ARM::arm_str_pre(u32 op2) {
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

void ARM::arm_str_post(u32 op2) {
    
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

void ARM::arm_msr_reg() {
    // user programs cant change cpsr
    u8 rm = opcode & 0xF;
    u32 mask = 0;
    for (int i = 0; i < 4; i++) {
        if (get_bit(i + 16, regs.r[rm])) {
            mask |= (0xFF << (i * 8));
        }
    }
    
    
    if (get_bit(22, opcode)) {
        regs.spsr = (regs.r[rm] & mask) | (regs.spsr & ~mask);
    } else {
        
        // can only update bits 0..23 in a privileged mode (not usr mode)
        if ((regs.cpsr & 0x1F) == 0x10) {
            // remove way to change bits 0..23
            mask &= 0xFF000000;
            regs.cpsr = (regs.r[rm] & mask) | (regs.cpsr & ~0xFF000000);
            return;
        } else {
            
            // proceed with setting cpsr normally
            // first change bits 0..7
            if (get_bit(16, opcode)) {
                update_mode(regs.r[rm] & 0x1F);
            }

            // set bits 5..7 too
            regs.cpsr = (regs.cpsr & ~0xE0) | (regs.r[rm] & 0xE0);

            // then change bits 8..23
            for (int i = 0; i < 3; i++) {
                if (get_bit(17 + i, opcode)) {
                    regs.cpsr = ((0xFF << ((i + 1) * 8)) & regs.r[rm]) | (regs.cpsr & ~(0xFF << ((i + 1) * 8)));
                }
            }
        }
    }
    regs.r[15] += 4;

}

void ARM::arm_msr_imm() {
    // user programs cant change cpsr
    u32 immediate = arm_imm_data_processing();
    u32 mask = 0;
    for (int i = 0; i < 4; i++) {
        if (get_bit(i + 16, immediate)) {
            mask |= (0xFF << (i * 8));
        }
    }

    
    if (get_bit(22, opcode)) {
        regs.spsr = (immediate & mask) | ~(mask & regs.spsr);
    } else {
        
        // can only update bits 0..23 in a privileged mode (not usr mode)
        if ((regs.cpsr & 0x1F) == 0x10) {
            // remove way to change bits 0..23
            mask &= 0xFF000000;
            regs.cpsr = (immediate & mask) | (regs.cpsr & ~0xFF000000);
            return;
        } else {
            
            // proceed with setting cpsr normally
            // first change bits 0..7
            if (get_bit(16, opcode)) {
                update_mode(immediate);
            }

            // set bits 5..7 too
            regs.cpsr = (regs.cpsr & ~0xE0) | (immediate & 0xE0);

            // then change bits 8..23
            for (int i = 0; i < 3; i++) {
                if (get_bit(17 + i, opcode)) {
                    regs.cpsr = ((0xFF << ((i + 1) * 8)) & immediate) | (regs.cpsr & ~(0xFF << ((i + 1) * 8)));
                }
            }
        }
    }

    regs.r[15] += 4;

}

void ARM::arm_mrs_cpsr() {
    u8 rd = (opcode >> 12) & 0xF;
    // move the cpsr to the register rd
    regs.r[rd] = regs.cpsr;

    regs.r[15] += 4;
}

void ARM::arm_mrs_spsr() {
    u8 rd = (opcode >> 12) & 0xF;
    // move the spsr into the register rd
    regs.r[rd] = regs.spsr;

    regs.r[15] += 4;
}


void ARM::arm_strh_pre(u32 op2) {
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

    write_halfword(address, regs.r[rd]);
    
    regs.r[15] += 4;
}

void ARM::arm_strh_post(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    if (rd == 15) {
        log_fatal("handle");
    }

    write_halfword(regs.r[rn], regs.r[rd]);

    // always write back to base register in post indexing
    regs.r[rn] += op2;

    regs.r[15] += 4;
}

void ARM::arm_ldrh_pre(u32 op2) {
    u8 rd = (opcode >> 12) & 0xF;
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn] + op2;
    // check for writeback
    if (get_bit(21, opcode)) {
        // write back to rn
        regs.r[rn] = address;
    }

    regs.r[rd] = read_halfword(address);

    

    regs.r[15] += 4;
}

u32 ARM::arm_imm_halfword_signed_data_transfer() {
    return ((opcode >> 4) & 0xF0) | (opcode & 0xF);
}

void ARM::arm_mcr_reg() {
    // arm9 exclusive as it involves coprocessor transfers
    if (cpu_id == ARMv4) {
        return;
    }

    u8 crm = opcode & 0xF;
    u8 crn = (opcode >> 16) & 0xF;
    u8 opcode2 = (opcode >> 5) & 0x7;
    u8 rd = (opcode >> 12) & 0xF;
    nds->cp15.write_reg(crn, crm, opcode2, regs.r[rd]);

    regs.r[15] += 4;
}

void ARM::arm_mrc_reg() {
    // arm9 exclusive as it involves coprocessor transfers
    if (cpu_id == ARMv4) {
        return;
    }

    u8 crm = opcode & 0xF;
    u8 crn = (opcode >> 16) & 0xF;
    u8 opcode2 = (opcode >> 5) & 0x7;
    u8 rd = (opcode >> 12) & 0xF;
    u32 data = nds->cp15.read_reg(crn, crm, opcode2);
    if (rd == 15) {
        // set flags instead
        regs.cpsr = (data & 0xF0000000) | (regs.cpsr & 0x0FFFFFFF);
    } else {
        // set rd normally
        regs.r[rd] = data;
    }

    regs.r[15] += 4;
}

void ARM::arm_stmiaw() {
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
void ARM::arm_stmibw() {
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
void ARM::arm_stmdbw() {
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
void ARM::arm_stmdaw() {
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

void ARM::arm_ldmiaw() {
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

void ARM::arm_ldmia() {
    u8 rn = (opcode >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (opcode & (1 << i)) {
            regs.r[i] = read_word(address);
            address += 4;
        }
    }
    
    if (get_bit(15, opcode)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}



void ARM::arm_ldmibw() {
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



void ARM::arm_ldmdbw() {
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

void ARM::arm_ldmdaw() {
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

void ARM::arm_ldmiauw() {
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

void ARM::arm_ldmdauw() {
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

void ARM::arm_ldmibuw() {
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

void ARM::arm_ldmdbuw() {
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



u32 ARM::arm_reg_halfword_signed_data_transfer() {
    return regs.r[opcode & 0xF];
}


u32 ARM::arm_rpll() {
    u8 rm = opcode & 0xF;
    u8 shift_amount = (opcode >> 7) & 0x1F;
    
    return regs.r[rm] << shift_amount;
}

u32 ARM::arm_rplr() {
    u8 rm = opcode & 0xF;
    u8 shift_amount = (opcode >> 7) & 0x1F;
    if (shift_amount == 0) {
        return 0;
    } else {
        return regs.r[rm] >> shift_amount;
    }
}

u32 ARM::arm_rpar() {
    u8 rm = opcode & 0xF;
    u8 shift_amount = (opcode >> 7) & 0x1F;
    u8 msb = opcode >> 31;

    if (shift_amount == 0) {
        return 0xFFFFFFFF * msb;
    } else {
        return (regs.r[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    }
}

u32 ARM::arm_rprr() {
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








// thumb instructions start here

void ARM::thumb_ldrpc_imm() {
    u32 immediate = (opcode & 0xFF) << 2;
    u8 rd = (opcode >> 8) & 0x7;
    // in this instruction pc is word aligned (pc & 0xFFFFFFFC)
    u32 address = (regs.r[15] & ~0x3) + immediate;
    regs.r[rd] = read_word(address);
    // log_fatal("address is 0x%04x", address);
    if (address & 0x3) {
        u8 shift_amount = (address & 0x3) * 8;
        regs.r[rd] = rotate_right(regs.r[rd], shift_amount);
    }

    regs.r[15] += 2;
}

void ARM::thumb_str_reg() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;
    u8 rm = (opcode >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    write_word(address, regs.r[rd]);

    regs.r[15] += 2;
}

void ARM::thumb_push_lr() {
    // - 4 from r13 as lr is always going to be pushed in this instruction
    u32 address = regs.r[13];

    for (int i = 0; i < 8; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }

    address -= 4;

    // set r13
    regs.r[13] = address;

    for (int i = 0; i < 8; i++) {
        if (get_bit(i, opcode)) {
            write_word(address, regs.r[i]);
            address += 4;
        }
    }

    // write lr to stack too
    write_word(address, regs.r[14]);
    address += 4;

    

    regs.r[15] += 2;
}

void ARM::thumb_strh_imm5() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;

    u32 immediate = (opcode >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate * 2);
    write_halfword(address, regs.r[rd]);

    regs.r[15] += 2;
}

void ARM::thumb_ldrh_imm5() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;
    u32 immediate_5 = (opcode >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate_5 * 2);

    regs.r[rd] = read_halfword(address);

    regs.r[15] += 2;
}

void ARM::thumb_str_imm5() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;
    u32 immediate = (opcode >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate * 4);

    write_word(address, regs.r[rd]);

    regs.r[15] += 2;
}

void ARM::thumb_strb_imm5() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;

    u32 immediate = (opcode >> 6) & 0x1F;


    u32 address = regs.r[rn] + immediate;
    write_byte(address, regs.r[rd] & 0xFF);

    regs.r[15] += 2;
}

void ARM::thumb_strsp_reg() {
    u8 rd = (opcode >> 8) & 0x7;
    u32 immediate = opcode & 0xFF;
    u32 address = regs.r[13] + (immediate * 4);
    write_word(address, regs.r[rd]);

    regs.r[15] += 2;
}

void ARM::thumb_pop() {
    u32 address = regs.r[13];

    for (int i = 0; i < 8; i++) {
        if (get_bit(i, opcode)) {
            regs.r[i] = read_word(address);
            address += 4;
        }
    }
    
    // writeback to r13
    regs.r[13] = address;

    regs.r[15] += 2;
}

void ARM::thumb_pop_pc() {
    u32 address = regs.r[13];

    for (int i = 0; i < 8; i++) {
        if (get_bit(i, opcode)) {
            regs.r[i] = read_word(address);
            address += 4;
        }
    }

    // now handle r15 stuff
    regs.r[15] = read_word(address);
    
    address += 4;
    
    // writeback to r13
    regs.r[13] = address;

    // now to handle mode switch and stuff
    // if cpu is armv4 or bit 0 of r15 is 1 stay in thumb state
    if ((cpu_id == ARMv4) || (regs.r[15] & 0x1)) {
        // halfword align r15 and flush pipeline
        regs.r[15] &= ~1;
        thumb_flush_pipeline();
    } else {
        // clear bit 5 of cpsr to switch to arm state
        regs.cpsr &= ~(1 << 5);
        regs.r[15] &= ~3;
        arm_flush_pipeline();
    }
}

void ARM::thumb_ldrsp_reg() {
    u32 immediate = opcode & 0xFF;
    u8 rd = (opcode >> 8) & 0x7;
    u32 address = regs.r[13] + (immediate * 4);
    if (address & 0x3) {
        log_fatal("uhhh");
    } else {
        regs.r[rd] = read_word(address);
    }

    regs.r[15] += 2;
}

void ARM::thumb_ldrb_imm5() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;

    u32 immediate = (opcode >> 6) & 0x1F;


    u32 address = regs.r[rn] + immediate;
    regs.r[rd] = read_byte(address);

    regs.r[15] += 2;
}

void ARM::thumb_push() {
    u32 address = regs.r[13];

    for (int i = 0; i < 8; i++) {
        if (get_bit(i, opcode)) {
            address -= 4;
        }
    }

    // set r13
    regs.r[13] = address;

    for (int i = 0; i < 8; i++) {
        if (get_bit(i, opcode)) {
            write_word(address, regs.r[i]);
            address += 4;
        }
    }

    regs.r[15] += 2;
}

void ARM::thumb_ldrh_reg() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;
    u8 rm = (opcode >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    regs.r[rd] = read_halfword(address);

    regs.r[15] += 2;
}

void ARM::thumb_ldr_imm5() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;
    u32 immediate = (opcode >> 6) & 0x1F;
    u32 address = regs.r[rn] + (immediate * 4);

    regs.r[rd] = read_word(address);

    if (address & 0x3) {
        u8 shift_amount = (address & 0x3) * 8;
        regs.r[rd] = rotate_right(regs.r[rd], shift_amount);
    }

    regs.r[15] += 2;
}

void ARM::thumb_stmia_reg() {
    u8 rn = (opcode >> 8) & 0x7;
    u32 address = regs.r[rn];

    for (int i = 0; i < 8; i++) {
        if (get_bit(i, opcode)) {
            write_word(address, regs.r[i]);
            address += 4;
        }
    }

    // writeback to rn

    regs.r[rn] = address;

    regs.r[15] += 2;
}

void ARM::thumb_ldmia_reg() {
    u8 rn = (opcode >> 8) & 0x3;
    u32 address = regs.r[rn];

    for (int i = 0; i < 8; i++) {
        if (get_bit(i, opcode)) {
            regs.r[i] = read_word(address);
            address += 4;
        }
    }

    // writeback to rn

    regs.r[rn] = address;

    regs.r[15] += 2;
}

void ARM::thumb_ldr_reg() {
    u8 rd = opcode & 0x7;
    u8 rn = (opcode >> 3) & 0x7;
    u8 rm = (opcode >> 6) & 0x7;

    u32 address = regs.r[rn] + regs.r[rm];

    regs.r[rd] = read_word(address);

    if (address & 0x3) {
        u8 shift_amount = (address & 0x3) * 8;
        regs.r[rd] = rotate_right(regs.r[rd], shift_amount);
    }

    regs.r[15] += 2;
}