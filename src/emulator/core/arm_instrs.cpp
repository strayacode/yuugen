#include <emulator/core/arm.h>
#include <emulator/common/arithmetic.h>
#include <stdio.h>

void ARM::b() {
    if (evaluate_condition()) {
        // execute branch
        // offset is shifted left by 2 and sign extended to 32 bits
        u32 offset = (get_bit(23, opcode) ? 0xFF000000 : 0) | ((opcode & 0xFFFFFF) << 2);
        regs.r15 += offset;
        flush_pipeline();
    }
}

void ARM::mov_imm() {
    if (evaluate_condition()) {
        u8 rd = get_bit_range(12, 15, opcode);
        u8 shift = get_bit_range(8, 11, opcode);
        u8 immediate = opcode & 0xFF;
        u32 op2 = rotate_right(immediate, shift * 2);
        set_reg(rd, op2);
    }
}