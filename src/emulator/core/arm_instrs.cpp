#include <emulator/core/arm.h>
#include <emulator/common/arithmetic.h>

void ARM::b() {
    if (evaluate_condition()) {
        // execute branch
        // offset is shifted left by 2 and sign extended to 32 bits
        u32 offset = (get_bit(23, opcode) ? 0xFF000000 : 0) | ((opcode & 0xFFFFFF) << 2);
        regs.r15 += offset;
        flush_pipeline();
    }
}