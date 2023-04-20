#include "common/logger.h"
#include "core/arm/interpreter/interpreter.h"

namespace core::arm {

void Interpreter::arm_branch_link_maybe_exchange() {
    // if ((instruction & 0xF0000000) != 0xF0000000) {
    //     arm_branch_link();
    // } else {
    //     arm_branch_link_exchange();
    // }
}

void Interpreter::arm_branch_exchange() {
    // u8 rm = instruction & 0xF;
    // if (state.gpr[rm] & 0x1) {
    //     // switch to thumb mode execution
    //     state.cpsr.t = true;
    //     state.gpr[15] = state.gpr[rm] & ~1;
    //     thumb_flush_pipeline();
    // } else {
    //     state.gpr[15] = state.gpr[rm] & ~3;
    //     arm_flush_pipeline();
    // }
}

void Interpreter::arm_branch_link() {
    // const bool link = (instruction >> 24) & 0x1;
    // u32 offset = ((instruction & (1 << 23)) ? 0xFC000000 : 0) | ((instruction & 0xFFFFFF) << 2);
    
    // if (link) {
    //     // store the address of the instruction after the current instruction in the link register
    //     state.gpr[14] = state.gpr[15] - 4;
    // }
    
    // // r15 is at instruction address + 8
    // state.gpr[15] += offset;

    // arm_flush_pipeline();
}

void Interpreter::arm_branch_link_exchange() {
    // if (arch == Arch::ARMv4) {
    //     return;
    // }

    // state.gpr[14] = state.gpr[15] - 4;
    // state.cpsr.t = true;

    // u32 offset = (((instruction & (1 << 23)) ? 0xFC000000: 0) | ((instruction & 0xFFFFFF) << 2)) + ((instruction & (1 << 24)) >> 23);
    // state.gpr[15] += offset;
    // thumb_flush_pipeline();
}

void Interpreter::arm_branch_link_exchange_register() {
    // if (arch == Arch::ARMv4) {
    //     return;
    // }

    // state.gpr[14] = state.gpr[15] - 4;

    // u8 rm = instruction & 0xF;
    // if (state.gpr[rm] & 0x1) {
    //     state.cpsr.t = true;
    //     state.gpr[15] = state.gpr[rm] & ~1;
    //     thumb_flush_pipeline();
    // } else {
    //     state.gpr[15] = state.gpr[rm] & ~3;
    //     arm_flush_pipeline();
    // }
}

void Interpreter::arm_software_interrupt() {
    // state.spsr_banked[BANK_SVC].data = state.cpsr.data;

    // switch_mode(MODE_SVC);

    // // disable interrupts
    // state.cpsr.i = true;
    // state.gpr[14] = state.gpr[15] - 4;

    // // jump to the exception base in the bios
    // state.gpr[15] = coprocessor.get_exception_base() + 0x08;
    // arm_flush_pipeline();
}

void Interpreter::arm_breakpoint() {
    logger.error("Interpreter: implement arm_breakpoint");
    // todo();
}

void Interpreter::thumb_branch_exchange() {
    // u8 rm = (instruction >> 3) & 0xF;
    // if (state.gpr[rm] & 0x1) {
    //     // just load rm into r15 normally in thumb
    //     state.gpr[15] = state.gpr[rm] & ~1;
    //     thumb_flush_pipeline();
    // } else {
    //     // switch to arm state
    //     // clear bit 5 in cpsr
    //     state.cpsr.t = false;
    //     state.gpr[15] = state.gpr[rm] & ~3;
    //     arm_flush_pipeline();
    // }
}

void Interpreter::thumb_branch_link_exchange() {
    // if (arch == Arch::ARMv4) {
    //     return;
    // }

    // u8 rm = (instruction >> 3) & 0xF;
    // u32 next_instruction_address = state.gpr[15] - 2;
    // state.gpr[14] = next_instruction_address | 1;
    
    // if (state.gpr[rm] & 0x1) {
    //     state.gpr[15] = state.gpr[rm] & ~1;
    //     thumb_flush_pipeline();
    // } else {
    //     state.cpsr.t = false;
    //     state.gpr[15] = state.gpr[rm] & ~3;
    //     arm_flush_pipeline();
    // }
}

void Interpreter::thumb_branch_link_setup() {
    // u32 immediate = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);

    // state.gpr[14] = state.gpr[15] + (immediate << 11);
    // state.gpr[15] += 2;
}

void Interpreter::thumb_branch_link_offset() {
    // u32 offset = (instruction & 0x7FF) << 1;
    // u32 next_instruction_address = state.gpr[15] - 1;

    // state.gpr[15] = (state.gpr[14] + offset) & ~1;
    // state.gpr[14] = next_instruction_address;
    // thumb_flush_pipeline();
}

void Interpreter::thumb_branch_link_exchange_offset() {
    // // arm9 specific instruction
    // if (arch == Arch::ARMv4) {
    //     return;
    // }

    // u32 offset = (instruction & 0x7FF) << 1;
    // u32 next_instruction_address = state.gpr[15] - 2;
    // state.gpr[15] = (state.gpr[14] + offset) & ~0x3;
    // state.gpr[14] = next_instruction_address | 1;

    // // set t flag to 0
    // state.cpsr.t = false;

    // // flush the pipeline
    // arm_flush_pipeline();
}

void Interpreter::thumb_branch() {
    // u32 offset = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);
    
    // state.gpr[15] += offset;
    // thumb_flush_pipeline();
}

void Interpreter::thumb_branch_conditional() {
    // u8 condition = (instruction >> 8) & 0xF;

    // if (evaluate_condition(condition)) {
    //     u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
    //     state.gpr[15] += offset;
    //     thumb_flush_pipeline();
    // } else {
    //     state.gpr[15] += 2;
    // }
}

void Interpreter::thumb_software_interrupt() {
    // state.spsr_banked[BANK_SVC].data = state.cpsr.data;

    // switch_mode(MODE_SVC);

    // state.cpsr.t = false;
    // state.cpsr.i = true;
    // state.gpr[14] = state.gpr[15] - 2;

    // // jump to the exception base in the bios
    // state.gpr[15] = coprocessor.get_exception_base() + 0x08;
    // arm_flush_pipeline();
}

} // namespace core::arm