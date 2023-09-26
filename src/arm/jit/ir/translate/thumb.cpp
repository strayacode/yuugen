#include "common/logger.h"
#include "common/bits.h"
#include "arm/instructions.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/value.h"
#include "arm/jit/jit.h"

namespace arm {

Translator::BlockStatus Translator::thumb_alu_immediate() {
    auto opcode = ThumbALUImmediate::decode(instruction);
    auto op2 = ir.constant(opcode.imm);
    bool uses_op1 = opcode.opcode != ThumbALUImmediate::Opcode::MOV;
    IRValue op1;
    IRVariable result;

    if (uses_op1) {
        op1 = ir.load_gpr(opcode.rd);
    }
    
    if (opcode.rd == 15) {
        logger.todo("handle pc in thumb_alu_immediate");
    }

    switch (opcode.opcode) {
    case ThumbALUImmediate::Opcode::MOV:
        result = ir.copy(op2);
        ir.store_nz(result);
        break;
    case ThumbALUImmediate::Opcode::CMP: {
        auto result = ir.subtract(op1, op2);
        ir.store_nz(result);
        ir.store_sub_cv(op1, op2, result);
        break;
    }
    case ThumbALUImmediate::Opcode::ADD:
        result = ir.add(op1, op2);
        ir.store_nz(result);
        ir.store_add_cv(op1, op2, result);
        break;
    case ThumbALUImmediate::Opcode::SUB:
        result = ir.subtract(op1, op2);
        ir.store_nz(result);
        ir.store_sub_cv(op1, op2, result);
        break;
    }

    if (result.is_assigned()) {
        ir.store_gpr(opcode.rd, result);
    }

    ir.advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_offset() {
    auto opcode = ThumbBranchLinkOffset::decode(instruction);
    auto instruction_size = ir.basic_block.location.get_instruction_size();
    auto address = ir.add(ir.load_gpr(GPR::LR), ir.constant(opcode.offset + (2 * instruction_size)));
    ir.link();
    ir.branch(address);
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::thumb_branch_link_setup() {
    auto opcode = ThumbBranchLinkSetup::decode(instruction);
    auto address = ir.add(ir.load_gpr(GPR::PC), ir.constant(opcode.imm));
    ir.store_gpr(GPR::LR, address);
    ir.advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_exchange_offset() {
    logger.todo("Translator: handle thumb_branch_link_exchange_offset");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch() {
    logger.todo("Translator: handle thumb_branch");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_push_pop() {
    auto opcode = ThumbPushPop::decode(instruction);
    auto address = ir.load_gpr(GPR::SP);
    
    if (opcode.pop) {
        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                ir.store_gpr(static_cast<GPR>(i), ir.memory_read(address, AccessSize::Word, AccessType::Aligned));
                address = ir.add(address, ir.constant(4));
            }
        }

        if (opcode.pclr) {
            auto data = ir.memory_read(address, AccessSize::Word, AccessType::Aligned);
            ir.store_gpr(GPR::SP, ir.add(address, ir.constant(4)));
            
            if (jit.arch == Arch::ARMv5) {
                ir.branch_exchange(data, ExchangeType::Bit0);
            } else {
                ir.branch(data);
            }
            
            return BlockStatus::Break;
        } else {
            ir.advance_pc();
            ir.store_gpr(GPR::SP, address);
            return BlockStatus::Continue;
        }
    } else {
        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                address = ir.subtract(address, ir.constant(4));
            }
        }

        if (opcode.pclr) {
            address = ir.subtract(address, ir.constant(4));
        }

        ir.store_gpr(GPR::SP, address);

        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                ir.memory_write(address, ir.load_gpr(static_cast<GPR>(i)), AccessSize::Word);
                address = ir.add(address, ir.constant(4));
            }
        }

        if (opcode.pclr) {
            ir.memory_write(address, ir.load_gpr(GPR::LR), AccessSize::Word);
        }

        ir.advance_pc();
        return BlockStatus::Continue;
    }
}

Translator::BlockStatus Translator::thumb_data_processing_register() {
    logger.todo("Translator: handle thumb_data_processing_register");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_special_data_processing() {
    logger.todo("Translator: handle thumb_special_data_processing");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_link_exchange() {
    logger.todo("Translator: handle thumb_branch_link_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_exchange() {
    logger.todo("Translator: handle thumb_branch_exchange");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_register_offset() {
    logger.todo("Translator: handle thumb_load_store_register_offset");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_signed() {
    logger.todo("Translator: handle thumb_load_store_signed");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_pc() {
    logger.todo("Translator: handle thumb_load_pc");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_sp_relative() {
    logger.todo("Translator: handle thumb_load_store_sp_relative");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_halfword() {
    auto opcode = ThumbLoadStoreHalfword::decode(instruction);

    if (opcode.load && opcode.rd == 15) {
        logger.todo("handle r15 in thumb_load_store_halfword");
    }

    auto address = ir.add(ir.load_gpr(opcode.rn), ir.constant(opcode.imm << 1));
    if (opcode.load) {
        ir.store_gpr(opcode.rd, ir.memory_read(address, AccessSize::Half, AccessType::Aligned));
    } else {
        ir.memory_write(address, ir.load_gpr(opcode.rd), AccessSize::Half);
    }

    ir.advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_add_subtract() {
    logger.todo("Translator: handle thumb_add_subtract");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_shift_immediate() {
    auto opcode = ThumbShiftImmediate::decode(instruction);

    if (opcode.rd == 15) {
        logger.todo("handle r15 in thumb_shift_immediate");
    }

    auto src = ir.load_gpr(opcode.rs);
    auto pair = ir.barrel_shifter(src, opcode.shift_type, ir.constant(opcode.amount));
    auto result = pair.first;

    ir.store_flag(Flag::C, pair.second);
    ir.store_nz(result);
    ir.store_gpr(opcode.rd, result);
    ir.advance_pc();
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_software_interrupt() {
    logger.todo("Translator: handle thumb_software_interrupt");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_branch_conditional() {
    auto opcode = ThumbBranchConditional::decode(instruction);
    auto instruction_size = ir.basic_block.location.get_instruction_size();
    ir.branch(ir.constant(ir.basic_block.current_address + (2 * instruction_size) + opcode.offset));
    return BlockStatus::Break;
}

Translator::BlockStatus Translator::thumb_load_store_multiple() {
    logger.todo("Translator: handle thumb_load_store_multiple");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_load_store_immediate() {
    logger.todo("Translator: handle thumb_load_store_immediate");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_add_sp_pc() {
    logger.todo("Translator: handle thumb_add_sp_pc");
    return BlockStatus::Continue;
}

Translator::BlockStatus Translator::thumb_adjust_stack_pointer() {
    logger.todo("Translator: handle thumb_adjust_stack_pointer");
    return BlockStatus::Continue;
}

} // namespace arm