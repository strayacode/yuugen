#include "core/arm/interpreter/interpreter.h"

namespace core::arm {

void Interpreter::arm_halfword_data_transfer() {
    // const bool load = (m_instruction >> 20) & 0x1;
    // const bool writeback = (m_instruction >> 21) & 0x1;
    // const bool immediate = (m_instruction >> 22) & 0x1;
    // const bool up = (m_instruction >> 23) & 0x1;
    // const bool pre = (m_instruction >> 24) & 0x1;
    // u8 rm = m_instruction & 0xF;
    // u8 opcode = (m_instruction >> 5) & 0x3;
    // u8 rd = (m_instruction >> 12) & 0xF;
    // u8 rn = (m_instruction >> 16) & 0xF;

    // u32 op2 = 0;
    // u32 address = m_gpr[rn];

    // bool do_writeback = !load || rd != rn;

    // if (immediate) {
    //     op2 = ((m_instruction >> 4) & 0xF0) | (m_instruction & 0xF);
    // } else {
    //     op2 = m_gpr[rm];
    // }

    // if (!up) {
    //     op2 *= -1;
    // }

    // if (pre) {
    //     address += op2;
    // }

    // m_gpr[15] += 4;

    // switch (opcode) {
    // case 0x1:
    //     if (load) {
    //         m_gpr[rd] = read_half(address);
    //     } else {
    //         write_half(address, m_gpr[rd]);
    //     }
    //     break;
    // case 0x2:
    //     if (load) {
    //         m_gpr[rd] = Common::sign_extend<u32, 8>(read_byte(address));
    //     } else if (m_arch == Arch::ARMv5) {
    //         // cpu locks up when rd is odd
    //         if (rd & 0x1) {
    //             log_fatal("undefined ldrd exception");
    //         }

    //         m_gpr[rd] = read_word(address);
    //         m_gpr[rd + 1] = read_word(address + 4);

    //         // when rn == rd + 1 writeback is not performed
    //         do_writeback = rn != (rd + 1);

    //         // when rd == 14 the pipeline is flushed
    //         // due to writing to r15
    //         if (rd == 14) {
    //             arm_flush_pipeline();
    //         }
    //     }
    //     break;
    // case 0x3:
    //     if (load) {
    //         m_gpr[rd] = Common::sign_extend<u32, 16>(read_half(address));
    //     } else if (m_arch == Arch::ARMv5) {
    //         // cpu locks up when rd is odd
    //         if (rd & 0x1) {
    //             log_fatal("undefined strd exception");
    //         }

    //         write_word(address, m_gpr[rd]);
    //         write_word(address + 4, m_gpr[rd + 1]);
    //     }
    //     break;
    // default:
    //     log_fatal("handle opcode %d", opcode);
    // }

    // if (do_writeback) {
    //     if (!pre) {
    //         m_gpr[rn] += op2;
    //     } else if (writeback) {
    //         m_gpr[rn] = address;
    //     }
    // }

    // if (rd == 15) {
    //     log_fatal("handle");
    // }
}

void Interpreter::arm_psr_transfer() {
    // const bool opcode = (m_instruction >> 21) & 0x1;
    // const bool spsr = (m_instruction >> 22) & 0x1;
    // u8 rm = m_instruction & 0xF;

    // if (opcode) {
    //     // msr
    //     u8 immediate = (m_instruction >> 25) & 0x1;
    //     u32 value = 0;

    //     u32 mask = 0;
    //     if (m_instruction & (1 << 16)) {
    //         mask |= 0x000000FF;
    //     }
    //     if (m_instruction & (1 << 17)) {
    //         mask |= 0x0000FF00;
    //     }
    //     if (m_instruction & (1 << 18)) {
    //         mask |= 0x00FF0000;
    //     }
    //     if (m_instruction & (1 << 19)) {
    //         mask |= 0xFF000000;
    //     }

    //     if (immediate) {
    //         u32 immediate = m_instruction & 0xFF;
    //         u8 rotate_amount = ((m_instruction >> 8) & 0xF) << 1;

    //         value = Common::rotate_right(immediate, rotate_amount);
    //     } else {
    //         value = m_gpr[rm];
    //     }

    //     // TODO: check later
    //     if (spsr) {
    //         if (has_spsr()) {
    //             set_spsr((get_spsr() & ~mask) | (value & mask));
    //         }
    //     } else {
    //         if (m_instruction & (1 << 16) && is_privileged()) {
    //             switch_mode(value & 0x1F);
    //         }

    //         m_cpsr.data = (m_cpsr.data & ~mask) | (value & mask);
    //     }
    // } else {
    //     // mrs
    //     u8 rd = (m_instruction >> 12) & 0xF;

    //     if (spsr) {
    //         m_gpr[rd] = get_spsr();
    //     } else {
    //         m_gpr[rd] = m_cpsr.data;
    //     }
    // }
    
    // m_gpr[15] += 4;
}

void Interpreter::arm_block_data_transfer() {
    // const bool load = (m_instruction >> 20) & 0x1;
    // const bool writeback = (m_instruction >> 21) & 0x1;
    // const bool load_psr = (m_instruction >> 22) & 0x1;
    // const bool up = (m_instruction >> 23) & 0x1;
    // bool pre = (m_instruction >> 24) & 0x1;
    // u8 rn = (m_instruction >> 16) & 0xF;
    // u8 r15_in_rlist = (m_instruction >> 15) & 0x1;
    // u32 address = m_gpr[rn];
    // u8 old_mode = m_cpsr.mode;
    // u16 rlist = m_instruction & 0xFFFF;
    // int first = 0;
    // int bytes = 0;
    // u32 new_base = 0;

    // if (rlist != 0) {
    //     for (int i = 15; i >= 0; i--) {
    //         if (rlist & (1 << i)) {
    //             first = i;
    //             bytes += 4;
    //         }
    //     }
    // } else {
    //     // handle empty rlist
    //     bytes = 0x40;

    //     if (m_arch == Arch::ARMv4) {
    //         // only r15 gets transferred
    //         rlist = 1 << 15;
    //         r15_in_rlist = true;
    //     }
    // }

    // if (up) {
    //     new_base = address + bytes;
    // } else {
    //     pre = !pre;
    //     address -= bytes;
    //     new_base = address;
    // }

    // // increment r15 before doing transfer because if r15 in rlist and stm is used,
    // // the value written is the address of the stm instruction + 12
    // m_gpr[15] += 4;

    // // stm armv4: store old base if rb is first in rlist, otherwise store new base
    // // stm armv5: always store old base
    // if (writeback && !load) {
    //     if ((m_arch == Arch::ARMv4) && (first != rn)) {
    //         m_gpr[rn] = new_base;
    //     }
    // }

    // // make sure to only do user bank transfer if r15 is not in rlist or instruction is not ldm
    // // ~(A and B) = ~A or ~B
    // bool user_switch_mode = load_psr && (!load || !r15_in_rlist);

    // if (user_switch_mode) {
    //     switch_mode(MODE_USR);
    // }

    // // registers are transferred in order from lowest to highest
    // for (int i = first; i < 16; i++) {
    //     if (!(rlist & (1 << i))) {
    //         continue;
    //     }

    //     if (pre) {
    //         address += 4;
    //     }

    //     if (load) {
    //         m_gpr[i] = read_word(address);
    //     } else {
    //         write_word(address, m_gpr[i]);
    //     }

    //     if (!pre) {
    //         address += 4;
    //     } 
    // }

    // if (writeback) {
    //     // ldm armv4: writeback if rb is not in rlist
    //     // ldm armv5: writeback if rb is only register or not the last register in rlist
    //     if (load) {
    //         if (m_arch == Arch::ARMv5) {
    //             if ((rlist == (1 << rn)) || !((rlist >> rn) == 1)) {
    //                 m_gpr[rn] = new_base;
    //             }
    //         } else {
    //             if (!(rlist & (1 << rn))) {
    //                 m_gpr[rn] = new_base;
    //             }
    //         }
    //     } else {
    //         m_gpr[rn] = new_base;
    //     }
    // } 

    // if (user_switch_mode) {
    //     // switch back to old mode at the end if user bank transfer
    //     switch_mode(old_mode);

    //     if (load && r15_in_rlist) {
    //         todo();
    //     }
    // }

    // if (load && r15_in_rlist) {
    //     if ((m_arch == Arch::ARMv5) && (m_gpr[15] & 0x1)) {
    //         m_cpsr.t = true;
    //         thumb_flush_pipeline();
    //     } else {
    //         arm_flush_pipeline();
    //     }
    // }
}

void Interpreter::arm_single_data_transfer() {
    // const bool load = (m_instruction >> 20) & 0x1;
    // const bool writeback = (m_instruction >> 21) & 0x1;
    // const bool byte = (m_instruction >> 22) & 0x1;
    // const bool up = (m_instruction >> 23) & 0x1;
    // const bool pre = (m_instruction >> 24) & 0x1;
    // const bool shifted_register = (m_instruction >> 25) & 0x1;
    // u8 rd = (m_instruction >> 12) & 0xF;
    // u8 rn = (m_instruction >> 16) & 0xF;

    // u32 op2 = 0;
    // u32 address = m_gpr[rn];

    // if (shifted_register) {
    //     op2 = arm_get_shifted_register_single_data_transfer();
    // } else {
    //     op2 = m_instruction & 0xFFF;
    // }

    // if (!up) {
    //     op2 *= -1;
    // }

    // if (pre) {
    //     address += op2;
    // }

    // // increment r15 by 4 since if we are writing r15 it must be r15 + 12.
    // // however if we are loading into r15 this increment won't matter as well
    // m_gpr[15] += 4;

    // if (load) {
    //     if (byte) {
    //         m_gpr[rd] = read_byte(address);
    //     } else {
    //         m_gpr[rd] = read_word_rotate(address);
    //     }
    // } else {
    //     if (byte) {
    //         write_byte(address, m_gpr[rd]);
    //     } else {
    //         write_word(address, m_gpr[rd]);
    //     }
    // }

    // if (!load || rd != rn) {
    //     if (!pre) {
    //         m_gpr[rn] += op2;
    //     } else if (writeback) {
    //         m_gpr[rn] = address;
    //     }
    // }

    // if (load && rd == 15) {
    //     if ((m_arch == Arch::ARMv5) && (m_gpr[15] & 1)) {
    //         m_cpsr.t = true;
    //         m_gpr[15] &= ~1;
    //         thumb_flush_pipeline();
    //     } else {
    //         m_gpr[15] &= ~3;
    //         arm_flush_pipeline();
    //     }
    // }
}

u32 Interpreter::arm_get_shifted_register_single_data_transfer() {
    // u8 shift_type = (m_instruction >> 5) & 0x3;
    // u8 shift_amount = (m_instruction >> 7) & 0x1F;
    // u8 rm = m_instruction & 0xF;
    // u32 op2 = 0;

    // switch (shift_type) {
    // case 0x0:
    //     // LSL
    //     op2 = m_gpr[rm] << shift_amount;
    //     break;
    // case 0x1:
    //     // LSR
    //     if (shift_amount != 0) {
    //         op2 = m_gpr[rm] >> shift_amount;
    //     }
    //     break;
    // case 0x2: {
    //     // ASR
    //     u8 msb = m_gpr[rm] >> 31;

    //     if (shift_amount == 0) {
    //         op2 = 0xFFFFFFFF * msb;
    //     } else {
    //         op2 = (m_gpr[rm] >> shift_amount) | ((0xFFFFFFFF * msb) << (32 - shift_amount));
    //     }
    //     break;
    // }
    // case 0x3:
    //     // ROR
    //     if (shift_amount == 0) {
    //         // rotate right extended
    //         op2 = (m_cpsr.c << 31) | (m_gpr[rm] >> 1);
    //     } else {
    //         // rotate right
    //         op2 = Common::rotate_right(m_gpr[rm], shift_amount);
    //     }
    //     break;
    // }

    // return op2;
    return 0;
}

void Interpreter::arm_coprocessor_register_transfer() {
    // if (m_arch == Arch::ARMv4) {
    //     return;
    // }

    // u8 crm = m_instruction & 0xF;
    // u8 crn = (m_instruction >> 16) & 0xF;
    // u8 opcode2 = (m_instruction >> 5) & 0x7;
    // u8 rd = (m_instruction >> 12) & 0xF;

    // if (m_instruction & (1 << 20)) {
    //     m_gpr[rd] = m_coprocessor.read(crn, crm, opcode2);

    //     if (rd == 15) {
    //         todo();
    //     }
    // } else {
    //     m_coprocessor.write(crn, crm, opcode2, m_gpr[rd]);
    // }

    // m_gpr[15] += 4;
}

void Interpreter::thumb_load_pc() {
    // u32 immediate = (m_instruction & 0xFF) << 2;
    // u8 rd = (m_instruction >> 8) & 0x7;

    // // bit 1 of pc is always set to 0
    // u32 address = (m_gpr[15] & ~0x2) + immediate;
    // m_gpr[rd] = read_word(address);
    
    // m_gpr[15] += 2;
}

void Interpreter::thumb_load_store() {
    // u8 rd = m_instruction & 0x7;
    // u8 rn = (m_instruction >> 3) & 0x7;
    // u8 rm = (m_instruction >> 6) & 0x7;

    // u8 opcode = (m_instruction >> 10) & 0x3;
    // bool sign = m_instruction & (1 << 9);
    // u32 address = m_gpr[rn] + m_gpr[rm];

    // if (sign) {
    //     switch (opcode) {
    //     case 0x0:
    //         write_half(address, m_gpr[rd]);
    //         break;
    //     case 0x1:
    //         m_gpr[rd] = Common::sign_extend<u32, 8>(read_byte(address));
    //         break;
    //     case 0x2:
    //         m_gpr[rd] = read_half(address);
    //         break;
    //     case 0x3:
    //         m_gpr[rd] = Common::sign_extend<u32, 16>(read_half(address));
    //         break;
    //     }    
    // } else {
    //     switch (opcode) {
    //     case 0x0:
    //         write_word(address, m_gpr[rd]);
    //         break;
    //     case 0x1:
    //         write_byte(address, m_gpr[rd]);
    //         break;
    //     case 0x2: {
    //         m_gpr[rd] = read_word(address);

    //         if (address & 0x3) {
    //             int shift_amount = (address & 0x3) * 8;
    //             m_gpr[rd] = (m_gpr[rd] << (32 - shift_amount)) | (m_gpr[rd] >> shift_amount);
    //         }

    //         break;
    //     }
    //     case 0x3:
    //         m_gpr[rd] = read_byte(address);
    //         break;
    //     }
    // }
    
    // m_gpr[15] += 2;
}

void Interpreter::thumb_load_store_immediate() {
    // u8 rd = m_instruction & 0x7;
    // u8 rn = (m_instruction >> 3) & 0x7;
    // u32 immediate = (m_instruction >> 6) & 0x1F;

    // u8 opcode = (m_instruction >> 11) & 0x3;

    // switch (opcode) {
    // case 0x0:
    //     write_word(m_gpr[rn] + (immediate << 2), m_gpr[rd]);
    //     break;
    // case 0x1:
    //     m_gpr[rd] = read_word_rotate(m_gpr[rn] + (immediate << 2));
    //     break;
    // case 0x2:
    //     write_byte(m_gpr[rn] + immediate, m_gpr[rd]);
    //     break;
    // case 0x3:
    //     m_gpr[rd] = read_byte(m_gpr[rn] + immediate);
    //     break;
    // }
    
    // m_gpr[15] += 2;
}

void Interpreter::thumb_push_pop() {
    // bool pclr = (m_instruction >> 8) & 0x1;
    // bool pop = (m_instruction >> 11) & 0x1;

    // u32 address = m_gpr[13];

    // if (pop) {
    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             m_gpr[i] = read_word(address);
    //             address += 4;
    //         }
    //     }

    //     if (pclr) {
    //         m_gpr[15] = read_word(address);
    //         address += 4;

    //         if ((m_arch == Arch::ARMv4) || (m_gpr[15] & 0x1)) {
    //             // halfword align r15 and flush pipeline
    //             m_gpr[15] &= ~1;
    //             thumb_flush_pipeline();
    //         } else {
    //             // clear bit 5 of cpsr to switch to arm state
    //             m_cpsr.t = false;
    //             m_gpr[15] &= ~3;
    //             arm_flush_pipeline();
    //         }
    //     } else {
    //         m_gpr[15] += 2;
    //     }

    //     m_gpr[13] = address;
    // } else {
    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             address -= 4;
    //         }
    //     }

    //     if (pclr) {
    //         address -= 4;
    //     }

    //     m_gpr[13] = address;

    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             write_word(address, m_gpr[i]);
    //             address += 4;
    //         }
    //     }

    //     if (pclr) {
    //         write_word(address, m_gpr[14]);
    //     }

    //     m_gpr[15] += 2;
    // }
}

void Interpreter::thumb_load_store_sp_relative() {
    // u32 immediate = m_instruction & 0xFF;
    // u8 rd = (m_instruction >> 8) & 0x7;

    // bool load = m_instruction & (1 << 11);

    // u32 address = m_gpr[13] + (immediate << 2);

    // if (load) {
    //     m_gpr[rd] = read_word_rotate(address);
    // } else {
    //     write_word(address, m_gpr[rd]);
    // }
    
    // m_gpr[15] += 2;
}

void Interpreter::thumb_load_store_halfword() {
    // u8 rd = m_instruction & 0x7;
    // u8 rn = (m_instruction >> 3) & 0x7;
    // u32 immediate = (m_instruction >> 6) & 0x1F;
    // u32 address = m_gpr[rn] + (immediate << 1);

    // bool load = m_instruction & (1 << 11);

    // if (load) {
    //     m_gpr[rd] = read_half(address);
    // } else {
    //     write_half(address, m_gpr[rd]);
    // }

    // m_gpr[15] += 2;
}

void Interpreter::thumb_load_store_multiple() {
    // u8 rn = (m_instruction >> 8) & 0x7;
    // u32 address = m_gpr[rn];

    // bool load = m_instruction & (1 << 11);
    
    // // TODO: handle edgecases
    // if (load) {
    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             m_gpr[i] = read_word(address);
    //             address += 4;
    //         }
    //     }

    //     // if rn is in rlist:
    //     // if arm9 writeback if rn is the only register or not the last register in rlist
    //     // if arm7 then no writeback if rn in rlist
    //     if (m_arch == Arch::ARMv5) {
    //         if (((m_instruction & 0xFF) == static_cast<u32>(1 << rn)) || !(((m_instruction & 0xFF) >> rn) == 1)) {
    //             m_gpr[rn] = address;
    //         }
    //     } else if (!(m_instruction & static_cast<u32>(1 << rn))) {
    //         m_gpr[rn] = address;
    //     }
    // } else {
    //     for (int i = 0; i < 8; i++) {
    //         if (m_instruction & (1 << i)) {
    //             write_word(address, m_gpr[i]);
    //             address += 4;
    //         }
    //     }

    //     m_gpr[rn] = address;
    // }

    // m_gpr[15] += 2;
}

} // namespace core::arm