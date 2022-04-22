#pragma once

void arm_branch_link_maybe_exchange() {
    if ((instruction & 0xF0000000) != 0xF0000000) {
        arm_branch_link();
    } else {
        arm_branch_link_exchange();
    }
}

void arm_branch_link() {
    const bool link = (instruction >> 24) & 0x1;
    u32 offset = ((instruction & (1 << 23)) ? 0xFC000000 : 0) | ((instruction & 0xFFFFFF) << 2);
    
    if (link) {
        // store the address of the instruction after the current instruction in the link register
        regs.r[14] = regs.r[15] - 4;
    }
    
    // r15 is at instruction address + 8
    regs.r[15] += offset;

    ARMFlushPipeline();
}

void arm_branch_exchange() {
    u8 rm = instruction & 0xF;
    if (regs.r[rm] & 0x1) {
        // set bit 5 of cpsr to switch to thumb state
        regs.cpsr |= (1 << 5);
        regs.r[15] = regs.r[rm] & ~1;
        ThumbFlushPipeline();
    } else {
        // no need to clear bit 5 of cpsr as we are already in arm mode
        regs.r[15] = regs.r[rm] & ~3;
        ARMFlushPipeline();
    }
}

void arm_branch_link_exchange() {
    if (arch == CPUArch::ARMv4) {
        return;
    }

    regs.r[14] = regs.r[15] - 4;
    regs.cpsr |= (1 << 5);

    u32 offset = (((instruction & (1 << 23)) ? 0xFC000000: 0) | ((instruction & 0xFFFFFF) << 2)) + ((instruction & (1 << 24)) >> 23);
    regs.r[15] += offset;
    ThumbFlushPipeline();
}

void arm_branch_link_exchange_register() {
    if (arch == CPUArch::ARMv4) {
        return;
    }

    regs.r[14] = regs.r[15] - 4;

    u8 rm = instruction & 0xF;
    if (regs.r[rm] & 0x1) {
        regs.cpsr |= (1 << 5);
        regs.r[15] = regs.r[rm] & ~1;
        ThumbFlushPipeline();
    } else {
        regs.r[15] = regs.r[rm] & ~3;
        ARMFlushPipeline();
    }
}

void arm_breakpoint() {
    log_fatal("handle bkpt");
}

void arm_undefined() {
    log_fatal("handle undefined");
}