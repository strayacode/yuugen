#pragma once
#include <nds/common/types.h>

class NDS;

class ARM7 {
public:
    ARM7(NDS *nds);

    void reset();
    void step();

private:
    NDS *nds;

    enum cpu_modes {
        USR = 0x10,
        SYS = 0x1F,
        FIQ = 0x11,
        SVC = 0x13,
        ABT = 0x17,
        IRQ = 0x12,
        UND = 0x1B,
    };

    enum condition_codes {
        // each number specifies the bit to access in cpsr
        N_FLAG = 31,
        Z_FLAG = 30,
        C_FLAG = 29,
        V_FLAG = 28,
    };

    enum instruction_modes {
        ARM,
        THUMB,
    };

    struct registers {
        u32 r0;
        u32 r1;
        u32 r2;
        u32 r3;
        u32 r4;
        u32 r5;
        u32 r6;
        u32 r7;
        u32 r8;
        u32 r9;
        u32 r10;
        u32 r11;
        u32 r12;

        // r13: stack pointer
        u32 r13;

        // r14: link register
        u32 r14;

        // r15: program counter
        u32 r15;

        u32 cpsr;


        // fiq mode registers
        u32 r8_fiq;
        u32 r9_fiq;
        u32 r10_fiq;
        u32 r11_fiq;
        u32 r12_fiq;

        // r13: stack pointer
        u32 r13_fiq;

        // r14: link register
        u32 r14_fiq;

        u32 spsr_fiq;

        // svc mode registers
        u32 r13_svc;
        u32 r14_svc;
        u32 spsr_svc;

        // abt mode registers
        u32 r13_abt;
        u32 r14_abt;
        u32 spsr_abt;

        // irq mode registers
        u32 r13_irq;
        u32 r14_irq;
        u32 spsr_irq;

        // und mode registers
        u32 r13_und;
        u32 r14_und;
        u32 spsr_und;

    } regs;

    u32 opcode; // used for storing the first instruction in the pipeline

    u32 pipeline[2]; // store the addresses of the 2 instructions. first is the current executing instruction and the second is the instruction being decoded

    u32 get_reg(u32 reg);
    void set_reg(u32 reg, u32 value);

    void execute_instruction();

    void firmware_boot();
    
    void direct_boot();

    void flush_pipeline();

    // some helper functions
    bool is_arm();
    bool get_condition_flag(int condition_flag);

    bool evaluate_condition();

    // arm instructions
    void b();

    
};