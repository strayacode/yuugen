#include "common/logger.h"
#include "arm/interpreter/interpreter.h"
#include "arm/disassembler/disassembler.h"

namespace arm {

static Decoder<Interpreter> decoder;
static Disassembler disassembler;

Interpreter::Interpreter(Arch arch, Memory& memory, Coprocessor& coprocessor) : arch(arch), memory(memory), coprocessor(coprocessor) {
    generate_condition_table();
}

void Interpreter::reset() {
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

bool log = true;
u64 i = 0;

void Interpreter::run(int cycles) {
    while (cycles--) {
        if (halted) {
            return;
        }

        if (irq && !state.cpsr.i) {
            handle_interrupt();
        }

        instruction = pipeline[0];
        pipeline[0] = pipeline[1];

        if (i == 1000000) {
            log = false;
        }

        if (log) {
            logger.log("arch %d ", static_cast<int>(arch));
            log_state();
        }

        if (state.cpsr.t) {
            state.gpr[15] &= ~0x1;
            pipeline[1] = code_read_half(state.gpr[15]);

            auto handler = decoder.get_thumb_handler(instruction);
            (this->*handler)();
        } else {
            state.gpr[15] &= ~0x3;
            pipeline[1] = code_read_word(state.gpr[15]);

            if (evaluate_condition(static_cast<Condition>(instruction >> 28))) {
                auto handler = decoder.get_arm_handler(instruction);
                (this->*handler)();
            } else {
                state.gpr[15] += 4;
            }
        }

        i++;
    }
}

void Interpreter::flush_pipeline() {
    if (state.cpsr.t) {
        thumb_flush_pipeline();
    } else {
        arm_flush_pipeline();
    }
}

void Interpreter::set_mode(Mode mode) {
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

void Interpreter::update_irq(bool irq) {
    this->irq = irq;
}

bool Interpreter::is_halted() {
    return halted;
}

void Interpreter::update_halted(bool halted) {
    this->halted = halted;
}

Arch Interpreter::get_arch() {
    return arch;
}

void Interpreter::illegal_instruction() {
    logger.error("Interpreter: illegal instruction %08x at pc = %08x", instruction, state.gpr[15]);
}

void Interpreter::arm_flush_pipeline() {
    state.gpr[15] &= ~3;
    pipeline[0] = code_read_word(state.gpr[15]);
    pipeline[1] = code_read_word(state.gpr[15] + 4);
    state.gpr[15] += 8;
}

void Interpreter::thumb_flush_pipeline() {
    state.gpr[15] &= ~1;
    pipeline[0] = code_read_half(state.gpr[15]);
    pipeline[1] = code_read_half(state.gpr[15] + 2);
    state.gpr[15] += 4;
}

void Interpreter::generate_condition_table() {
    for (int i = 0; i < 16; i++) {
        bool n = i & 8;
        bool z = i & 4;
        bool c = i & 2;
        bool v = i & 1;

        condition_table[Condition::EQ][i] = z;
        condition_table[Condition::NE][i] = !z;
        condition_table[Condition::CS][i] = c;
        condition_table[Condition::CC][i] = !c;
        condition_table[Condition::MI][i] = n;
        condition_table[Condition::PL][i] = !n;
        condition_table[Condition::VS][i] = v;
        condition_table[Condition::VC][i] = !v;
        condition_table[Condition::HI][i] = (c && !z);
        condition_table[Condition::LS][i] = (!c || z);
        condition_table[Condition::GE][i] = (n == v);
        condition_table[Condition::LT][i] = (n != v);
        condition_table[Condition::GT][i] = (!z && (n == v));
        condition_table[Condition::LE][i] = (z || (n != v));
        condition_table[Condition::AL][i] = true;

        // this one is architecture and instruction dependent
        condition_table[Condition::NV][i] = true;
    }
}

bool Interpreter::evaluate_condition(Condition condition) {
    if (condition == Condition::NV) {
        return (arch == Arch::ARMv5) && (instruction & 0x0e000000) == 0xa000000;
    }

    return condition_table[condition][state.cpsr.data >> 28];
}

Bank Interpreter::get_bank(Mode mode) {
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
        logger.warn("Interpreter: mode %02x doesn't have a bank", static_cast<u8>(mode));
        return Bank::USR;
    }
}

void Interpreter::set_nz(u32 result) {
    state.cpsr.n = result >> 31;
    state.cpsr.z = result == 0;
}

u16 Interpreter::code_read_half(u32 addr) {
    return memory.read<u16, Bus::Code>(addr);
}

u32 Interpreter::code_read_word(u32 addr) {
    return memory.read<u32, Bus::Code>(addr);
}

u8 Interpreter::read_byte(u32 addr) {
    return memory.read<u8, Bus::Data>(addr);
}

u16 Interpreter::read_half(u32 addr) {
    return memory.read<u16, Bus::Data>(addr);
}

u32 Interpreter::read_word(u32 addr) {
    return memory.read<u32, Bus::Data>(addr);
}

u32 Interpreter::read_word_rotate(u32 addr) {
    u32 value = memory.read<u32, Bus::Data>(addr);
    int amount = (addr & 0x3) * 8;
    return common::rotate_right(value, amount);
}

void Interpreter::write_byte(u32 addr, u8 data) {
    memory.write<u8, Bus::Data>(addr, data);
}

void Interpreter::write_half(u32 addr, u16 data) {
    memory.write<u16, Bus::Data>(addr, data);
}

void Interpreter::write_word(u32 addr, u32 data) {
    memory.write<u32, Bus::Data>(addr, data);
}

bool Interpreter::calculate_add_overflow(u32 op1, u32 op2, u32 result) {
    return (~(op1 ^ op2) & (op2 ^ result)) >> 31;
}

bool Interpreter::calculate_sub_overflow(u32 op1, u32 op2, u32 result) {
    return ((op1 ^ op2) & (op1 ^ result)) >> 31;
}

void Interpreter::handle_interrupt() {
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

    state.gpr[15] = coprocessor.get_exception_base() + 0x18;
    arm_flush_pipeline();
}

void Interpreter::undefined_exception() {
    logger.warn("Interpreter: undefined exception fired for instruction %08x at %08x", instruction, state.gpr[15]);

    state.spsr_banked[Bank::UND].data = state.cpsr.data;
    set_mode(Mode::UND);

    state.cpsr.i = true;
    state.gpr[14] = state.gpr[15] - 4;
    state.gpr[15] = coprocessor.get_exception_base() + 0x04;
    arm_flush_pipeline();
}

void Interpreter::log_state() {
    for (int i = 0; i < 16; i++) {
        logger.log("r%d: %08x ", i, state.gpr[i]);
    }

    logger.log("cpsr: %08x ", state.cpsr.data);
    logger.log("%08x\n", instruction);
}

} // namespace arm