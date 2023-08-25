#include "common/logger.h"
#include "arm/jit/jit.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

Jit::Jit(Arch arch, Memory& memory, Coprocessor& coprocessor) : arch(arch), memory(memory), coprocessor(coprocessor) {}

void Jit::reset() {
    state.gpr.fill(0);

    for (int i = 0; i < 6; i++) {
        state.gpr_banked[i].fill(0);
        state.spsr_banked[i].data = 0;
    }

    state.cpsr.data = 0xd3;
    set_mode(Mode::SVC);
    irq = false;
    halted = false;
    block_cache.reset();
    cycles_available = 0;
}

void Jit::run(int cycles) {
    // cycles_available += cycles;

    // if (cycles_available < cycles) {
    //     return;
    // }

    // while (cycles_available > 0) {
    //     if (halted) {
    //         return;
    //     }

    //     if (irq && !state.cpsr.i) {
    //         handle_interrupt();
    //     }

    //     BasicBlock* basic_block = block_cache.lookup()

    //     instruction = pipeline[0];
    //     pipeline[0] = pipeline[1];

    //     if (state.cpsr.t) {
    //         state.gpr[15] &= ~0x1;
    //         pipeline[1] = code_read_half(state.gpr[15]);

    //         auto handler = decoder.get_thumb_handler(instruction);
    //         (this->*handler)();
    //     } else {
    //         state.gpr[15] &= ~0x3;
    //         pipeline[1] = code_read_word(state.gpr[15]);

    //         if (evaluate_condition(static_cast<Condition>(instruction >> 28))) {
    //             auto handler = decoder.get_arm_handler(instruction);
    //             (this->*handler)();
    //         } else {
    //             state.gpr[15] += 4;
    //         }
    //     }
    // }
    logger.todo("Jit: handle execution at pc %08x", state.gpr[15]);
}

void Jit::flush_pipeline() {
    // nop for the jit
}

void Jit::set_mode(Mode mode) {
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

void Jit::update_irq(bool irq) {
    this->irq = irq;
}

bool Jit::is_halted() {
    return halted;
}

void Jit::update_halted(bool halted) {
    this->halted = halted;
}

Arch Jit::get_arch() {
    return arch;
}

Bank Jit::get_bank(Mode mode) {
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
        logger.warn("Jit: mode %02x doesn't have a bank", static_cast<u8>(mode));
        return Bank::USR;
    }
}

u16 Jit::code_read_half(u32 addr) {
    return memory.read<u16, Bus::Code>(addr);
}

u32 Jit::code_read_word(u32 addr) {
    return memory.read<u32, Bus::Code>(addr);
}

u8 Jit::read_byte(u32 addr) {
    return memory.read<u8, Bus::Data>(addr);
}

u16 Jit::read_half(u32 addr) {
    return memory.read<u16, Bus::Data>(addr);
}

u32 Jit::read_word(u32 addr) {
    return memory.read<u32, Bus::Data>(addr);
}

u32 Jit::read_word_rotate(u32 addr) {
    u32 value = memory.read<u32, Bus::Data>(addr);
    int amount = (addr & 0x3) * 8;
    return common::rotate_right(value, amount);
}

void Jit::write_byte(u32 addr, u8 data) {
    // TODO: handle potential code invalidation
    memory.write<u8, Bus::Data>(addr, data);
}

void Jit::write_half(u32 addr, u16 data) {
    // TODO: handle potential code invalidation
    memory.write<u16, Bus::Data>(addr, data);
}

void Jit::write_word(u32 addr, u32 data) {
    // TODO: handle potential code invalidation
    memory.write<u32, Bus::Data>(addr, data);
}

void Jit::handle_interrupt() {
    halted = false;
    state.spsr_banked[Bank::IRQ].data = state.cpsr.data;
    set_mode(Mode::IRQ);
    state.cpsr.i = true;
    
    if (state.cpsr.t) {
        state.cpsr.t = false;
        state.gpr[14] = state.gpr[15];
    } else {
        state.gpr[14] = state.gpr[15] - 4;
    }

    state.gpr[15] = coprocessor.get_exception_base() + 0x18 + 4;
}

} // namespace arm