#include "common/logger.h"
#include "arm/jit/jit.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

Jit::Jit(Arch arch, Memory& memory, Coprocessor& coprocessor) : arch(arch), memory(memory), coprocessor(coprocessor), translator(*this) {
    // configure jit settings
    // TODO: use a global settings struct to configure the jit
    config.block_size = 32;
}

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
    cycles_available += cycles;

    while (cycles_available > 0) {
        if (halted) {
            return;
        }

        if (irq && !state.cpsr.i) {
            handle_interrupt();
        }

        BasicBlock::Key key{state};

        // logger.todo("Jit: handle execution at pc %08x mode %02x is arm %d key %016lx", key.get_pc(), static_cast<int>(key.get_mode()), key.is_arm(), key.value);

        BasicBlock* basic_block = block_cache.get(key);
        if (!basic_block) {
            basic_block = compile(key);
            logger.todo("Jit: handle compilation to ir opcodes");
        }
    }
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

u32 Jit::get_gpr(GPR gpr) {
    return state.gpr[gpr];
}

void Jit::set_gpr(GPR gpr, u32 value) {
    if (gpr == GPR::PC) {
        // for pc writes, we need to consider the effects of the pipeline
        if (state.cpsr.t) {
            state.gpr[GPR::PC] = value + 4;
        } else {
            state.gpr[GPR::PC] = value + 8;
        }
    } else {
        state.gpr[gpr] = value;
    }
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

BasicBlock* Jit::compile(BasicBlock::Key key) {
    BasicBlock* basic_block = new BasicBlock(key);
    translator.translate(*basic_block);

    // TODO: compile with selected backend

    return basic_block;
}

} // namespace arm