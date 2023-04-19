#include "core/arm/interpreter/interpreter.h"

namespace core::arm {

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