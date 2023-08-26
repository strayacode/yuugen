#include "common/logger.h"
#include "common/types.h"
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
    // u32 addr = 
    

    logger.debug("Translator: instruction size %d pc %08x", instruction_size, key.get_pc());

    for (int i = 0; i < jit.config.block_size; i++) {
        if (is_arm) {
            logger.todo("Translator: handle arm translation");
        } else {
            logger.todo("Translator: handle thumb translation");
        }
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