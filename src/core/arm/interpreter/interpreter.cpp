#include "core/arm/interpreter/interpreter.h"

namespace core::arm {

static Decoder<Interpreter> decoder;

Interpreter::Interpreter(Arch arch, Memory& memory, Coprocessor& coprocessor) : arch(arch), memory(memory), coprocessor(coprocessor) {}

void Interpreter::reset() {
    state.gpr.fill(0);

    for (int i = 0; i < 6; i++) {
        state.gpr_banked[i].fill(0);
        state.spsr_banked[i].data = 0;
    }

    state.cpsr.data = 0xd3;
    set_mode(Mode::SVC);

    pipeline.fill(0);
}

void Interpreter::run(int cycles) {
    while (cycles--) {
        // TODO: handle interrupts in a nice way

        instruction = pipeline[0];
        pipeline[0] = pipeline[1];

        if (state.cpsr.t) {
            state.gpr[15] &= ~0x1;
            pipeline[1] = memory.read<u16, Bus::Code>(state.gpr[15]);

            auto handler = decoder.get_thumb_handler(instruction);
            (this->*handler)();
        } else {
            state.gpr[15] &= ~0x3;
            pipeline[1] = memory.read<u32, Bus::Code>(state.gpr[15]);

            // if (evaluate_condition(static_cast<Condition>(instruction >> 28))) {
            //     auto handler = decoder.get_arm_handler(instruction);
            //     (this->*handler)();
            // } else {
            //     state.gpr[15] += 4;
            // }
        }
    }
}

void Interpreter::jump_to(u32 addr) {
    state.gpr[15] = addr;

    if (state.cpsr.t) {
        thumb_flush_pipeline();
    } else {
        arm_flush_pipeline();
    }
}

void Interpreter::set_mode(Mode mode) {

}

void Interpreter::arm_flush_pipeline() {
    state.gpr[15] &= ~3;
    pipeline[0] = memory.read<u32, Bus::Code>(state.gpr[15]);
    pipeline[1] = memory.read<u32, Bus::Code>(state.gpr[15] + 4);
    state.gpr[15] += 8;
}

void Interpreter::thumb_flush_pipeline() {
    state.gpr[15] &= ~1;
    pipeline[0] = memory.read<u16, Bus::Code>(state.gpr[15]);
    pipeline[1] = memory.read<u16, Bus::Code>(state.gpr[15] + 2);
    state.gpr[15] += 4;
}

} // namespace core::arm