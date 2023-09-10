#include "common/logger.h"
#include "common/types.h"
#include "common/bits.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/jit.h"
#include "arm/decoder.h"

namespace arm {

static Decoder<Translator> decoder;

Translator::Translator(Arch arch, Jit& jit) : arch(arch), jit(jit) {}

void Translator::translate(BasicBlock& basic_block) {
    Emitter emitter{basic_block};
    auto location = basic_block.location;
    instruction_size = location.get_instruction_size();
    code_address = location.get_address() - 2 * instruction_size;
    
    logger.debug("Translator: translate basic block instruction size %d pc %08x", instruction_size, code_address);

    for (int i = 0; i < jit.config.block_size; i++) {
        // TODO: for now each instruction takes only 1 cycle,
        // but in the future we should at compile time figure out instruction timings
        // with I, N and S cycles
        basic_block.cycles++;

        basic_block.num_instructions++;

        if (location.is_arm()) {
            instruction = code_read_word(code_address);
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

            auto handler = decoder.get_arm_handler(instruction);
            auto status = (this->*handler)(emitter);

            if (status == BlockStatus::Break) {
                break;
            }
        } else {
            instruction = code_read_half(code_address);
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

            auto handler = decoder.get_thumb_handler(instruction);
            auto status = (this->*handler)(emitter);

            if (status == BlockStatus::Break) {
                break;
            }
        }

        code_address += instruction_size;
    }

    basic_block.dump();
}

Translator::BlockStatus Translator::illegal_instruction([[maybe_unused]] Emitter& emitter) {
    logger.error("Translator: illegal instruction %08x at pc = %08x", instruction, jit.get_state().gpr[15]);
    return BlockStatus::Break;
}

void Translator::emit_advance_pc(Emitter& emitter) {
    emitter.store_gpr(GPR::PC, IRConstant{code_address + instruction_size});
}

void Translator::emit_link(Emitter& emitter) {
    emitter.store_gpr(GPR::LR, IRConstant{code_address + instruction_size});
}

IRVariable Translator::emit_barrel_shifter(Emitter& emitter, IRValue value, ShiftType shift_type, IRValue amount, bool set_carry) {
    switch (shift_type) {
    case ShiftType::LSL:
        return emitter.logical_shift_left(value, amount, set_carry);
    case ShiftType::LSR:
        return emitter.logical_shift_right(value, amount, set_carry);
    case ShiftType::ASR:
        return emitter.arithmetic_shift_right(value, amount, set_carry);
    case ShiftType::ROR:
        return emitter.rotate_right(value, amount, set_carry);
    }
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