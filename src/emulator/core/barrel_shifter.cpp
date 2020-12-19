#include <emulator/core/arm.h>
#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>

// LSL rm (performs a logical shift left on the register rm (occurs when I bit is 0 in data processing))
u32 ARM::lli() {
    // get the actual data of the register specified by bits 0..3
    u32 rm = get_reg(opcode & 0xF);

    // get the shift
    int shift = get_bit_range(4, 11, opcode);

    // perform the LSL
    return (rm << shift);

}

// LSR rm (peforms a logical shift right on the register rm (occurs when I bit is 0 in data processing))
u32 ARM::lri() {
    // get the actual data of the register specified by bits 0..3
    u32 rm = get_reg(opcode & 0xF);

    // get the shift
    int shift = get_bit_range(4, 11, opcode);

    // perform the LSR
    return (rm >> shift);

}