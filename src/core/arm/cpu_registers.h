#pragma once

#include <common/types.h>

struct CPURegisters {
    // 16 general purpose registers
    u32 r[16];

    // this stores 5 banks of registers from r8-r14
    u32 r_banked[6][7];

    // the current program status register
    u32 cpsr;

    // this is for the banked spsrs
    u32 spsr_banked[6];
};
