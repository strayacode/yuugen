#include "common/string.h"
#include "arm/instructions.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

std::string Disassembler::arm_branch_link_maybe_exchange(u32 instruction) {
    if ((instruction & 0xf0000000) != 0xf0000000) {
        return arm_branch_link(instruction);
    } else {
        return arm_branch_link_exchange(instruction);
    }
}

std::string Disassembler::arm_branch_exchange(u32 instruction) {
    const const auto opcode = ARMBranchExchange::decode(instruction);
    return common::format("bx%s %s", condition_names[opcode.condition], register_names[opcode.rm]);
}

std::string Disassembler::arm_count_leading_zeroes(u32 instruction) {
    return "handle arm_count_leading_zeroes";
}

std::string Disassembler::arm_branch_link(u32 instruction) {
    const auto opcode = ARMBranchLink::decode(instruction);
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
    const auto opcode = ARMBranchExchange::decode(instruction);
    return common::format("blx%s %s", condition_names[opcode.condition], register_names[opcode.rm]);
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
    const auto opcode = ARMHalfwordDataTransfer::decode(instruction);
    std::string rhs_string = common::format(" %s [%s%s, ", register_names[opcode.rd], register_names[opcode.rn], opcode.pre ? "" : "]");

    if (opcode.imm) {
        rhs_string += common::format("#0x%08x", opcode.rhs.imm);
    } else {
        rhs_string += common::format("%s", register_names[opcode.rhs.rm]);
    }

    if (opcode.pre) {
        rhs_string += "]";
    }

    if (opcode.writeback) {
        rhs_string += "!";
    }

    if (opcode.half && opcode.sign) {
        if (opcode.load) {
            return "ldrsh" + rhs_string;
        } else {
            return "strd" + rhs_string;
        }
    } else if (opcode.half) {
        if (opcode.load) {
            return "ldrh" + rhs_string;
        } else {
            return "strh" + rhs_string;
        }
    } else {
        if (opcode.load) {
            return "ldrsb" + rhs_string;
        } else {
            return "ldrd" + rhs_string;
        }
    }
}

std::string Disassembler::arm_status_load(u32 instruction) {
    const auto opcode = ARMStatusLoad::decode(instruction);
    auto psr_string = opcode.spsr ? "spsr" : "cpsr";
    return common::format("mrs %s, %s", register_names[opcode.rd], psr_string);
}

std::string Disassembler::arm_status_store_register(u32 instruction) {
    const auto opcode = ARMStatusStore::decode(instruction);
    std::string mask = "";
    if (opcode.mask & 0xff000000) {
        mask += "f";
    }

    if (opcode.mask & 0xff0000) {
        mask += "s";
    }

    if (opcode.mask & 0xff00) {
        mask += "x";
    }

    if (opcode.mask & 0xff) {
        mask += "c";
    }
    
    return common::format("msr %s, %s", mask.c_str(), register_names[opcode.rhs.rm]);
}

std::string Disassembler::arm_status_store_immediate(u32 instruction) {
    const auto opcode = ARMStatusStore::decode(instruction);
    std::string mask = "";
    if (opcode.mask & 0xff000000) {
        mask += "f";
    }

    if (opcode.mask & 0xff0000) {
        mask += "s";
    }

    if (opcode.mask & 0xff00) {
        mask += "x";
    }

    if (opcode.mask & 0xff) {
        mask += "c";
    }
    
    return common::format("msr %s, #0x%08x", mask.c_str(), opcode.rhs.rotated);
}

std::string Disassembler::arm_block_data_transfer(u32 instruction) {
    return "handle arm_block_data_transfer";
}

std::string Disassembler::arm_single_data_transfer(u32 instruction) {
    const auto opcode = ARMSingleDataTransfer::decode(instruction);
    std::string rhs_string = "";
    const auto opcode_string = opcode.load ? "ldr" : "str";
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
    const auto opcode = ARMDataProcessing::decode(instruction);
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
            rhs_string = common::format("%s %s %s", register_names[opcode.rhs.reg.rm], shift_type, register_names[opcode.rhs.reg.amount.rs]);
        }
    }

    auto operand_only = [&](const char *type) {
        return common::format("%s%s%s %s, %s", type, condition_names[opcode.condition], set_flags_string, register_names[opcode.rn], rhs_string.c_str());
    };

    auto destination_only = [&](const char *type) {
        return common::format("%s%s%s %s, %s", type, condition_names[opcode.condition], set_flags_string, register_names[opcode.rd], rhs_string.c_str());
    };
    
    auto operand_and_destination = [&](const char *type) {
        return common::format("%s%s%s %s, %s, %s", type, condition_names[opcode.condition], set_flags_string, register_names[opcode.rd], register_names[opcode.rn], rhs_string.c_str());
    };

    switch (opcode.opcode) {
    case ARMDataProcessing::Opcode::AND:
        return operand_and_destination("and");
    case ARMDataProcessing::Opcode::EOR:
        return operand_and_destination("eor");
    case ARMDataProcessing::Opcode::SUB:
        return operand_and_destination("sub");
    case ARMDataProcessing::Opcode::RSB:
        return operand_and_destination("rsb");
    case ARMDataProcessing::Opcode::ADD:
        return operand_and_destination("add");
    case ARMDataProcessing::Opcode::ADC:
        return operand_and_destination("adc");
    case ARMDataProcessing::Opcode::SBC:
        return operand_and_destination("sbc");
    case ARMDataProcessing::Opcode::RSC:
        return operand_and_destination("rsc");
    case ARMDataProcessing::Opcode::TST:
        return operand_only("tst");
    case ARMDataProcessing::Opcode::TEQ:
        return operand_only("teq");
    case ARMDataProcessing::Opcode::CMP:
        return operand_only("cmp");
    case ARMDataProcessing::Opcode::CMN:
        return operand_only("cmn");
    case ARMDataProcessing::Opcode::ORR:
        return operand_and_destination("orr");
    case ARMDataProcessing::Opcode::MOV:
        return destination_only("mov");
    case ARMDataProcessing::Opcode::BIC:
        return operand_and_destination("bic");
    case ARMDataProcessing::Opcode::MVN:
        return destination_only("mvn");
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

} // namespace arm