#include "common/logger.h"
#include "common/types.h"
#include "common/bits.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/jit.h"
#include "arm/decoder.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

static Decoder<Translator> decoder;
static Disassembler disassembler;

Translator::Translator(Jit& jit, IREmitter& ir) : jit(jit), ir(ir) {}

void Translator::translate() {
    auto& basic_block = ir.basic_block;
    auto location = basic_block.location;
    
    logger.debug("Translator: translate basic block instruction size %d pc %08x", location.get_instruction_size(), location.get_address());
    
    for (int i = 0; i < jit.config.block_size; i++) {
        if (location.is_arm()) {
            instruction = code_read_word(basic_block.current_address);
            auto condition = evaluate_arm_condition();

            if (i == 0) {
                // if this is the first instruction in the block then that
                // will signify the condition of all instructions in the block
                basic_block.condition = condition;
            } else if (condition != basic_block.condition) {
                // if any of the following instructions doesn't have the same condition
                // then the block is terminated
                break;
            }

            logger.debug("%s %08x %08x", disassembler.disassemble_arm(instruction).c_str(), instruction, basic_block.current_address);
            
            auto handler = decoder.get_arm_handler(instruction);
            auto status = (this->*handler)();

            // TODO: for now each instruction takes only 1 cycle,
            // but in the future we should at compile time figure out instruction timings
            // with I, N and S cycles
            basic_block.cycles++;
            basic_block.num_instructions++;

            if (status == BlockStatus::Break) {
                break;
            }
        } else {
            instruction = code_read_half(basic_block.current_address);
            auto condition = evaluate_thumb_condition();

            if (i == 0) {
                // if this is the first instruction in the block then that
                // will signify the condition of all instructions in the block
                basic_block.condition = condition;
            } else if (condition != basic_block.condition) {
                // if any of the following instructions doesn't have the same condition
                // then the block is terminated
                break;
            }

            logger.debug("%s %08x %08x", disassembler.disassemble_thumb(instruction).c_str(), instruction, basic_block.current_address);
            
            auto handler = decoder.get_thumb_handler(instruction);
            auto status = (this->*handler)();

            // TODO: for now each instruction takes only 1 cycle,
            // but in the future we should at compile time figure out instruction timings
            // with I, N and S cycles
            basic_block.cycles++;
            basic_block.num_instructions++;

            if (status == BlockStatus::Break) {
                break;
            }
        }

        basic_block.advance();
    }

    basic_block.dump();
}

Translator::BlockStatus Translator::illegal_instruction() {
    logger.error("Translator: illegal instruction %08x at pc = %08x", instruction, jit.get_state().gpr[15]);
    return BlockStatus::Break;
}

u16 Translator::code_read_half(u32 addr) {
    return jit.memory.read<u16, Bus::Code>(addr);
}

u32 Translator::code_read_word(u32 addr) {
    return jit.memory.read<u32, Bus::Code>(addr);
}

Condition Translator::evaluate_arm_condition() {
    Condition condition = static_cast<Condition>(common::get_field<28, 4>(instruction));
    if (condition == Condition::NV && jit.arch == Arch::ARMv5) {
        // on armv5 nv instructions are treated as unconditional
        return Condition::AL;
    } else {
        return condition;
    }
}

Condition Translator::evaluate_thumb_condition() {
    auto type = common::get_field<12, 4>(instruction);
    if (type == 0xd) {
        Condition condition = static_cast<Condition>(common::get_field<8, 4>(instruction));
        if (condition == Condition::NV) {
            return Condition::AL;
        } else {
            return condition;
        }
    }

    return Condition::AL;
}

} // namespace arm