#include <emulator/core/arm9.h>
#include <stdio.h>
#include <emulator/common/arithmetic.h>

void ARM9::b() {
    if (evaluate_condition()) {
        // execute branch
        // offset is shifted left by 2 and sign extended to 32 bits
        u32 offset = (get_bit(23, opcode) ? 0xFF000000 : 0) | ((opcode & 0xFFFFFF) << 2);
        regs.r15 += offset;
        flush_pipeline();
    }
}