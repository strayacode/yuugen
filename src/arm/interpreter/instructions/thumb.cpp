#include "common/logger.h"
#include "common/bits.h"
#include "arm/interpreter/interpreter.h"

namespace arm {

void Interpreter::thumb_add_subtract() {
    auto opcode = ThumbAddSubtract::decode(instruction);
    u32 lhs = state.gpr[opcode.rs];
    u32 rhs = opcode.imm ? opcode.rn : state.gpr[opcode.rn];

    if (opcode.sub) {
        state.gpr[opcode.rd] = alu_sub(lhs, rhs, true);
    } else {
        state.gpr[opcode.rd] = alu_add(lhs, rhs, true);
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_shift_immediate() {
    auto opcode = ThumbShiftImmediate::decode(instruction);
    bool carry = state.cpsr.c;

    state.gpr[opcode.rd] = barrel_shifter(state.gpr[opcode.rs], opcode.shift_type, opcode.amount, carry, true);
    state.cpsr.c = carry;
    set_nz(state.gpr[opcode.rd]);
    state.gpr[15] += 2;
}

void Interpreter::thumb_alu_immediate() {
    auto opcode = ThumbALUImmediate::decode(instruction);
    switch (opcode.opcode) {
    case ThumbALUImmediate::MOV:
        state.gpr[opcode.rd] = opcode.imm;
        state.cpsr.n = false;
        state.cpsr.z = opcode.imm == 0;
        break;
    case 0x1:
        alu_cmp(state.gpr[opcode.rd], opcode.imm);
        break;
    case 0x2:
        state.gpr[opcode.rd] = alu_add(state.gpr[opcode.rd], opcode.imm, true);
        break;
    case 0x3:
        state.gpr[opcode.rd] = alu_sub(state.gpr[opcode.rd], opcode.imm, true);
        break;
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_data_processing_register() {
    auto opcode = ThumbDataProcessingRegister::decode(instruction);
    bool carry = state.cpsr.c;

    switch (opcode.opcode) {
    case ThumbDataProcessingRegister::Opcode::AND:
        state.gpr[opcode.rd] = alu_and(state.gpr[opcode.rd], state.gpr[opcode.rs], true);
        break;
    case ThumbDataProcessingRegister::Opcode::EOR:
        state.gpr[opcode.rd] = alu_eor(state.gpr[opcode.rd], state.gpr[opcode.rs], true);
        break;
    case ThumbDataProcessingRegister::Opcode::LSL:
        state.gpr[opcode.rd] = alu_lsl(state.gpr[opcode.rd], state.gpr[opcode.rs] & 0xff, carry);
        state.cpsr.c = carry;
        set_nz(state.gpr[opcode.rd]);
        break;
    case ThumbDataProcessingRegister::Opcode::LSR:
        state.gpr[opcode.rd] = alu_lsr(state.gpr[opcode.rd], state.gpr[opcode.rs] & 0xff, carry, false);
        state.cpsr.c = carry;
        set_nz(state.gpr[opcode.rd]);
        break;
    case ThumbDataProcessingRegister::Opcode::ASR:
        state.gpr[opcode.rd] = alu_asr(state.gpr[opcode.rd], state.gpr[opcode.rs] & 0xff, carry, false);
        state.cpsr.c = carry;
        set_nz(state.gpr[opcode.rd]);
        break;
    case ThumbDataProcessingRegister::Opcode::ADC:
        state.gpr[opcode.rd] = alu_adc(state.gpr[opcode.rd], state.gpr[opcode.rs], true);
        break;
    case ThumbDataProcessingRegister::Opcode::SBC:
        state.gpr[opcode.rd] = alu_sbc(state.gpr[opcode.rd], state.gpr[opcode.rs], true);
        break;
    case ThumbDataProcessingRegister::Opcode::ROR:
        state.gpr[opcode.rd] = alu_ror(state.gpr[opcode.rd], state.gpr[opcode.rs] & 0xff, carry, false);
        state.cpsr.c = carry;
        set_nz(state.gpr[opcode.rd]);
        break;
    case ThumbDataProcessingRegister::Opcode::TST:
        alu_tst(state.gpr[opcode.rd], state.gpr[opcode.rs]);
        break;
    case ThumbDataProcessingRegister::Opcode::NEG:
        state.gpr[opcode.rd] = alu_sub(0, state.gpr[opcode.rs], true);
        break;
    case ThumbDataProcessingRegister::Opcode::CMP:
        alu_cmp(state.gpr[opcode.rd], state.gpr[opcode.rs]);
        break;
    case ThumbDataProcessingRegister::Opcode::CMN:
        alu_cmn(state.gpr[opcode.rd], state.gpr[opcode.rs]);
        break;
    case ThumbDataProcessingRegister::Opcode::ORR:
        state.gpr[opcode.rd] = alu_orr(state.gpr[opcode.rd], state.gpr[opcode.rs], true);
        break;
    case ThumbDataProcessingRegister::Opcode::MUL:
        state.gpr[opcode.rd] *= state.gpr[opcode.rs];
        set_nz(state.gpr[opcode.rd]);
        break;
    case ThumbDataProcessingRegister::Opcode::BIC:
        state.gpr[opcode.rd] = alu_bic(state.gpr[opcode.rd], state.gpr[opcode.rs], true);
        break;
    case ThumbDataProcessingRegister::Opcode::MVN:
        state.gpr[opcode.rd] = alu_mvn(state.gpr[opcode.rs], true);
        break;
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_special_data_processing() {
    auto opcode = ThumbSpecialDataProcessing::decode(instruction);
    switch (opcode.opcode) {
    case ThumbSpecialDataProcessing::Opcode::ADD:
        state.gpr[opcode.rd] += state.gpr[opcode.rs];
        if (opcode.rd == 15) {
            thumb_flush_pipeline();
        } else {
            state.gpr[15] += 2;    
        }

        break;
    case ThumbSpecialDataProcessing::Opcode::CMP:
        alu_cmp(state.gpr[opcode.rd], state.gpr[opcode.rs]);
        state.gpr[15] += 2;
        break;
    case ThumbSpecialDataProcessing::Opcode::MOV:
        state.gpr[opcode.rd] = state.gpr[opcode.rs];
        if (opcode.rd == 15) {
            thumb_flush_pipeline();
        } else {
            state.gpr[15] += 2;
        }

        break;
    }
}

void Interpreter::thumb_adjust_stack_pointer() {
    auto opcode = ThumbAdjustStackPointer::decode(instruction);
    if (opcode.sub) {
        state.gpr[13] -= opcode.imm;
    } else {
        state.gpr[13] += opcode.imm;
    }
    
    state.gpr[15] += 2;
}

void Interpreter::thumb_add_sp_pc() {
    auto opcode = ThumbAddSPPC::decode(instruction);
    if (opcode.sp) {
        state.gpr[opcode.rd] = state.gpr[13] + opcode.imm;
    } else {
        state.gpr[opcode.rd] = (state.gpr[15] & ~0x2) + opcode.imm;
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_branch_exchange() {
    auto opcode = ThumbBranchExchange::decode(instruction);
    if (state.gpr[opcode.rm] & 0x1) {
        state.gpr[15] = state.gpr[opcode.rm] & ~0x1;
        thumb_flush_pipeline();
    } else {
        state.cpsr.t = false;
        state.gpr[15] = state.gpr[opcode.rm] & ~0x3;
        arm_flush_pipeline();
    }
}

void Interpreter::thumb_branch_link_exchange() {
    if (arch == Arch::ARMv4) {
        return;
    }

    u8 rm = (instruction >> 3) & 0xF;
    u32 next_instruction_address = state.gpr[15] - 2;
    state.gpr[14] = next_instruction_address | 1;
    
    if (state.gpr[rm] & 0x1) {
        state.gpr[15] = state.gpr[rm] & ~1;
        thumb_flush_pipeline();
    } else {
        state.cpsr.t = false;
        state.gpr[15] = state.gpr[rm] & ~3;
        arm_flush_pipeline();
    }
}

void Interpreter::thumb_branch_link_setup() {
    auto opcode = ThumbBranchLinkSetup::decode(instruction);
    state.gpr[14] = state.gpr[15] + opcode.imm;
    state.gpr[15] += 2;
}

void Interpreter::thumb_branch_link_offset() {
    auto opcode = ThumbBranchLinkOffset::decode(instruction); 
    u32 next_instruction_addr = state.gpr[15] - 2;
    state.gpr[15] = (state.gpr[14] + opcode.offset) & ~0x1;
    state.gpr[14] = next_instruction_addr | 0x1;
    thumb_flush_pipeline();
}

void Interpreter::thumb_branch_link_exchange_offset() {
    // arm9 specific instruction
    if (arch == Arch::ARMv4) {
        return;
    }

    u32 offset = (instruction & 0x7FF) << 1;
    u32 next_instruction_address = state.gpr[15] - 2;
    state.gpr[15] = (state.gpr[14] + offset) & ~0x3;
    state.gpr[14] = next_instruction_address | 1;

    // set t flag to 0
    state.cpsr.t = false;

    // flush the pipeline
    arm_flush_pipeline();
}

void Interpreter::thumb_branch() {
    auto opcode = ThumbBranch::decode(instruction);
    state.gpr[15] += opcode.offset;
    thumb_flush_pipeline();
}

void Interpreter::thumb_branch_conditional() {
    auto opcode = ThumbBranchConditional::decode(instruction);
    if (evaluate_condition(opcode.condition)) {
        state.gpr[15] += opcode.offset;
        thumb_flush_pipeline();
    } else {
        state.gpr[15] += 2;
    }
}

void Interpreter::thumb_software_interrupt() {
    state.spsr_banked[Bank::SVC].data = state.cpsr.data;
    set_mode(Mode::SVC);

    state.cpsr.t = false;
    state.cpsr.i = true;
    state.gpr[14] = state.gpr[15] - 2;
    state.gpr[15] = coprocessor.get_exception_base() + 0x08;
    arm_flush_pipeline();
}

void Interpreter::thumb_load_pc() {
    auto opcode = ThumbLoadPC::decode(instruction);
    u32 addr = (state.gpr[15] & ~0x2) + opcode.imm;
    state.gpr[opcode.rd] = read_word(addr);
    state.gpr[15] += 2;
}

void Interpreter::thumb_load_store_register_offset() {
    auto opcode = ThumbLoadStoreRegisterOffset::decode(instruction);
    u32 addr = state.gpr[opcode.rn] + state.gpr[opcode.rm];
    switch (opcode.opcode) {
    case ThumbLoadStoreRegisterOffset::Opcode::STR:
        write_word(addr, state.gpr[opcode.rd]);
        break;
    case ThumbLoadStoreRegisterOffset::Opcode::STRB:
        write_byte(addr, state.gpr[opcode.rd]);
        break;
    case ThumbLoadStoreRegisterOffset::Opcode::LDR:
        state.gpr[opcode.rd] = read_word_rotate(addr);
        break;
    case ThumbLoadStoreRegisterOffset::Opcode::LDRB:
        state.gpr[opcode.rd] = read_byte(addr);
        break;
    }
    
    state.gpr[15] += 2;
}

void Interpreter::thumb_load_store_signed() {
    auto opcode = ThumbLoadStoreSigned::decode(instruction);
    u32 addr = state.gpr[opcode.rn] + state.gpr[opcode.rm];
    switch (opcode.opcode) {
    case ThumbLoadStoreSigned::Opcode::STRH:
        write_half(addr, state.gpr[opcode.rd]);
        break;
    case ThumbLoadStoreSigned::Opcode::LDRSB:
        state.gpr[opcode.rd] = common::sign_extend<s32, 8>(read_byte(addr));
        break;
    case ThumbLoadStoreSigned::Opcode::LDRH:
        state.gpr[opcode.rd] = read_half(addr);
        break;
    case ThumbLoadStoreSigned::Opcode::LDRSH:
        state.gpr[opcode.rd] = common::sign_extend<s32, 16>(read_half(addr));
        break;
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_load_store_immediate() {
    auto opcode = ThumbLoadStoreImmediate::decode(instruction);
    switch (opcode.opcode) {
    case ThumbLoadStoreImmediate::Opcode::STR:
        write_word(state.gpr[opcode.rn] + (opcode.imm << 2), state.gpr[opcode.rd]);
        break;
    case ThumbLoadStoreImmediate::Opcode::LDR:
        state.gpr[opcode.rd] = read_word_rotate(state.gpr[opcode.rn] + (opcode.imm << 2));
        break;
    case ThumbLoadStoreImmediate::Opcode::STRB:
        write_byte(state.gpr[opcode.rn] + opcode.imm, state.gpr[opcode.rd]);
        break;
    case ThumbLoadStoreImmediate::Opcode::LDRB:
        state.gpr[opcode.rd] = read_byte(state.gpr[opcode.rn] + opcode.imm);
        break;
    }
    
    state.gpr[15] += 2;
}

void Interpreter::thumb_push_pop() {
    auto opcode = ThumbPushPop::decode(instruction);
    u32 addr = state.gpr[13];

    if (opcode.pop) {
        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                state.gpr[i] = read_word(addr);
                addr += 4;
            }
        }

        if (opcode.pclr) {
            state.gpr[15] = read_word(addr);
            state.gpr[13] = addr + 4;
            
            if ((arch == Arch::ARMv4) || (state.gpr[15] & 0x1)) {
                state.gpr[15] &= ~0x1;
                thumb_flush_pipeline();
            } else {
                state.cpsr.t = false;
                state.gpr[15] &= ~0x3;
                arm_flush_pipeline();
            }
        } else {
            state.gpr[15] += 2;
            state.gpr[13] = addr;
        }
    } else {
        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                addr -= 4;
            }
        }

        if (opcode.pclr) {
            addr -= 4;
        }

        state.gpr[13] = addr;

        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                write_word(addr, state.gpr[i]);
                addr += 4;
            }
        }

        if (opcode.pclr) {
            write_word(addr, state.gpr[14]);
        }

        state.gpr[15] += 2;
    }
}

void Interpreter::thumb_load_store_sp_relative() {
    u32 immediate = instruction & 0xFF;
    u8 rd = (instruction >> 8) & 0x7;

    bool load = instruction & (1 << 11);

    u32 address = state.gpr[13] + (immediate << 2);

    if (load) {
        state.gpr[rd] = read_word_rotate(address);
    } else {
        write_word(address, state.gpr[rd]);
    }
    
    state.gpr[15] += 2;
}

void Interpreter::thumb_load_store_halfword() {
    auto opcode = ThumbLoadStoreHalfword::decode(instruction);
    u32 addr = state.gpr[opcode.rn] + (opcode.imm << 1);
    if (opcode.load) {
        state.gpr[opcode.rd] = read_half(addr);
    } else {
        write_half(addr, state.gpr[opcode.rd]);
    }

    state.gpr[15] += 2;
}

void Interpreter::thumb_load_store_multiple() {
    u8 rn = (instruction >> 8) & 0x7;
    u32 address = state.gpr[rn];

    bool load = instruction & (1 << 11);
    
    // TODO: handle edgecases
    if (load) {
        for (int i = 0; i < 8; i++) {
            if (instruction & (1 << i)) {
                state.gpr[i] = read_word(address);
                address += 4;
            }
        }

        // if rn is in rlist:
        // if arm9 writeback if rn is the only register or not the last register in rlist
        // if arm7 then no writeback if rn in rlist
        if (arch == Arch::ARMv5) {
            if (((instruction & 0xFF) == static_cast<u32>(1 << rn)) || !(((instruction & 0xFF) >> rn) == 1)) {
                state.gpr[rn] = address;
            }
        } else if (!(instruction & static_cast<u32>(1 << rn))) {
            state.gpr[rn] = address;
        }
    } else {
        for (int i = 0; i < 8; i++) {
            if (instruction & (1 << i)) {
                write_word(address, state.gpr[i]);
                address += 4;
            }
        }

        state.gpr[rn] = address;
    }

    state.gpr[15] += 2;
}

} // namespace arm