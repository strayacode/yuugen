#pragma once
#include <emulator/common/types.h>

class Emulator;

class ARM7 {
public:
    ARM7(Emulator *emulator);

    void reset();
    void step();

private:
    Emulator *emulator;

    enum cpu_modes {
        USR = 0x10,
        SYS,
        FIQ,
        SVC,
        ABT,
        IRQ,
        UND,
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

    u32 opcode;

    u32 pipeline[2]; // store the addresses of the 2 instructions after the current opcode

    typedef void (*arm_instr_t)();
    arm_instr_t arm_instrs[4095];

    typedef void (*thumb_instr_t)();
    thumb_instr_t thumb_instrs[255];

    u32 get_reg(u32 reg);
    void set_reg(u32 reg, u32 value);

    

    void read_instruction();
    void execute_instruction();

    // TODO: work on firmware boot later
    void firmware_boot();
    
    // work on direct boot first
    void direct_boot();

    bool is_arm();
    
};