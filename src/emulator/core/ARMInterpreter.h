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

    enum register_banks {
        USR_BANK = 0,
        FIQ_BANK = 1,
        SVC_BANK = 2,
        ABT_BANK = 3,
        IRQ_BANK = 4,
        UND_BANK = 5,
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
        
        // banked register arrays applies to r8, r9, r10, r11, r12, r13, r14
        u32 r_banked[6][7] = {};
        u32 cpsr;

        u32 spsr_fiq;
        u32 spsr_svc;
        u32 spsr_abt;
        u32 spsr_irq;
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
    void update_mode(u8 new_mode);

    u8 get_bank(u8 mode);

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

    // helpers
    u32 imm_data_processing();
    u32 imms_data_processing();
    u32 imm_single_data_transfer();
    u32 reg_halfword_signed_data_transfer();
    u32 imm_halfword_signed_data_transfer();

    // load and store instructions
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
    void ldrsb_pre(u32 op2);

    // data processing
    void subs(u32 op2);
    void sub(u32 op2);
    void cmps(u32 op2);
    void bics(u32 op2);
    void bic(u32 op2);
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
    void swp();
    void swpb();
    void mlas();
    void muls();
    void umulls();
    void smulls();
    void umlals();
    void smlals();
    void msr_reg();
    void msr_imm();
    void mrs_cpsr();

    // branch instructions
    void b();
    void bl();
    void bx();

    // ldm/stm instructions
    void stmiaw(); // store multiple increment after with writeback
    void ldmiaw(); // load multiple increment after with writeback
    void ldmiauw(); // load multiple increment after with writeback and use user registers i think
    void stmdbw(); // store multiple decrement before with writeback
    void stmdaw(); // store multiple decrement after with writeback
    void ldmibw(); // load multiple increment before with writeback
    void ldmibuw(); // load multiple increment before with writeback and use user registers i think
    void ldmdbuw(); // load multiple decrement before with writeback and use user registers i think
    void ldmdbw(); // load multiple decrement before with writeback
    void ldmdaw(); // load multiple decrement after with writeback
    void ldmdauw(); // load multiple decrement after with writeback and use user registers i think
    void stmibw(); // store multiple increment before with writeback


    // ARM9 exclusive instructions
    void clz(); 

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

    u32 rris();
    u32 rri();

    // load/store shifts
    u32 rpll(); // use in load store to shift a register rm with lsl
    u32 rplr(); // use in load store to shift a register rm with lsr
    u32 rpar(); // use in load store to shift a register rm with asr
    u32 rprr(); // use in load store to shift a register rm with ror
};