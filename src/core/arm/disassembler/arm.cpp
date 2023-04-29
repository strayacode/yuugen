#include "common/string.h"
#include "core/arm/instructions.h"
#include "core/arm/disassembler/disassembler.h"

namespace core::arm {

std::string Disassembler::arm_branch_link_maybe_exchange(u32 instruction) {
    if ((instruction & 0xf0000000) != 0xf0000000) {
        return arm_branch_link(instruction);
    } else {
        return arm_branch_link_exchange(instruction);
    }
}

std::string Disassembler::arm_branch_exchange(u32 instruction) {
    return "handle arm_branch_exchange";
}

std::string Disassembler::arm_count_leading_zeroes(u32 instruction) {
    return "handle arm_count_leading_zeroes";
}

std::string Disassembler::arm_branch_link(u32 instruction) {
    auto opcode = ARMBranchLink::decode(instruction);
    if (opcode.link) {
        return common::format("bl%s #0x%08x", condition_names[opcode.condition], opcode.offset + 8);
    } else {
        return common::format("b%s #0x%08x", condition_names[opcode.condition], opcode.offset + 8);
    }
}

std::string Disassembler::arm_branch_link_exchange(u32 instruction) {
    return "handle arm_branch_link_exchange";
}

std::string Disassembler::arm_branch_link_exchange_register(u32 instruction) {
    return "handle arm_branch_link_exchange_register";
}

std::string Disassembler::arm_single_data_swap(u32 instruction) {
    return "handle arm_single_data_swap";
}

std::string Disassembler::arm_multiply(u32 instruction) {
    return "handle arm_multiply";
}

std::string Disassembler::arm_saturating_add_subtract(u32 instruction) {
    return "handle arm_saturating_add_subtract";
}

std::string Disassembler::arm_multiply_long(u32 instruction) {
    return "handle arm_multiply_long";
}

std::string Disassembler::arm_halfword_data_transfer(u32 instruction) {
    return "handle arm_halfword_data_transfer";
}

std::string Disassembler::arm_status_load(u32 instruction) {
    return "handle arm_status_load";
}

std::string Disassembler::arm_status_store(u32 instruction) {
    return "handle arm_status_store";
}

std::string Disassembler::arm_block_data_transfer(u32 instruction) {
    return "handle arm_block_data_transfer";
}

std::string Disassembler::arm_single_data_transfer(u32 instruction) {
    auto opcode = ARMSingleDataTransfer::decode(instruction);
    std::string rhs_string = "";
    auto opcode_string = opcode.load ? "ldr" : "str";
    auto byte_string = opcode.byte ? "b" : "";

    if (opcode.imm) {
        if (opcode.rhs.imm) {
            rhs_string = common::format(", #0x%08x", opcode.rhs.imm);
        }
    } else {
        const char* shift_types[4] = {"lsl", "lsr", "asr", "ror"};
        const char* shift_type = shift_types[static_cast<int>(opcode.rhs.reg.shift_type)];
        rhs_string = common::format(", %s %s %s", register_names[opcode.rhs.reg.rm], shift_type, register_names[opcode.rhs.reg.amount]);
    }

    if (opcode.pre) {
        rhs_string = common::format("[%s%s]", register_names[opcode.rn], rhs_string.c_str());
    } else {
        rhs_string = common::format("[%s]%s", register_names[opcode.rn], rhs_string.c_str());
    }

    return common::format("%s%s %s, %s", opcode_string, byte_string, register_names[opcode.rd], rhs_string.c_str());
}

std::string Disassembler::arm_data_processing(u32 instruction) {
    auto opcode = ARMDataProcessing::decode(instruction);
    auto set_flags_string = opcode.set_flags ? "s" : "";
    std::string rhs_string = "";

    if (opcode.imm) {
        rhs_string = common::format("#0x%08x", opcode.rhs.imm.rotated);
    } else {
        const char* shift_types[4] = {"lsl", "lsr", "asr", "ror"};
        const char* shift_type = shift_types[static_cast<int>(opcode.rhs.reg.shift_type)];

        if (opcode.rhs.reg.imm) {
            if (opcode.rhs.reg.amount.imm) {
                rhs_string = common::format("%s %s #0x%02x", register_names[opcode.rhs.reg.rm], shift_type, opcode.rhs.reg.amount.imm);
            } else {
                rhs_string = common::format("%s", register_names[opcode.rhs.reg.rm]);
            }
        } else {
            rhs_string = common::format("%s %s %s", register_names[opcode.rd], shift_type, register_names[opcode.rhs.reg.amount.rs]);
        }
    }

    switch (opcode.opcode) {
    case ARMDataProcessing::Opcode::AND:
        return common::format("and%s%s %s, %s, %s", set_flags_string, condition_names[opcode.condition], register_names[opcode.rd], register_names[opcode.rn], rhs_string.c_str());
    case ARMDataProcessing::Opcode::EOR:
        return common::format("eor");
    case ARMDataProcessing::Opcode::SUB:
        return common::format("sub");
    case ARMDataProcessing::Opcode::RSB:
        return common::format("rsb");
    case ARMDataProcessing::Opcode::ADD:
        return common::format("add");
    case ARMDataProcessing::Opcode::ADC:
        return common::format("adc");
    case ARMDataProcessing::Opcode::SBC:
        return common::format("sbc");
    case ARMDataProcessing::Opcode::RSC:
        return common::format("rsc");
    case ARMDataProcessing::Opcode::TST:
        return common::format("tst");
    case ARMDataProcessing::Opcode::TEQ:
        return common::format("teq");
    case ARMDataProcessing::Opcode::CMP:
        return common::format("cmp");
    case ARMDataProcessing::Opcode::CMN:
        return common::format("cmn");
    case ARMDataProcessing::Opcode::ORR:
        return common::format("orr");
    case ARMDataProcessing::Opcode::MOV:
        return common::format("mov%s%s %s, %s", set_flags_string, condition_names[opcode.condition], register_names[opcode.rd], rhs_string.c_str());
    case ARMDataProcessing::Opcode::BIC:
        return common::format("bic");
    case ARMDataProcessing::Opcode::MVN:
        return common::format("mvn");
    }

    return "...";
}

std::string Disassembler::arm_coprocessor_register_transfer(u32 instruction) {
    return "handle arm_coprocessor_register_transfer";
}

std::string Disassembler::arm_software_interrupt(u32 instruction) {
    return "handle arm_software_interrupt";
}

std::string Disassembler::arm_signed_multiply_accumulate_long(u32 instruction) {
    return "handle arm_signed_multiply_accumulate_long";
}

std::string Disassembler::arm_signed_multiply_word(u32 instruction) {
    return "handle arm_signed_multiply_word";
}

std::string Disassembler::arm_signed_multiply(u32 instruction) {
    return "handle arm_signed_multiply";
}

std::string Disassembler::arm_breakpoint(u32 instruction) {
    return "handle arm_breakpoint";
}

} // namespace core::arm