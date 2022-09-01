#include "Core/ARM/Interpreter/Interpreter.h"

void Interpreter::arm_branch_link_maybe_exchange() {
    if ((m_instruction & 0xF0000000) != 0xF0000000) {
        arm_branch_link();
    } else {
        arm_branch_link_exchange();
    }
}

void Interpreter::arm_branch_exchange() {
    u8 rm = m_instruction & 0xF;
    if (m_gpr[rm] & 0x1) {
        // switch to thumb mode execution
        m_cpsr.t = true;
        m_gpr[15] = m_gpr[rm] & ~1;
        thumb_flush_pipeline();
    } else {
        m_gpr[15] = m_gpr[rm] & ~3;
        arm_flush_pipeline();
    }
}

void Interpreter::arm_branch_link() {
    const bool link = (m_instruction >> 24) & 0x1;
    u32 offset = ((m_instruction & (1 << 23)) ? 0xFC000000 : 0) | ((m_instruction & 0xFFFFFF) << 2);
    
    if (link) {
        // store the address of the instruction after the current instruction in the link register
        m_gpr[14] = m_gpr[15] - 4;
    }
    
    // r15 is at instruction address + 8
    m_gpr[15] += offset;

    arm_flush_pipeline();
}

void Interpreter::arm_branch_link_exchange() {
    if (m_arch == Arch::ARMv4) {
        return;
    }

    m_gpr[14] = m_gpr[15] - 4;
    m_cpsr.t = true;

    u32 offset = (((m_instruction & (1 << 23)) ? 0xFC000000: 0) | ((m_instruction & 0xFFFFFF) << 2)) + ((m_instruction & (1 << 24)) >> 23);
    m_gpr[15] += offset;
    thumb_flush_pipeline();
}

void Interpreter::arm_branch_link_exchange_register() {
    if (m_arch == Arch::ARMv4) {
        return;
    }

    m_gpr[14] = m_gpr[15] - 4;

    u8 rm = m_instruction & 0xF;
    if (m_gpr[rm] & 0x1) {
        m_cpsr.t = true;
        m_gpr[15] = m_gpr[rm] & ~1;
        thumb_flush_pipeline();
    } else {
        m_gpr[15] = m_gpr[rm] & ~3;
        arm_flush_pipeline();
    }
}

void Interpreter::arm_software_interrupt() {
    m_spsr_banked[BANK_SVC].data = m_cpsr.data;

    switch_mode(MODE_SVC);

    // disable interrupts
    m_cpsr.i = true;
    m_gpr[14] = m_gpr[15] - 4;

    // jump to the exception base in the bios
    m_gpr[15] = m_coprocessor.get_exception_base() + 0x08;
    arm_flush_pipeline();
}

void Interpreter::arm_breakpoint() {
    todo();
}

void Interpreter::thumb_branch_exchange() {
    u8 rm = (m_instruction >> 3) & 0xF;
    if (m_gpr[rm] & 0x1) {
        // just load rm into r15 normally in thumb
        m_gpr[15] = m_gpr[rm] & ~1;
        thumb_flush_pipeline();
    } else {
        // switch to arm state
        // clear bit 5 in cpsr
        m_cpsr.t = false;
        m_gpr[15] = m_gpr[rm] & ~3;
        arm_flush_pipeline();
    }
}

void Interpreter::thumb_branch_link_exchange() {
    if (m_arch == Arch::ARMv4) {
        return;
    }

    u8 rm = (m_instruction >> 3) & 0xF;
    u32 next_instruction_address = m_gpr[15] - 2;
    m_gpr[14] = next_instruction_address | 1;
    
    if (m_gpr[rm] & 0x1) {
        m_gpr[15] = m_gpr[rm] & ~1;
        thumb_flush_pipeline();
    } else {
        m_cpsr.t = false;
        m_gpr[15] = m_gpr[rm] & ~3;
        arm_flush_pipeline();
    }
}

void Interpreter::thumb_branch_link_setup() {
    u32 immediate = ((m_instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((m_instruction & 0x7FF) << 1);

    m_gpr[14] = m_gpr[15] + (immediate << 11);
    m_gpr[15] += 2;
}

void Interpreter::thumb_branch_link_offset() {
    u32 offset = (m_instruction & 0x7FF) << 1;
    u32 next_instruction_address = m_gpr[15] - 1;

    m_gpr[15] = (m_gpr[14] + offset) & ~1;
    m_gpr[14] = next_instruction_address;
    thumb_flush_pipeline();
}

void Interpreter::thumb_branch_link_exchange_offset() {
    // arm9 specific instruction
    if (m_arch == Arch::ARMv4) {
        return;
    }

    u32 offset = (m_instruction & 0x7FF) << 1;
    u32 next_instruction_address = m_gpr[15] - 2;
    m_gpr[15] = (m_gpr[14] + offset) & ~0x3;
    m_gpr[14] = next_instruction_address | 1;

    // set t flag to 0
    m_cpsr.t = false;

    // flush the pipeline
    arm_flush_pipeline();
}

void Interpreter::thumb_branch() {
    u32 offset = ((m_instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((m_instruction & 0x7FF) << 1);
    
    m_gpr[15] += offset;
    thumb_flush_pipeline();
}

void Interpreter::thumb_branch_conditional() {
    u8 condition = (m_instruction >> 8) & 0xF;

    if (evaluate_condition(condition)) {
        u32 offset = ((m_instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((m_instruction & 0xFF) << 1);
        m_gpr[15] += offset;
        thumb_flush_pipeline();
    } else {
        m_gpr[15] += 2;
    }
}

void Interpreter::thumb_software_interrupt() {
    m_spsr_banked[BANK_SVC].data = m_cpsr.data;

    switch_mode(MODE_SVC);

    m_cpsr.t = false;
    m_cpsr.i = true;
    m_gpr[14] = m_gpr[15] - 2;

    // jump to the exception base in the bios
    m_gpr[15] = m_coprocessor.get_exception_base() + 0x08;
    arm_flush_pipeline();
}