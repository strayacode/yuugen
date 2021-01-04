#include <emulator/core/ARMInterpreter.h>
#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>
#include <emulator/common/log.h>

void ARMInterpreter::b() {
    u32 offset = (get_bit(23, opcode) ? 0xFF000000: 0) | ((opcode & 0xFFFFFF) << 2);
    // use offset - 4 as we perform the prefetch before the instruction is executed
    regs.r[15] += offset;
    flush_pipeline();
}

void ARMInterpreter::bl() {
    u32 offset = (get_bit(23, opcode) ? 0xFF000000: 0) | ((opcode & 0xFFFFFF) << 2);
    // store the address of the instruction after the branch into the link register
    regs.r[14] = regs.r[15] - 4;
    
    
    // use offset - 4 as we perform the prefetch before the instruction is executed
    regs.r[15] += offset;
    flush_pipeline();
}

void ARMInterpreter::bx() {
    u8 rm = opcode & 0xF;
    if ((regs.r[rm] & 0x1) == 1) {
        log_fatal("thumb switching not implemented yet!");
    }
    regs.r[15] = regs.r[rm];
    flush_pipeline();
}