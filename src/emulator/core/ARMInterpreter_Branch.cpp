#include <emulator/core/ARMInterpreter.h>
#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>
#include <emulator/common/log.h>

void ARMInterpreter::arm_b() {
    u32 offset = (get_bit(23, opcode) ? 0xFF000000: 0) | ((opcode & 0xFFFFFF) << 2);
    // use offset - 4 as we perform the prefetch before the instruction is executed
    regs.r[15] += offset;
    flush_pipeline();
}

void ARMInterpreter::arm_bl() {
    u32 offset = (get_bit(23, opcode) ? 0xFF000000: 0) | ((opcode & 0xFFFFFF) << 2);
    // store the address of the instruction after the branch into the link register
    regs.r[14] = regs.r[15] - 4;
    
    
    // use offset - 4 as we perform the prefetch before the instruction is executed
    regs.r[15] += offset;
    flush_pipeline();
}

void ARMInterpreter::arm_bx() {
    u8 rm = opcode & 0xF;
    if ((regs.r[rm] & 0x1) == 1) {
        // set bit 5 of cpsr to switch to thumb state
        regs.cpsr |= (1 << 5);
        regs.r[15] = regs.r[rm] & ~1;
    } else {
        // clear bit 5 of cpsr to switch to arm state
        regs.cpsr &= ~(1 << 5);
        regs.r[15] = regs.r[rm] & ~3;
    }
    
    flush_pipeline();
}

// thumb instructions start here

void ARMInterpreter::thumb_bl_setup() {
    u32 immediate_11 = (get_bit(10, opcode) ? 0xFFFFF000 : 0) | ((opcode & 0x7FF) << 1);

    regs.r[14] = regs.r[15] + (immediate_11 << 11);

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_bl_offset() {
    u32 offset_11 = (opcode & 0x7FF) << 1;
    u32 next_instruction_address = regs.r[15] - 2;
    regs.r[15] = (regs.r[14] + offset_11) & ~1;
    regs.r[14] = next_instruction_address | 1;
    flush_pipeline();
}

void ARMInterpreter::thumb_bcs() {
    // only branch if carry flag set
    if (get_condition_flag(C_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARMInterpreter::thumb_bcc() {
    // only branch if carry flag cleared
    if (!get_condition_flag(C_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARMInterpreter::thumb_beq() {
    // only branch if zero flag set
    if (get_condition_flag(Z_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARMInterpreter::thumb_bne() {
    // only branch if zero flag clear
    if (!get_condition_flag(Z_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARMInterpreter::thumb_bmi() {
    // only branch if negative flag set
    if (get_condition_flag(N_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARMInterpreter::thumb_bpl() {
    // only branch if negative flag clear
    if (!get_condition_flag(N_FLAG)) {
        u32 offset = (get_bit(7, opcode) ? 0xFFFFFE00 : 0) | ((opcode & 0xFF) << 1);
        regs.r[15] += offset;
        flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}


void ARMInterpreter::thumb_b() {
    u32 offset = (get_bit(10, opcode) ? 0xFFFFFF00 : 0) | ((opcode & 0x7FF) << 1);
    regs.r[15] += offset;
    flush_pipeline();   
}

void ARMInterpreter::thumb_bx() {
    u8 rm = (opcode >> 3) & 0xF;

    if (regs.r[rm] & 0x1) {
        // just load rm into r15 normally in thumb
        regs.r[15] = regs.r[rm] & ~1;
    } else {
        // switch to arm state
        // clear bit 5 in cpsr
        regs.cpsr &= ~(1 << 5);
        regs.r[15] = regs.r[rm] & ~3;
    }

    flush_pipeline();
}

void ARMInterpreter::thumb_blx_offset() {
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
    flush_pipeline();
}

void ARMInterpreter::thumb_blx() {
    // arm9 specific instruction
    if (cpu_id == ARMv4) {
        return;
    }
    u8 rm = (opcode >> 3) & 0x7;

    u32 next_instruction_address = regs.r[15] - 2;
    regs.r[14] = next_instruction_address | 1;

    regs.r[15] = (regs.r[rm] & ~1) << 1;

    // change bit 5 of cpsr
    if (regs.r[rm] & 0x1) {
        // stay in thumb state
        // TODO: handle halfword alignment i think
        regs.cpsr |= (1 << 5);
    } else {
        // switch to arm state
        // TODO: handle word alignment i think
        regs.cpsr &= ~(1 << 5);
    }
    flush_pipeline();

}