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
    log_fatal("handle bx");
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