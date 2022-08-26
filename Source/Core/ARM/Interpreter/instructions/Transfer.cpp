#include "Common/Bits.h"
#include "Core/ARM/Interpreter/Interpreter.h"

void Interpreter::arm_halfword_data_transfer() {
    const bool load = (m_instruction >> 20) & 0x1;
    const bool writeback = (m_instruction >> 21) & 0x1;
    const bool immediate = (m_instruction >> 22) & 0x1;
    const bool up = (m_instruction >> 23) & 0x1;
    const bool pre = (m_instruction >> 24) & 0x1;
    u8 rm = m_instruction & 0xF;
    u8 opcode = (m_instruction >> 5) & 0x3;
    u8 rd = (m_instruction >> 12) & 0xF;
    u8 rn = (m_instruction >> 16) & 0xF;

    u32 op2 = 0;
    u32 address = m_gpr[rn];

    bool do_writeback = !load || rd != rn;

    if (immediate) {
        op2 = ((m_instruction >> 4) & 0xF0) | (m_instruction & 0xF);
    } else {
        op2 = m_gpr[rm];
    }

    if (!up) {
        op2 *= -1;
    }

    if (pre) {
        address += op2;
    }

    m_gpr[15] += 4;

    switch (opcode) {
    case 0x1:
        if (load) {
            m_gpr[rd] = read_half(address);
        } else {
            write_half(address, m_gpr[rd]);
        }
        break;
    case 0x2:
        if (load) {
            m_gpr[rd] = Common::sign_extend<u32, 8>(read_byte(address));
        } else if (m_arch == Arch::ARMv5) {
            // cpu locks up when rd is odd
            if (rd & 0x1) {
                log_fatal("undefined ldrd exception");
            }

            m_gpr[rd] = read_word(address);
            m_gpr[rd + 1] = read_word(address + 4);

            // when rn == rd + 1 writeback is not performed
            do_writeback = rn != (rd + 1);

            // when rd == 14 the pipeline is flushed
            // due to writing to r15
            if (rd == 14) {
                arm_flush_pipeline();
            }
        }
        break;
    case 0x3:
        if (load) {
            m_gpr[rd] = Common::sign_extend<u32, 16>(read_half(address));
        } else if (m_arch == Arch::ARMv5) {
            // cpu locks up when rd is odd
            if (rd & 0x1) {
                log_fatal("undefined strd exception");
            }

            write_word(address, m_gpr[rd]);
            write_word(address + 4, m_gpr[rd + 1]);
        }
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }

    if (do_writeback) {
        if (!pre) {
            m_gpr[rn] += op2;
        } else if (writeback) {
            m_gpr[rn] = address;
        }
    }

    if (rd == 15) {
        log_fatal("handle");
    }
}

void Interpreter::arm_psr_transfer() {
    todo();
}

void Interpreter::arm_block_data_transfer() {
    todo();
}

void Interpreter::arm_single_data_transfer() {
    const bool load = (m_instruction >> 20) & 0x1;
    const bool writeback = (m_instruction >> 21) & 0x1;
    const bool byte = (m_instruction >> 22) & 0x1;
    const bool up = (m_instruction >> 23) & 0x1;
    const bool pre = (m_instruction >> 24) & 0x1;
    const bool shifted_register = (m_instruction >> 25) & 0x1;
    u8 rd = (m_instruction >> 12) & 0xF;
    u8 rn = (m_instruction >> 16) & 0xF;

    u32 op2 = 0;
    u32 address = m_gpr[rn];

    if (shifted_register) {
        op2 = arm_get_shifted_register_single_data_transfer();
    } else {
        op2 = m_instruction & 0xFFF;
    }

    if (!up) {
        op2 *= -1;
    }

    if (pre) {
        address += op2;
    }

    // increment r15 by 4 since if we are writing r15 it must be r15 + 12.
    // however if we are loading into r15 this increment won't matter as well
    m_gpr[15] += 4;

    if (load) {
        if (byte) {
            m_gpr[rd] = read_byte(address);
        } else {
            m_gpr[rd] = read_word_rotate(address);
        }
    } else {
        if (byte) {
            write_byte(address, m_gpr[rd]);
        } else {
            write_word(address, m_gpr[rd]);
        }
    }

    if (!load || rd != rn) {
        if (!pre) {
            m_gpr[rn] += op2;
        } else if (writeback) {
            m_gpr[rn] = address;
        }
    }

    if (load && rd == 15) {
        if ((m_arch == Arch::ARMv5) && (m_gpr[15] & 1)) {
            m_cpsr.t = true;
            m_gpr[15] &= ~1;
            thumb_flush_pipeline();
        } else {
            m_gpr[15] &= ~3;
            arm_flush_pipeline();
        }
    }
}

u32 Interpreter::arm_get_shifted_register_single_data_transfer() {
    u8 shift_type = (m_instruction >> 5) & 0x3;
    u8 shift_amount = (m_instruction >> 7) & 0x1F;
    u8 rm = m_instruction & 0xF;
    u32 op2 = 0;

    switch (shift_type) {
    case 0x0:
        // LSL
        op2 = m_gpr[rm] << shift_amount;
        break;
    case 0x1:
        // LSR
        if (shift_amount != 0) {
            op2 = m_gpr[rm] >> shift_amount;
        }
        break;
    case 0x2: {
        // ASR
        u8 msb = m_gpr[rm] >> 31;

        if (shift_amount == 0) {
            op2 = 0xFFFFFFFF * msb;
        } else {
            op2 = (m_gpr[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
        }
        break;
    }
    case 0x3:
        // ROR
        if (shift_amount == 0) {
            // rotate right extended
            op2 = (m_cpsr.c << 31) | (m_gpr[rm] >> 1);
        } else {
            // rotate right
            op2 = Common::rotate_right(m_gpr[rm], shift_amount);
        }
        break;
    }

    return op2;
}

void Interpreter::arm_coprocessor_register_transfer() {
    todo();
}

void Interpreter::thumb_push_pop() {
    todo();
}

void Interpreter::thumb_load_store() {
    todo();
}

void Interpreter::thumb_load_pc() {
    todo();
}

void Interpreter::thumb_load_store_sp_relative() {
    todo();
}

void Interpreter::thumb_load_store_halfword() {
    todo();
}

void Interpreter::thumb_load_store_multiple() {
    todo();
}

void Interpreter::thumb_load_store_immediate() {
    todo();
}