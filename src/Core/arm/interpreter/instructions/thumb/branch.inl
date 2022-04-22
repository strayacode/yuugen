#pragma once

void thumb_branch_exchange() {
    u8 rm = (instruction >> 3) & 0xF;
    if (regs.r[rm] & 0x1) {
        // just load rm into r15 normally in thumb
        regs.r[15] = regs.r[rm] & ~1;
        ThumbFlushPipeline();
    } else {
        // switch to arm state
        // clear bit 5 in cpsr
        regs.cpsr &= ~(1 << 5);
        regs.r[15] = regs.r[rm] & ~3;
        ARMFlushPipeline();
    }
}

void thumb_branch_link_exchange() {
    if (arch == CPUArch::ARMv4) {
        return;
    }

    u8 rm = (instruction >> 3) & 0xF;
    u32 next_instruction_address = regs.r[15] - 2;
    regs.r[14] = next_instruction_address | 1;
    
    if (regs.r[rm] & 0x1) {
        regs.r[15] = regs.r[rm] & ~1;
        ThumbFlushPipeline();
    } else {
        regs.cpsr &= ~(1 << 5);
        regs.r[15] = regs.r[rm] & ~3;
        ARMFlushPipeline();
    }
}

void thumb_branch_link_setup() {
    u32 immediate = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);

    regs.r[14] = regs.r[15] + (immediate << 11);
    regs.r[15] += 2;
}

void thumb_branch_link_offset() {
    u32 offset = (instruction & 0x7FF) << 1;
    u32 next_instruction_address = regs.r[15] - 1;

    regs.r[15] = (regs.r[14] + offset) & ~1;
    regs.r[14] = next_instruction_address;
    ThumbFlushPipeline();
}

void thumb_branch_link_exchange_offset() {
    // arm9 specific instruction
    if (arch == CPUArch::ARMv4) {
        return;
    }

    u32 offset = (instruction & 0x7FF) << 1;
    u32 next_instruction_address = regs.r[15] - 2;
    regs.r[15] = (regs.r[14] + offset) & ~0x3;
    regs.r[14] = next_instruction_address | 1;

    // set t flag to 0
    regs.cpsr &= ~(1 << 5);

    // flush the pipeline
    ARMFlushPipeline();
}

void thumb_branch() {
    u32 offset = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);
    
    regs.r[15] += offset;
    ThumbFlushPipeline();
}

void thumb_branch_conditional() {
    u8 condition = (instruction >> 8) & 0xF;

    if (ConditionEvaluate(condition)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}