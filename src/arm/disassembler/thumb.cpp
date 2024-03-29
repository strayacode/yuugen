#include "common/string.h"
#include "arm/instructions.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

std::string Disassembler::thumb_alu_immediate(u32 instruction) {
    const auto opcode = ThumbALUImmediate::decode(instruction);
    switch (opcode.opcode) {
    case ThumbALUImmediate::Opcode::MOV:
        return common::format("mov %s, #0x%08x", register_names[opcode.rd], opcode.imm);
    case ThumbALUImmediate::Opcode::CMP:
        return common::format("cmp %s, #0x%08x", register_names[opcode.rd], opcode.imm);
    case ThumbALUImmediate::Opcode::ADD:
        return common::format("add %s, #0x%08x", register_names[opcode.rd], opcode.imm);
    case ThumbALUImmediate::Opcode::SUB:
        return common::format("sub %s, #0x%08x", register_names[opcode.rd], opcode.imm);
    }
}

std::string Disassembler::thumb_branch_link_offset(u32 instruction) {
    const auto opcode = ThumbBranchLinkOffset::decode(instruction); 
    return common::format("bl #0x%08x", opcode.offset + 4);
}

std::string Disassembler::thumb_branch_link_setup(u32 instruction) {
    return "handle thumb_branch_link_setup";
}

std::string Disassembler::thumb_branch_link_exchange_offset(u32 instruction) {
    return "handle thumb_branch_link_exchange_offset";
}

std::string Disassembler::thumb_branch(u32 instruction) {
    return "handle thumb_branch";
}

std::string Disassembler::thumb_push_pop(u32 instruction) {
    const auto opcode = ThumbPushPop::decode(instruction);
    std::string rlist = "";
    bool first = true;
    if (opcode.pop) {
        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                if (first) {
                    first = false;
                    rlist += "{";
                } else {
                    rlist += ", ";
                }

                rlist += register_names[i];
            }
        }

        if (opcode.pclr) {
            rlist += ", pc";
        }

        rlist += "}";
        return common::format("pop %s", rlist.c_str());
    } else {
        for (int i = 0; i < 8; i++) {
            if (opcode.rlist & (1 << i)) {
                if (first) {
                    first = false;
                    rlist += "{";
                } else {
                    rlist += ", ";
                }

                rlist += register_names[i];
            }
        }

        if (opcode.pclr) {
            rlist += ", lr";
        }

        rlist += "}";
        return common::format("push %s", rlist.c_str());
    }
}

std::string Disassembler::thumb_data_processing_register(u32 instruction) {
    auto opcode = ThumbDataProcessingRegister::decode(instruction);
    switch (opcode.opcode) {
    case ThumbDataProcessingRegister::Opcode::SBC:
        return common::format("sbcs %s, %s, %s", register_names[opcode.rd], register_names[opcode.rd], register_names[opcode.rs]);
    case ThumbDataProcessingRegister::Opcode::NEG:
        return common::format("negs %s, %s", register_names[opcode.rd], register_names[opcode.rs]);
    default:
        return common::format("handle thumb_data_processing_register %d", static_cast<int>(opcode.opcode));
    }
}

std::string Disassembler::thumb_special_data_processing(u32 instruction) {
    return "handle thumb_special_data_processing";
}

std::string Disassembler::thumb_branch_link_exchange(u32 instruction) {
    return "handle thumb_branch_link_exchange";
}

std::string Disassembler::thumb_branch_exchange(u32 instruction) {
    return "handle thumb_branch_exchange";
}

std::string Disassembler::thumb_load_store_register_offset(u32 instruction) {
    return "handle thumb_load_store_register_offset";
}

std::string Disassembler::thumb_load_store_signed(u32 instruction) {
    return "handle thumb_load_store_signed";
}

std::string Disassembler::thumb_load_pc(u32 instruction) {
    return "handle thumb_load_pc";
}

std::string Disassembler::thumb_load_store_sp_relative(u32 instruction) {
    return "handle thumb_load_store_sp_relative";
}

std::string Disassembler::thumb_load_store_halfword(u32 instruction) {
    const auto opcode = ThumbLoadStoreHalfword::decode(instruction);
    if (opcode.load) {
        return common::format("ldrh %s, [%s]", register_names[opcode.rd], register_names[opcode.rn]);
    } else {
        return common::format("strh %s, [%s]", register_names[opcode.rd], register_names[opcode.rn]);
    }
}

std::string Disassembler::thumb_add_subtract(u32 instruction) {
    return "handle thumb_add_subtract";
}

std::string Disassembler::thumb_shift_immediate(u32 instruction) {
    auto opcode = ThumbShiftImmediate::decode(instruction);
    return common::format("%ss %s, %s, #0x%02x", shift_types[static_cast<int>(opcode.shift_type)], register_names[opcode.rd], register_names[opcode.rs], opcode.amount);
}

std::string Disassembler::thumb_software_interrupt(u32 instruction) {
    return "handle thumb_software_interrupt";
}

std::string Disassembler::thumb_branch_conditional(u32 instruction) {
    return "handle thumb_branch_conditional";
}

std::string Disassembler::thumb_load_store_multiple(u32 instruction) {
    return "handle thumb_load_store_multiple";
}

std::string Disassembler::thumb_load_store_immediate(u32 instruction) {
    const auto opcode = ThumbLoadStoreImmediate::decode(instruction);
    switch (opcode.opcode) {
    case ThumbLoadStoreImmediate::Opcode::STR:
        return common::format("str %s, [%s]", register_names[opcode.rn], register_names[opcode.rd]);
    case ThumbLoadStoreImmediate::Opcode::LDR:
        return common::format("ldr %s, [%s]", register_names[opcode.rd], register_names[opcode.rn]);
    case ThumbLoadStoreImmediate::Opcode::STRB:
        return common::format("strb %s, [%s]", register_names[opcode.rn], register_names[opcode.rd]);
    case ThumbLoadStoreImmediate::Opcode::LDRB:
        return common::format("ldrb %s, [%s]", register_names[opcode.rd], register_names[opcode.rn]);
    }
}

std::string Disassembler::thumb_add_sp_pc(u32 instruction) {
    return "handle thumb_add_sp_pc";
}

std::string Disassembler::thumb_adjust_stack_pointer(u32 instruction) {
    return "handle thumb_adjust_stack_pointer";
}


} // namespace arm