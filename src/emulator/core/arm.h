#pragma once
#include <emulator/common/types.h>
#include <stdio.h>

class Emulator;

class ARM {
public:
    ARM(Emulator *emulator, int cpu_id);

    void reset();
    void step();
    
private:
    Emulator *emulator;

    // 1 = arm9, 0 = arm7
    int cpu_id;

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

    // lut table
    using arm_function_pointer = void (ARM::*)();
    arm_function_pointer arm_lut_table[4096] = {};

    using thumb_function_pointer = void (ARM::*)();
    thumb_function_pointer thumb_lut_table[256] = {};

    void fill_arm_lut_table();
    void fill_thumb_lut_table();

    u8 read_byte(u32 addr);
    u16 read_halfword(u32 addr);
    u32 read_word(u32 addr);

    // implement these later
    void write_byte(u32 addr, u8 data);
    void write_halfword(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    u32 get_reg(u8 reg);
    void set_reg(u8 reg, u32 data);

    void execute_instruction();
    void flush_pipeline();

    void firmware_boot();
    void direct_boot();

    // some helper functions
    bool is_arm();
    bool get_condition_flag(int condition_flag);
    void set_condition_flag(int condition_flag, bool data);
    bool condition_evaluate();
    void debug_regs();


    // arm instruction handlers
    void arm_branch();
    void arm_undefined();
    void arm_data_processing();
    void arm_single_data_transfer(); // for ldr and str
    void arm_halfword_data_transfer_immediate();
    void arm_halfword_data_transfer_register();
    void arm_branch_exchange();
    void arm_block_data_transfer();

    // data processing helpers
    u32 sub(u32 op1, u32 op2, bool set_flags);
    u32 add(u32 op1, u32 op2, bool set_flags);
    u32 mov(u32 op2, bool set_flags);
    u32 _xor(u32 op1, u32 op2, bool set_flags);
    u32 bic(u32 op1, u32 op2, bool set_flags);
    u32 _and(u32 op1, u32 op2, bool set_flags);

    // shift stuff
    u32 lsl(u32 op2, u8 shift_amount); 
    u32 lsr(u32 op2, u8 shift_amount); 
   
    #ifdef FILE_LOG
    int counter = 0;
    FILE *file;
    #endif


};