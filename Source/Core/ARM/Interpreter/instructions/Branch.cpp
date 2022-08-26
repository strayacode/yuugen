#include "Core/ARM/Interpreter/Interpreter.h"

void Interpreter::arm_branch_link_maybe_exchange() {
    if ((m_instruction & 0xF0000000) != 0xF0000000) {
        arm_branch_link();
    } else {
        arm_branch_link_exchange();
    }
}

void Interpreter::arm_branch_exchange() {
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
    todo();
}

void Interpreter::arm_software_interrupt() {
    todo();
}

void Interpreter::arm_breakpoint() {
    todo();
}

void Interpreter::thumb_branch_link_exchange() {
    todo();
}

void Interpreter::thumb_branch_link_offset() {
    todo();
}

void Interpreter::thumb_branch_link_setup() {
    todo();
}

void Interpreter::thumb_branch_link_exchange_offset() {
    todo();
}

void Interpreter::thumb_branch() {
    todo();
}

void Interpreter::thumb_branch_exchange() {
    todo();
}

void Interpreter::thumb_software_interrupt() {
    todo();
}

void Interpreter::thumb_branch_conditional() {
    todo();
}