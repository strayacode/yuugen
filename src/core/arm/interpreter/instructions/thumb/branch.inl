#pragma once

void ThumbBranchExchange() {
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

void ThumbBranchLinkSetup() {
    u32 immediate = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);

    regs.r[14] = regs.r[15] + (immediate << 11);
    regs.r[15] += 2;
}

void ThumbBranchLinkOffset() {
    u32 offset = (instruction & 0x7FF) << 1;
    u32 next_instruction_address = regs.r[15] - 1;

    regs.r[15] = (regs.r[14] + offset) & ~1;
    regs.r[14] = next_instruction_address;
    ThumbFlushPipeline();
}

void ThumbBranchLinkExchangeOffset() {
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

void ThumbBranch() {
    u32 offset = ((instruction & (1 << 10)) ? 0xFFFFF000 : 0) | ((instruction & 0x7FF) << 1);
    
    regs.r[15] += offset;
    ThumbFlushPipeline();
}

void ThumbBranchConditional() {
    u8 condition = (instruction >> 8) & 0xF;

    if (ConditionEvaluate(condition)) {
        u32 offset = ((instruction & (1 << 7)) ? 0xFFFFFE00 : 0) | ((instruction & 0xFF) << 1);
        regs.r[15] += offset;
        ThumbFlushPipeline();
    } else {
        regs.r[15] += 2;
    }
}

void ThumbSoftwareInterrupt() {
    log_fatal("handle")
}