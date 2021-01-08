#include <emulator/core/ARMInterpreter.h>
#include <emulator/common/types.h>
#include <emulator/common/log.h>
#include <emulator/common/arithmetic.h>

void ARMInterpreter::arm_msr_reg() {
    // user programs cant change cpsr
    u8 rm = opcode & 0xF;
    u32 mask = 0;
    for (int i = 0; i < 4; i++) {
        if (get_bit(i + 16, regs.r[rm])) {
            mask |= (0xFF << (i * 8));
        }
    }

    
    if (get_bit(22, opcode)) {
        u32 value = (regs.r[rm] & mask) | ~(mask & get_spsr());
        set_spsr(value);
    } else {
        
        // can only update bits 0..23 in a privileged mode (not usr mode)
        if ((regs.cpsr & 0x1F) == 0x10) {
            // remove way to change bits 0..23
            mask &= 0xFF000000;
            regs.cpsr = (regs.r[rm] & mask) | (regs.cpsr & ~0xFF000000);
            return;
        } else {
            
            // proceed with setting cpsr normally
            // first change bits 0..7
            if (get_bit(16, opcode)) {
                // set bits 5..7 too
                update_mode(regs.r[rm]);
            }

            // then change bits 8..23
            for (int i = 0; i < 3; i++) {
                if (get_bit(17 + i, opcode)) {
                    regs.cpsr = ((0xFF << ((i + 1) * 8)) & regs.r[rm]) | (regs.cpsr & ~(0xFF << ((i + 1) * 8)));
                }
            }
        }
    }

    regs.r[15] += 4;

}

void ARMInterpreter::arm_msr_imm() {
    // user programs cant change cpsr
    u32 immediate = arm_imm_data_processing();
    u32 mask = 0;
    for (int i = 0; i < 4; i++) {
        if (get_bit(i + 16, immediate)) {
            mask |= (0xFF << (i * 8));
        }
    }

    
    if (get_bit(22, opcode)) {
        u32 value = (immediate & mask) | ~(mask & get_spsr());
        set_spsr(value);
    } else {
        
        // can only update bits 0..23 in a privileged mode (not usr mode)
        if ((regs.cpsr & 0x1F) == 0x10) {
            // remove way to change bits 0..23
            mask &= 0xFF000000;
            regs.cpsr = (immediate & mask) | (regs.cpsr & ~0xFF000000);
            return;
        } else {
            
            // proceed with setting cpsr normally
            // first change bits 0..7
            if (get_bit(16, opcode)) {
                // set bits 5..7 too
                update_mode(immediate);
            }

            // then change bits 8..23
            for (int i = 0; i < 3; i++) {
                if (get_bit(17 + i, opcode)) {
                    regs.cpsr = ((0xFF << ((i + 1) * 8)) & immediate) | (regs.cpsr & ~(0xFF << ((i + 1) * 8)));
                }
            }
        }
    }

    regs.r[15] += 4;

}


void ARMInterpreter::arm_mrs_cpsr() {
    u8 rd = (opcode >> 12) & 0xF;
    // move the cpsr to the register rd
    regs.r[rd] = regs.cpsr;

    regs.r[15] += 4;
}
