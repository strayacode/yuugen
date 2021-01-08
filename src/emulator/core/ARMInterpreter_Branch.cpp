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
    u32 immediate_11 = (get_bit(10, opcode) ? 0xFFFFF800 : 0) | (opcode & 0x7FF);
    u32 offset = immediate_11 << 12; 

    regs.r[14] = regs.r[15] + offset;

    regs.r[15] += 2;
}

void ARMInterpreter::thumb_bl_offset() {
    u32 offset_11 = (opcode & 0x7FF) << 1;
    u32 next_instruction_address = regs.r[15] + 2;
    regs.r[15] = regs.r[14] + offset_11;
    regs.r[14] = next_instruction_address | 1;
    flush_pipeline();
}

void ARMInterpreter::thumb_bcs() {
    // only branch if carry flag set
    if (get_condition_flag(C_FLAG)) {
        u32 offset = (s32)(s8)(opcode & 0xFF) << 1;
        regs.r[15] += offset;
        flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ARMInterpreter::thumb_bcc() {
    // only branch if carry flag cleared
    if (!get_condition_flag(C_FLAG)) {
        u32 offset = (s32)(s8)(opcode & 0xFF) << 1;
        regs.r[15] += offset;
        flush_pipeline();
    } else {
        regs.r[15] += 2;
    }
}