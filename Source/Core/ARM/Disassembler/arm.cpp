#include "Common/Types.h"
#include "Common/format.h"
#include "Common/Log.h"
#include "Common/Bits.h"
#include "Core/ARM/Disassembler/Disassembler.h"

std::string Disassembler::arm_branch_link_maybe_exchange(u32 instruction) {
    if ((instruction & 0xF0000000) != 0xF0000000) {
        const bool link = (instruction >> 24) & 0x1;
        s32 offset = (((instruction & (1 << 23)) ? 0xFC000000 : 0) | ((instruction & 0xFFFFFF) << 2)) + 8;

        if (link) {
            return format("bl #0x%08x", offset);
        } else {
            return format("b #0x%08x", offset);
        }

    } else {
        s32 offset = (((instruction & (1 << 23)) ? 0xFC000000: 0) | ((instruction & 0xFFFFFF) << 2)) + ((instruction & (1 << 24)) >> 23);
        return format("blx #0x%08x", offset);
    }
}

std::string Disassembler::arm_branch_exchange(u32 instruction) {
    u8 rm = instruction & 0xF;
    return format("bx r%d", rm);
}

std::string Disassembler::arm_count_leading_zeroes(u32 instruction) {
    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;

    return format("clz r%d, r%d", rd, rm);
}

std::string Disassembler::arm_branch_link_exchange_register(u32 instruction) {
    u8 rm = instruction & 0xF;
    return format("blx r%d", rm);
}

std::string Disassembler::arm_single_data_swap(u32 instruction) {
    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u8 byte = (instruction >> 22) & 0x1;
    
    return format("swp%s r%d, r%d, [r%d]", byte ? "b" : "", rd, rm, rn);
}

std::string Disassembler::arm_multiply(u32 instruction) {
    const bool accumulate = (instruction >> 21) & 0x1;
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rn = (instruction >> 12) & 0xF;
    u8 rd = (instruction >> 16) & 0xF;

    std::string set_flags = (instruction >> 20) & 0x1 ? "s" : "";
    
    if (accumulate) {
        return format("mla%s r%d, r%d, r%d, r%d", set_flags.c_str(), rd, rm, rs, rn);
    } else {
        return format("mul%s r%d, r%d, r%d", set_flags.c_str(), rd, rm, rs);
    }
}

std::string Disassembler::arm_saturating_add_subtract(u32 instruction) {
    u8 rm = instruction & 0xF;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    u8 opcode = (instruction >> 20) & 0xF;

    switch (opcode) {
    case 0x0:
        return format("qadd r%d, r%d, r%d", rd, rm, rn);
    case 0x2:
        return format("qsub r%d, r%d, r%d", rd, rm, rn);
    case 0x4:
        return format("qdadd r%d, r%d, r%d", rd, rm, rn);
    case 0x6:
        return format("qdsub r%d, r%d, r%d", rd, rm, rn);
    default:
        log_fatal("handle opcode %d", opcode);
    }
}

std::string Disassembler::arm_multiply_long(u32 instruction) {
    u8 rm = instruction & 0xF;
    u8 rs = (instruction >> 8) & 0xF;
    u8 rdlo = (instruction >> 12) & 0xF;
    u8 rdhi = (instruction >> 16) & 0xF;
    u8 opcode = (instruction >> 21) & 0xF;

    std::string set_flags = (instruction >> 20) & 0x1 ? "s" : "";

    switch (opcode) {
    case 0x2:
        return format("umaal r%d, r%d, r%d, r%d", rdlo, rdhi, rm, rs);
    case 0x4:
        return format("umull%s r%d, r%d, r%d, r%d", set_flags.c_str(), rdlo, rdhi, rm, rs);
    case 0x5:
        return format("umlal%s r%d, r%d, r%d, r%d", set_flags.c_str(), rdlo, rdhi, rm, rs);
    case 0x6:
        return format("smull%s r%d, r%d, r%d, r%d", set_flags.c_str(), rdlo, rdhi, rm, rs);
    case 0x7:
        return format("smlal%s r%d, r%d, r%d, r%d", set_flags.c_str(), rdlo, rdhi, rm, rs);
    default:
        log_fatal("handle opcode %d", opcode);
    }
}

std::string Disassembler::arm_halfword_data_transfer(u32 instruction) {
    u8 rm = instruction & 0xF;
    u8 opcode = (instruction >> 5) & 0x3;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;
    const bool load = (instruction >> 20) & 0x1;
    const bool writeback = (instruction >> 21) & 0x1;
    const bool immediate = (instruction >> 22) & 0x1;
    const bool up = (instruction >> 23) & 0x1;
    const bool pre = (instruction >> 24) & 0x1;

    std::string address = format(" [r%d%s, ", rn, pre ? "" : "]");

    if (immediate) {
        address += format("#0x%08x", ((instruction >> 4) & 0xF0) | (instruction & 0xF));
    } else {
        address += format("r%d", rm);
    }

    if (pre) {
        address += "]";
    }

    if (writeback) {
        address += "!";
    }

    switch (opcode) {
    case 0x1:
        if (load) {
            return "ldrh" + address;
        } else {
            return "strh" + address;
        }
        break;
    case 0x2:
        if (load) {
            return "ldrsb" + address;
        } else {
            return "ldrd" + address;
        }
        break;
    case 0x3:
        if (load) {
            return "ldrsh" + address;
        } else {
            return "strd" + address;
        }
        break;
    default:
        log_fatal("handle opcode %d", opcode);
    }
}

std::string Disassembler::arm_psr_transfer(u32 instruction) {
    const bool opcode = (instruction >> 21) & 0x1;
    const bool spsr = (instruction >> 22) & 0x1;
    u8 rm = instruction & 0xF;

    if (opcode) {
        // msr
        u8 immediate = (instruction >> 25) & 0x1;

        std::string mask = "";

        if (instruction & (1 << 19)) {
            mask += "f";
        }

        if (instruction & (1 << 18)) {
            mask += "s";
        }

        if (instruction & (1 << 17)) {
            mask += "x";
        }

        if (instruction & (1 << 16)) {
            mask += "c";
        }
        
        if (immediate) {
            u32 immediate = instruction & 0xFF;
            u8 rotate_amount = ((instruction >> 8) & 0xF) << 1;

            return format("msr %s, #0x%08x", mask.c_str(), Common::rotate_right(immediate, rotate_amount));
        } else {
            return format("msr %s, r%d", mask.c_str(), rm);
        }
    } else {
        // mrs
        u8 rd = (instruction >> 12) & 0xF;

        std::string psr = spsr ? "spsr" : "cpsr";

        return format("mrs r%d, %s", rd, psr.c_str());
    }
}

std::string Disassembler::arm_block_data_transfer(u32 instruction) {
    const bool load = (instruction >> 20) & 0x1;
    const bool writeback = (instruction >> 21) & 0x1;
    const bool load_psr = (instruction >> 22) & 0x1;
    const bool up = (instruction >> 23) & 0x1;
    const bool pre = (instruction >> 24) & 0x1;
    u8 rn = (instruction >> 16) & 0xF;

    int highest_bit = 0;

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            highest_bit = i;
        }
    }

    std::string regs_list = "";

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs_list += format("r%d", i);

            if (i != highest_bit) {
                regs_list += ", ";
            }
        }
    }

    if (load) {
        return format("ldm r%d, {%s}", rn, regs_list.c_str());
    } else {
        return format("stm r%d, {%s}", rn, regs_list.c_str());
    }
}

std::string Disassembler::arm_single_data_transfer(u32 instruction) {
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    const bool writeback = (instruction >> 21) & 0x1;
    const bool up = (instruction >> 23) & 0x1;
    const bool pre = (instruction >> 24) & 0x1;
    const bool shifted_register = (instruction >> 25) & 0x1;
    
    std::string name = (instruction >> 20) & 0x1 ? "ldr" : "str";
    std::string byte = (instruction >> 22) & 0x1 ? "b" : "";
    std::string address = "";

    if (!shifted_register && (instruction & 0xFFF) == 0) {
        address = format("[r%d]", rn);
    } else {
        if (pre) {
            address = format("[r%d, %s]", rn, arm_single_data_transfer_get_address(instruction).c_str());
        } else {
            address = format("[r%d], %s", rn, arm_single_data_transfer_get_address(instruction).c_str());
        }
    }
    
    return format("%s%s r%d, %s", name.c_str(), byte.c_str(), rd, address.c_str());
}

std::string Disassembler::arm_data_processing(u32 instruction) {
    u8 opcode = (instruction >> 21) & 0xF;

    switch (opcode) {
    case 0x0:
        return arm_data_processing_regular(instruction, "and");
    case 0x1:
        return arm_data_processing_regular(instruction, "eor");
    case 0x2:
        return arm_data_processing_regular(instruction, "sub");
    case 0x3:
        return arm_data_processing_regular(instruction, "rsb");
    case 0x4:
        return arm_data_processing_regular(instruction, "add");
    case 0x5:
        return arm_data_processing_regular(instruction, "adc");
    case 0x6:
        return arm_data_processing_regular(instruction, "sbc");
    case 0x7:
        return arm_data_processing_regular(instruction, "rsc");
    case 0x8:
        return arm_data_processing_only_operand(instruction, "tst");
    case 0x9:
        return arm_data_processing_only_operand(instruction, "teq");
    case 0xA:
        return arm_data_processing_only_operand(instruction, "cmp");
    case 0xB:
        return arm_data_processing_only_operand(instruction, "cmn");
    case 0xC:
        return arm_data_processing_regular(instruction, "orr");
    case 0xD:
        return arm_data_processing_only_destination(instruction, "mov");
    case 0xE:
        return arm_data_processing_regular(instruction, "bic");
    case 0xF:
        return arm_data_processing_only_destination(instruction, "mvn");
    default:
        return "";
    }
}

std::string Disassembler::arm_coprocessor_register_transfer(u32 instruction) {
    const bool opcode = (instruction >> 20) & 0x1;

    if (opcode) {
        return "mrc";
    } else {
        return "mcr";
    }
}

std::string Disassembler::arm_software_interrupt(u32 instruction) {
    u32 comment_field = instruction & 0xFFFFFF;

    return format("swi #0x%08x", comment_field);
}

std::string Disassembler::arm_signed_halfword_accumulate_long(u32 instruction) {
    return "";
}

std::string Disassembler::arm_signed_halfword_word_multiply(u32 instruction) {
    return "";
}

std::string Disassembler::arm_signed_halfword_multiply(u32 instruction) {
    return "";
}

std::string Disassembler::arm_breakpoint(u32 instruction) {
    return "";
}

std::string Disassembler::arm_data_processing_get_op2(u32 instruction) {
    const u8 shift_imm = (instruction >> 25) & 0x1;

    if (shift_imm) {
        u32 immediate = instruction & 0xFF;
        u8 shift_amount = ((instruction >> 8) & 0xF) << 1;

        return format("#0x%08x", Common::rotate_right(immediate, shift_amount));
    } else {
        u8 rm = instruction & 0xF;
        
        std::string shift_types[4] = {"lsl", "lsr", "asr", "ror"};
        std::string shift_type = shift_types[(instruction >> 5) & 0x3];

        bool immediate = !(instruction & (1 << 4));

        if (immediate) {
            u8 shift_amount = (instruction >> 7) & 0x1F;

            if (shift_amount) {
                return format("r%d", rm);
            }

            return format("r%d %s #0x%02x", rm, shift_type.c_str(), shift_amount);
        } else {
            u8 rs = (instruction >> 8) & 0xF;
            
            return format("r%d %s r%d", rm, shift_type.c_str(), rs);
        }
    }
}

std::string Disassembler::arm_data_processing_regular(u32 instruction, std::string opcode) {
    std::string set_flags = (instruction >> 20) & 0x1 ? "s" : "";
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    return format("%s%s r%d, r%d, %s", opcode.c_str(), set_flags.c_str(), rd, rn, arm_data_processing_get_op2(instruction).c_str());
}

std::string Disassembler::arm_data_processing_only_destination(u32 instruction, std::string opcode) {
    std::string set_flags = (instruction >> 20) & 0x1 ? "s" : "";
    u8 rd = (instruction >> 12) & 0xF;

    return format("%s%s r%d, %s", opcode.c_str(), set_flags.c_str(), rd, arm_data_processing_get_op2(instruction).c_str());
}

std::string Disassembler::arm_data_processing_only_operand(u32 instruction, std::string opcode) {
    u8 rn = (instruction >> 16) & 0xF;

    return format("%s r%d, %s", opcode.c_str(), rn, arm_data_processing_get_op2(instruction).c_str());
}

std::string Disassembler::arm_single_data_transfer_get_address(u32 instruction) {
    const bool shifted_register = (instruction >> 25) & 0x1;

    if (shifted_register) {
        return "handle";
    } else {
        return format("#0x%08x", instruction & 0xFFF);
    }
}
