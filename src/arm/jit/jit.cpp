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

        BasicBlock* basic_block = block_cache.get(key);
        if (!basic_block) {
            basic_block = compile(key);
            logger.todo("Jit: handle compilation to ir opcodes");
        }

        logger.todo("Jit: handle compilation of ir opcodes into executable code");
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
    return get_gpr(gpr, state.cpsr.mode);
}

u32 Jit::get_gpr(GPR gpr, Mode mode) {
    return *get_pointer_to_gpr(gpr, mode);
}

void Jit::set_gpr(GPR gpr, u32 value) {
    set_gpr(gpr, state.cpsr.mode, value);
}

void Jit::set_gpr(GPR gpr, Mode mode, u32 value) {
    if (gpr == GPR::PC) {
        // for pc writes, we need to consider the effects of the pipeline
        if (state.cpsr.t) {
            state.gpr[GPR::PC] = value + 4;
        } else {
            state.gpr[GPR::PC] = value + 8;
        }
    } else {
        *get_pointer_to_gpr(gpr, mode) = value;
    }
}

StatusRegister Jit::get_cpsr() {
    return state.cpsr;
}

void Jit::set_cpsr(StatusRegister value) {
    state.cpsr = value;
}

StatusRegister Jit::get_spsr(Mode mode) {
    return *get_pointer_to_spsr(mode);
}

void Jit::set_spsr(Mode mode, StatusRegister value) {
    *get_pointer_to_spsr(mode) = value;
}

bool Jit::has_spsr(Mode mode) {
    return mode != Mode::USR && mode != Mode::SYS;
}

u32* Jit::get_pointer_to_gpr(GPR gpr, Mode mode) {
    // TODO: profile this to see if it should be optimised
    auto start = mode == Mode::FIQ ? GPR::R8 : GPR::R13;
    if (has_spsr(mode) && gpr >= start && gpr <= GPR::R14) {
        return &state.gpr_banked[get_bank_from_mode(mode)][gpr - 8];
    } else {
        return &state.gpr[gpr];
    }
}

StatusRegister* Jit::get_pointer_to_cpsr() {
    return &state.cpsr;
}

StatusRegister* Jit::get_pointer_to_spsr(Mode mode) {
    if (!has_spsr(mode)) {
        logger.error("Jit: mode %02x doesn't have a spsr", static_cast<u8>(mode));
    }

    return &state.spsr_banked[get_bank_from_mode(mode)];
}

Bank Jit::get_bank_from_mode(Mode mode) {
    switch (mode) {
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
    logger.todo("Jit: handle interrupts");
}

BasicBlock* Jit::compile(BasicBlock::Key key) {
    BasicBlock* basic_block = new BasicBlock(key);
    translator.translate(*basic_block);

    // TODO: compile with selected backend

    return basic_block;
}

} // namespace arm