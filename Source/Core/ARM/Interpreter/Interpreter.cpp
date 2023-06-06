#include "Common/Bits.h"
#include "Common/log_file.h"
#include "Core/ARM/Interpreter/Interpreter.h"
#include "Core/ARM/Decoder.h"

static Decoder<Interpreter> s_decoder;

Interpreter::Interpreter(MemoryBase& memory, CoprocessorBase& coprocessor, Arch arch) : CPUBase(memory, coprocessor, arch) {
    generate_condition_table();
}

bool Interpreter::run(int cycles) {
    while (cycles--) {
        if (m_halted) {
            return true;
        }

#ifdef CPU_DEBUG
        if (m_watchpoints.contains(m_gpr[15])) {
            return false;
        }
#endif

        single_step();
    }

    return true;
}

u64 Interpreter::single_step() {
    if (m_ime && (m_ie & m_irf) && !m_cpsr.i) {
        handle_interrupt();
    }

    if (m_arch == Arch::ARMv5) {
        log_state();
    }

    // the instruction decoded previously is now executed
    m_instruction = m_pipeline[0];

    // the instruction fetched previously is now decoded
    m_pipeline[0] = m_pipeline[1];

    // fetch the next instruction for the pipeline
    if (is_arm()) {
        m_pipeline[1] = read_word(m_gpr[15]);
    } else {
        m_pipeline[1] = read_half(m_gpr[15]);
    }

    if (is_arm()) {
        Instruction inst = s_decoder.decode_arm(m_instruction);

        if (evaluate_condition(m_instruction >> 28)) {
            (this->*inst)();
        } else {
            m_gpr[15] += 4;
        }
    } else {
        Instruction inst = s_decoder.decode_thumb(m_instruction);
        (this->*inst)();
    }

    // assume 1 cycle per instruction for now
    return 1;
}

u8 Interpreter::read_byte(u32 addr) {
    return m_memory.read<u8>(addr);
}

u16 Interpreter::read_half(u32 addr) {
    return m_memory.read<u16>(addr);
}

u32 Interpreter::read_word(u32 addr) {
    return m_memory.read<u32>(addr);
}

u32 Interpreter::read_word_rotate(u32 addr) {
    u32 return_value = read_word(addr);

    if (addr & 0x3) {
        int shift_amount = (addr & 0x3) * 8;
        return_value = Common::rotate_right(return_value, shift_amount);
    }

    return return_value;
}

void Interpreter::write_byte(u32 addr, u8 data) {
    m_memory.write<u8>(addr, data);
}

void Interpreter::write_half(u32 addr, u16 data) {
    m_memory.write<u16>(addr, data);
}

void Interpreter::write_word(u32 addr, u32 data) {
    m_memory.write<u32>(addr, data);
}

void Interpreter::arm_flush_pipeline() {
    m_gpr[15] &= ~3;
    m_pipeline[0] = read_word(m_gpr[15]);
    m_pipeline[1] = read_word(m_gpr[15] + 4);
    m_gpr[15] += 8;
}

void Interpreter::thumb_flush_pipeline() {
    m_gpr[15] &= ~1;
    m_pipeline[0] = read_half(m_gpr[15]);
    m_pipeline[1] = read_half(m_gpr[15] + 2);
    m_gpr[15] += 4;
}

void Interpreter::handle_interrupt() {
    m_halted = false;
    
    m_spsr_banked[BANK_IRQ] = m_cpsr;

    switch_mode(MODE_IRQ);

    // disable interrupts
    m_cpsr.i = true;
    
    if (is_arm()) {
        m_gpr[14] = m_gpr[15] - 4;
    } else {
        // force execution in arm mode
        m_cpsr.t = false;
        m_gpr[14] = m_gpr[15];
    }

    // check the exception base and jump to the correct address in the bios
    // also only use cp15 exception base from control register if arm9
    m_gpr[15] = m_coprocessor.get_exception_base() + 0x18;
    
    arm_flush_pipeline();
}

void Interpreter::generate_condition_table() {
    // iterate through each combination of bits 28..31
    for (int i = 0; i < 16; i++) {
        bool n_flag = i & 8;
        bool z_flag = i & 4;
        bool c_flag = i & 2;
        bool v_flag = i & 1;

        m_condition_table[CONDITION_EQ][i] = z_flag;
        m_condition_table[CONDITION_NE][i] = !z_flag;
        m_condition_table[CONDITION_CS][i] = c_flag;
        m_condition_table[CONDITION_CC][i] = !c_flag;
        m_condition_table[CONDITION_MI][i] = n_flag;
        m_condition_table[CONDITION_PL][i] = !n_flag;
        m_condition_table[CONDITION_VS][i] = v_flag;
        m_condition_table[CONDITION_VC][i] = !v_flag;
        m_condition_table[CONDITION_HI][i] = (c_flag && !z_flag);
        m_condition_table[CONDITION_LS][i] = (!c_flag || z_flag);
        m_condition_table[CONDITION_GE][i] = (n_flag == v_flag);
        m_condition_table[CONDITION_LT][i] = (n_flag != v_flag);
        m_condition_table[CONDITION_GT][i] = (!z_flag && (n_flag == v_flag));
        m_condition_table[CONDITION_LE][i] = (z_flag || (n_flag != v_flag));
        m_condition_table[CONDITION_AL][i] = true;

        // this one depends on architecture and instruction
        m_condition_table[CONDITION_NV][i] = true;
    }
}

bool Interpreter::evaluate_condition(u32 condition) {
    if (condition == CONDITION_NV) {
        if (m_arch == Arch::ARMv5) {
            // using arm decoding table this can only occur for branch and branch with link and change to thumb
            // so where bits 25..27 is 0b101
            if ((m_instruction & 0x0E000000) == 0xA000000) {
                return true;
            }
        } else {
            return false;
        }
    }

    return m_condition_table[condition][m_cpsr.data >> 28];
}

void Interpreter::unknown_instruction() {
    log_fatal("[Interpreter] Instruction %08x is unimplemented at r15 = %08x", m_instruction, m_gpr[15]);
} 

void Interpreter::log_state() {
    for (int i = 0; i < 16; i++) {
        LogFile::Get().Log("r%d: %08x ", i, m_gpr[i]);
    }

    LogFile::Get().Log("cpsr: %08x ", m_cpsr.data);
    LogFile::Get().Log("%08x\n", m_instruction);
}