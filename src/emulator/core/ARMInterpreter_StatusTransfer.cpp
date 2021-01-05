#include <emulator/core/ARMInterpreter.h>
#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>

void ARMInterpreter::msr_reg() {
    // TODO: add correct mode changing later and fully implement behaviour of msr too
    u8 rm = opcode & 0xF;
    u32 mask = 0;
    for (int i = 0; i < 4; i++) {
        if (get_bit(i + 16, regs.r[rm])) {
            mask |= (0xFF << (i * 8));
        }
    }

    if (get_bit(22, opcode)) {
        set_spsr(regs.r[rm] & mask);
    } else {
        // only update bits 0..23 if in a privileged mode (not usr mode)
        if ((regs.cpsr & 0x1F) == 0x10) {
            // remove way to change bits 0..23
            mask &= 0xFF000000;
            regs.cpsr = regs.r[rm] & mask;

        }
    }
}
