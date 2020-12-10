#include <dmg/core/sm83.h>
#include <dmg/common/types.h>

void SM83::ld_sp_u16() {
    u8 lo_byte = memory.read_byte(regs.pc);
    tick();
    u8 hi_byte = memory.read_byte(regs.pc + 1);
    tick();
    regs.pc += 2;
    regs.sp = (hi_byte << 8 | lo_byte);
}

u8 SM83::xor_byte(u8 r1, u8 r2) {
    u8 result = r1 ^ r2;
    if (!result) {
        set_flag(Z_FLAG, 1);
    }
    set_flag(N_FLAG, 0);
    set_flag(H_FLAG, 0);
    set_flag(C_FLAG, 0);
    return result;
}