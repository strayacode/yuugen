#pragma once
#include <emulator/common/types.h>
#include <stdio.h>

class Emulator;

class ARMInterpreter {
public:
    ARMInterpreter(Emulator *emulator, int cpu_id);

    void reset();
    void step();
    
private:
    Emulator *emulator;

    enum architecture_types {
        ARMv4,
        ARMv5,
    };

    enum condition_codes {
        // each number specifies the bit to access in cpsr
        N_FLAG = 31,
        Z_FLAG = 30,
        C_FLAG = 29,
        V_FLAG = 28,
    };

    enum cpu_modes {
        USR = 0x10,
        SYS = 0x1F,
        FIQ = 0x11,
        SVC = 0x13,
        ABT = 0x17,
        IRQ = 0x12,
        UND = 0x1B,
    };

    // 1 = arm9, 0 = arm7
    int cpu_id;

    struct registers {
        u32 r[16] = {};
        u32 cpsr;

        // fiq mode registers
        u32 r8_fiq;
        u32 r9_fiq;
        u32 r10_fiq;
        u32 r11_fiq;
        u32 r12_fiq;
        u32 r13_fiq;
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

    void direct_boot();
    void flush_pipeline();

    bool is_arm();
    bool get_condition_flag(int condition_flag);
    void set_condition_flag(int condition_flag, bool data);
    bool condition_evaluate();

    u32 get_spsr();
    void set_spsr(u32 data);

    u8 read_byte(u32 addr);
    u16 read_halfword(u32 addr);
    u32 read_word(u32 addr);

    void write_byte(u32 addr, u8 data);
    void write_halfword(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    u32 opcode; // used for storing the current executing instruction
    u32 pipeline[2]; // store the opcodes of the current executing instruction and the one to be decoded

    bool debug = false;

    void execute_instruction();

    void mov(u32 op2);
    void movs(u32 op2);
    u32 imm_data_processing();
    u32 imms_data_processing();
    u32 imm_single_data_transfer();
    u32 reg_halfword_signed_data_transfer();
    u32 imm_halfword_signed_data_transfer();
    void str_pre(u32 op2);
    void str_post(u32 op2);
    void ldr_pre(u32 op2);
    void ldr_post(u32 op2);
    void ldrh_post(u32 op2);
    void ldrh_pre(u32 op2);
    void strh_pre(u32 op2);
    void strh_post(u32 op2);
    void strb_post(u32 op2);
    void strb_pre(u32 op2);
    void ldrb_post(u32 op2);
    void ldrb_pre(u32 op2);
    void subs(u32 op2);
    void sub(u32 op2);
    void cmps(u32 op2);
    void bics(u32 op2);
    void add(u32 op2);
    void adds(u32 op2);
    void adcs(u32 op2);
    void adc(u32 op2);
    void mvn(u32 op2);
    void ands(u32 op2);
    void _and(u32 op2);
    void eor(u32 op2);
    void eors(u32 op2);
    void tsts(u32 op2);
    void orr(u32 op2);
    void orrs(u32 op2);
    void cmns(u32 op2);
    void rscs(u32 op2);
    void sbcs(u32 op2);
    void mlas();
    void muls();
    void umulls();
    void smulls();
    void msr_reg();
    void b();
    void bl();
    void bx();
    void stmiaw(); // store multiple increment after with writeback
    void ldmiaw(); // load multiple increment after with writeback
    void stmdbw(); // store multiple decrement before with writeback

    // shift stuff
    u32 lli(); // LSL with a 5 bit immediate shift amount
    u32 llis(); // LSL with a 5 bit immediate shift amount
    u32 llrs(); // with a shift amount specified by the bottom byte of the register specified by bits 8..11
    u32 lri(); // LSR with a 5 bit immediate shift amount
    u32 lris();
    u32 lrrs();
    u32 ari(); // ASR with a 5 bit immediate shift amount
    u32 aris(); // ASR with a 5 bit immediate shift amount
    u32 arrs();
    
};