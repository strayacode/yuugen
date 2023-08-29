#include "common/logger.h"
#include "common/types.h"
#include "common/bits.h"
#include "arm/jit/ir/translator.h"
#include "arm/jit/ir/emitter.h"
#include "arm/jit/jit.h"
#include "arm/decoder.h"

namespace arm {

static Decoder<Translator> decoder;

Translator::Translator(Jit& jit) : jit(jit) {}

void Translator::translate(BasicBlock& basic_block) {
    Emitter emitter{basic_block};
    auto key = basic_block.key;
    auto is_arm = key.is_arm();
    auto instruction_size = key.is_arm() ? sizeof(u32) : sizeof(u16);
    u32 code_address = key.get_address() - 2 * instruction_size;
    
    logger.debug("Translator: translate basic block instruction size %d pc %08x", instruction_size, code_address);

    for (int i = 0; i < jit.config.block_size; i++) {
        if (is_arm) {
            instruction = code_read_word(code_address);
            Condition condition = static_cast<Condition>(common::get_field<28, 4>(instruction));

            if (condition == Condition::NV && jit.arch == Arch::ARMv5) {
                // on armv5 nv instructions are treated as unconditional
                condition = Condition::AL;
            }

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
            (this->*handler)();
        } else {
            logger.todo("Translator: handle thumb translation");
        }

        code_address += instruction_size;
    }
}

void Translator::illegal_instruction() {
    logger.error("Translator: illegal instruction %08x at pc = %08x", instruction, jit.get_state().gpr[15]);
}

u16 Translator::code_read_half(u32 addr) {
    return jit.memory.read<u16, Bus::Code>(addr);
}

u32 Translator::code_read_word(u32 addr) {
    return jit.memory.read<u32, Bus::Code>(addr);
}

} // namespace arm