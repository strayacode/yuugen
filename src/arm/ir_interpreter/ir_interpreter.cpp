#include "common/logger.h"
#include "arm/ir_interpreter/ir_interpreter.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

IRInterpreter::IRInterpreter(Arch arch, Memory& memory, Coprocessor& coprocessor) : arch(arch), memory(memory), coprocessor(coprocessor) {}

void IRInterpreter::reset() {
    state.gpr.fill(0);

    for (int i = 0; i < 6; i++) {
        state.gpr_banked[i].fill(0);
        state.spsr_banked[i].data = 0;
    }

    state.cpsr.data = 0xd3;
    set_mode(Mode::SVC);
    pipeline.fill(0);
    irq = false;
    halted = false;
}

void IRInterpreter::run(int cycles) {
    logger.todo("IRInterpreter: handle execution");
}

void IRInterpreter::flush_pipeline() {
    if (state.cpsr.t) {
        thumb_flush_pipeline();
    } else {
        arm_flush_pipeline();
    }
}

void IRInterpreter::set_mode(Mode mode) {
    mode = static_cast<Mode>(static_cast<u8>(mode) & 0x1f);
    auto old_bank = get_bank(state.cpsr.mode);
    auto new_bank = get_bank(mode);

    if (new_bank != Bank::USR) {
        state.spsr = &state.spsr_banked[new_bank];
    } else {
        state.spsr = &state.cpsr;
    }

    state.cpsr.mode = mode;

    // no need to bank switch if no change in banks used
    if (old_bank == new_bank) {
        return;
    }

    if (old_bank == Bank::FIQ || new_bank == Bank::FIQ) {
        for (int i = 0; i < 7; i++) {
            state.gpr_banked[old_bank][i] = state.gpr[i + 8];
        }

        for (int i = 0; i < 7; i++) {
            state.gpr[i + 8] = state.gpr_banked[new_bank][i];
        }
    } else {
        state.gpr_banked[old_bank][5] = state.gpr[13];
        state.gpr_banked[old_bank][6] = state.gpr[14];

        state.gpr[13] = state.gpr_banked[new_bank][5];
        state.gpr[14] = state.gpr_banked[new_bank][6];
    }
}

void IRInterpreter::update_irq(bool irq) {
    this->irq = irq;
}

bool IRInterpreter::is_halted() {
    return halted;
}

void IRInterpreter::update_halted(bool halted) {
    this->halted = halted;
}

Arch IRInterpreter::get_arch() {
    return arch;
}

void IRInterpreter::illegal_instruction() {
    logger.error("IRInterpreter: illegal instruction %08x at pc = %08x", instruction, state.gpr[15]);
}

void IRInterpreter::arm_flush_pipeline() {
    state.gpr[15] &= ~3;
    pipeline[0] = code_read_word(state.gpr[15]);
    pipeline[1] = code_read_word(state.gpr[15] + 4);
    state.gpr[15] += 8;
}

void IRInterpreter::thumb_flush_pipeline() {
    state.gpr[15] &= ~1;
    pipeline[0] = code_read_half(state.gpr[15]);
    pipeline[1] = code_read_half(state.gpr[15] + 2);
    state.gpr[15] += 4;
}

Bank IRInterpreter::get_bank(Mode mode) {
    switch (mode) {
    case Mode::USR: case Mode::SYS:
        return Bank::USR;
    case Mode::FIQ:
        return Bank::FIQ;
    case Mode::IRQ:
        return Bank::IRQ;
    case Mode::SVC:
        return Bank::SVC;
    case Mode::ABT:
        return Bank::ABT;
    case Mode::UND:
        return Bank::UND;
    default:
        logger.warn("IRInterpreter: mode %02x doesn't have a bank", static_cast<u8>(mode));
        return Bank::USR;
    }
}

u16 IRInterpreter::code_read_half(u32 addr) {
    return memory.read<u16, Bus::Code>(addr);
}

u32 IRInterpreter::code_read_word(u32 addr) {
    return memory.read<u32, Bus::Code>(addr);
}

u8 IRInterpreter::read_byte(u32 addr) {
    return memory.read<u8, Bus::Data>(addr);
}

u16 IRInterpreter::read_half(u32 addr) {
    return memory.read<u16, Bus::Data>(addr);
}

u32 IRInterpreter::read_word(u32 addr) {
    return memory.read<u32, Bus::Data>(addr);
}

u32 IRInterpreter::read_word_rotate(u32 addr) {
    u32 value = memory.read<u32, Bus::Data>(addr);
    int amount = (addr & 0x3) * 8;
    return common::rotate_right(value, amount);
}

void IRInterpreter::write_byte(u32 addr, u8 data) {
    // TODO: handle potential code invalidation
    memory.write<u8, Bus::Data>(addr, data);
}

void IRInterpreter::write_half(u32 addr, u16 data) {
    // TODO: handle potential code invalidation
    memory.write<u16, Bus::Data>(addr, data);
}

void IRInterpreter::write_word(u32 addr, u32 data) {
    // TODO: handle potential code invalidation
    memory.write<u32, Bus::Data>(addr, data);
}

} // namespace arm