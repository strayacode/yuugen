#include <common/arithmetic.h>
#include <common/types.h>
#include <nds/nds.h>
#include <nds/arm.h>

void ARM::arm_b() {
	u32 offset = (get_bit(23, opcode) ? 0xFF000000: 0) | ((opcode & 0xFFFFFF) << 2);
    // r15 is assumed to be 2 instructions in front
    regs.r[15] += offset;
    arm_flush_pipeline();
}

void ARM::arm_blx_offset() {
	// arm9 specific instruction
	if (cpu_id == ARMv4) {
		return;
	}

	// store address of instruction after blx instruction into lr
	regs.r[14] = regs.r[15] - 4;
	// set the t flag to 1 (switch to thumb mode)
	regs.cpsr |= (1 << 5);

	u32 offset = ((get_bit(23, opcode) ? 0xFF000000: 0) | ((opcode & 0xFFFFFF) << 2)) + (get_bit(24, opcode) << 1);
	regs.r[15] += offset;
	// since we switched to thumb mode do a 16 bit pipeline flush
	thumb_flush_pipeline();
}


void ARM::arm_bl() {
    u32 offset = (get_bit(23, opcode) ? 0xFF000000: 0) | ((opcode & 0xFFFFFF) << 2);
    // store the address of the instruction after the branch into the link register
    regs.r[14] = regs.r[15] - 4;
    
    
    // use offset - 4 as we perform the prefetch before the instruction is executed
    regs.r[15] += offset;
    arm_flush_pipeline();
}


void ARM::arm_bx() {
    u8 rm = opcode & 0xF;
    if ((regs.r[rm] & 0x1) == 1) {
        // set bit 5 of cpsr to switch to thumb state
        regs.cpsr |= (1 << 5);
        regs.r[15] = regs.r[rm] & ~1;
        thumb_flush_pipeline();
    } else {
        // clear bit 5 of cpsr to switch to arm state
        regs.cpsr &= ~(1 << 5);
        regs.r[15] = regs.r[rm] & ~3;
        arm_flush_pipeline();
    }
}

void ARM::arm_blx_reg() {
	// arm9 exclusive instruction

	if (cpu_id == ARMv4) {
		return;
	}

	u8 rm = opcode & 0xF;

	u32 next_instruction_address = regs.r[15] - 4;
	// store into lr
	regs.r[14] = next_instruction_address;
	// set t flag to rm[0]
	if ((regs.r[rm] & 0x1) == 1) {
		// set bit 5 of cpsr to switch to thumb state and flush pipeline accordingly
		regs.cpsr |= (1 << 5);
		regs.r[15] = regs.r[rm] & ~1;
		thumb_flush_pipeline();
	} else {
		// clear bit 5 of cpsr to switch to arm state
        regs.cpsr &= ~(1 << 5);
        regs.r[15] = regs.r[rm] & ~3;
        arm_flush_pipeline();
	}
}

void ARM::arm_swi() {
    // store the address of the next instruction after the swi in lr_svc
    regs.r_banked[BANK_SVC][6] = regs.r[15] - 4;

    // store the cpsr in spsr_svc
    regs.spsr_banked[BANK_SVC] = regs.cpsr;

    // enter supervisor mode
    update_mode(SVC);

    // always execute in arm state
    regs.cpsr &= ~(1 << 5);

    // fiq interrupts state is unchanged

    // disable normal interrupts
    regs.cpsr |= (1 << 7);
    

    // regs.r[14] = regs.r[15] - 4;
    // check the exception base and jump to the correct address in the bios
    // also only use cp15 exception base from control register if arm9
    regs.r[15] = ((cpu_id) ? nds->cp15.get_exception_base() : 0x00000000) + 0x08;
    
    arm_flush_pipeline();
}




// start of thumb instructions

void ARM::thumb_bl_setup() {
    u32 immediate_11 = (get_bit(10, opcode) ? 0xFFFFF000 : 0) | ((opcode & 0x7FF) << 1);

    regs.r[14] = regs.r[15] + (immediate_11 << 11);

    regs.r[15] += 2;
}

void ARM::thumb_bl_offset() {
    u32 offset_11 = (opcode & 0x7FF) << 1;
    u32 next_instruction_address = regs.r[15] - 2;
    regs.r[15] = (regs.r[14] + offset_11) & ~1;
    regs.r[14] = next_instruction_address | 1;
    thumb_flush_pipeline();
}


void ARM::thumb_bgt() {
	// only branch if z cleared AND (n equals v)
    if ((!get_condition_flag(Z_FLAG) && (get_condition_flag(N_FLAG) == get_condition_flag(V_FLAG)))) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_blt() {
	// only branch if n not equal to v
    if (get_condition_flag(N_FLAG) != get_condition_flag(V_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_bne() {
    // only branch if zero flag clear
    if (!get_condition_flag(Z_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_blx_reg() {
	// arm9 specific instruction
    if (cpu_id == ARMv4) {
        return;
    }

    u32 offset_11 = (opcode & 0x7FF) << 1;
    u32 next_instruction_address = regs.r[15] - 2;
    regs.r[15] = (regs.r[14] + offset_11) & ~3;
    regs.r[14] = next_instruction_address | 1;
    // set t flag to 0 (switch to arm mode)
    regs.cpsr &= ~(1 << 5);
    // flush the pipeline
    arm_flush_pipeline();
}

void ARM::thumb_bx_reg() {
    u8 rm = (opcode >> 3) & 0xF;
    if (regs.r[rm] & 0x1) {
        // just load rm into r15 normally in thumb
        regs.r[15] = regs.r[rm] & ~1;
        thumb_flush_pipeline();
    } else {
        // switch to arm state
        // clear bit 5 in cpsr
        regs.cpsr &= ~(1 << 5);
        regs.r[15] = regs.r[rm] & ~3;
        arm_flush_pipeline();
    }
}

void ARM::thumb_blx_offset() {
    // arm9 specific instruction
    if (cpu_id == ARMv4) {
        return;
    }

    u32 offset_11 = (opcode & 0x7FF) << 1;
    u32 next_instruction_address = regs.r[15] - 2;
    regs.r[15] = (regs.r[14] + offset_11) & 0xFFFFFFFC;
    regs.r[14] = next_instruction_address | 1;
    // set t flag to 0
    regs.cpsr &= ~(1 << 5);
    // flush the pipeline
    arm_flush_pipeline();
}

void ARM::thumb_b() {
    u32 offset = (get_bit(10, opcode) ? 0xFFFFF000 : 0) | ((opcode & 0x7FF) << 1);
    regs.r[15] += offset;
    thumb_flush_pipeline();   
}

void ARM::thumb_bcc() {
    // only branch if carry flag cleared
    if (!get_condition_flag(C_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_beq() {
    // only branch if zero flag set
    if (get_condition_flag(Z_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_bhi() {
    // only branch if c flag set and z flag cleared
    if (get_condition_flag(C_FLAG) && !get_condition_flag(Z_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_bpl() {
    // only branch if negative flag clear
    if (!get_condition_flag(N_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_bcs() {
    // only branch if carry flag set
    if (get_condition_flag(C_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_bmi() {
    // only branch if negative flag set
    if (get_condition_flag(N_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_ble() {
    
    // only branch on condition
    if ((get_condition_flag(Z_FLAG) || (get_condition_flag(N_FLAG) != get_condition_flag(V_FLAG)))) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        thumb_flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARM::thumb_swi() {
    // store the address of the next instruction after the swi in lr_svc
    regs.r_banked[BANK_SVC][6] = regs.r[15] - 2;

    // store the cpsr in spsr_svc
    regs.spsr_banked[BANK_SVC] = regs.cpsr;

    // enter supervisor mode
    update_mode(SVC);

    // always execute in arm state
    regs.cpsr &= ~(1 << 5);

    // fiq interrupts state is unchanged

    // disable normal interrupts
    regs.cpsr |= (1 << 7);
    

    // regs.r[14] = regs.r[15] - 4;
    // check the exception base and jump to the correct address in the bios
    regs.r[15] = ((cpu_id) ? nds->cp15.get_exception_base() : 0x00000000) + 0x08;
    
    arm_flush_pipeline();
}