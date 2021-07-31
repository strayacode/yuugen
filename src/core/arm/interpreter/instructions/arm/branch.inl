#pragma once

template <bool link>
void ARMBranchLink() {
    u32 offset = ((instruction & (1 << 23)) ? 0xFC000000 : 0) | ((instruction & 0xFFFFFF) << 2);
    
    if constexpr (link) {
        // store the address of the instruction after the current instruction in the link register
        regs.r[14] = regs.r[15] - 4;
    }
    
    // r15 is at instruction address + 8
    regs.r[15] += offset;

    ARMFlushPipeline();
}

void ARMBranchExchange() {
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

void ARMBranchLinkExchange() {
    log_fatal("handle blx");
}

void ARMBranchLinkExchangeRegister() {
    log_fatal("handle blx reg");
}

void ARMSoftwareInterrupt() {
    log_fatal("handle swi");
}

void ARMBreakpoint() {
    log_fatal("handle bkpt");
}

void ARMUndefined() {
    log_fatal("handle undefined");
}