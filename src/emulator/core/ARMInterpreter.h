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

    void arm_mov(u32 op2);
    void arm_movs(u32 op2);

    // helpers
    u32 arm_imm_data_processing();
    u32 arm_imms_data_processing();
    u32 arm_imm_single_data_transfer();
    u32 arm_reg_halfword_signed_data_transfer();
    u32 arm_imm_halfword_signed_data_transfer();
    
    // load and store instructions
    void arm_str_pre(u32 op2);
    void arm_str_post(u32 op2);
    void arm_ldr_pre(u32 op2);
    void arm_ldr_post(u32 op2);
    void arm_ldrh_post(u32 op2);
    void arm_ldrh_pre(u32 op2);
    void arm_strh_pre(u32 op2);
    void arm_strh_post(u32 op2);
    void arm_strb_post(u32 op2);
    void arm_strb_pre(u32 op2);
    void arm_ldrb_post(u32 op2);
    void arm_ldrb_pre(u32 op2);
    void arm_ldrsb_pre(u32 op2);

    // data processing
    void arm_subs(u32 op2);
    void arm_sub(u32 op2);
    void arm_cmps(u32 op2);
    void arm_bics(u32 op2);
    void arm_bic(u32 op2);
    void arm_add(u32 op2);
    void arm_adds(u32 op2);
    void arm_adcs(u32 op2);
    void arm_adc(u32 op2);
    void arm_mvn(u32 op2);
    void arm_ands(u32 op2);
    void arm_and(u32 op2);
    void arm_eor(u32 op2);
    void arm_eors(u32 op2);
    void arm_tsts(u32 op2);
    void arm_orr(u32 op2);
    void arm_orrs(u32 op2);
    void arm_cmns(u32 op2);
    void arm_rscs(u32 op2);
    void arm_sbcs(u32 op2);
    void arm_swp();
    void arm_swpb();
    void arm_mlas();
    void arm_muls();
    void arm_umulls();
    void arm_smulls();
    void arm_umlals();
    void arm_smlals();
    void arm_msr_reg();
    void arm_msr_imm();
    void arm_mrs_cpsr();

    // branch instructions
    void arm_b();
    void arm_bl();
    void arm_bx();

    // ldm/stm instructions
    void arm_stmiaw(); // store multiple increment after with writeback
    void arm_ldmiaw(); // load multiple increment after with writeback
    void arm_ldmiauw(); // load multiple increment after with writeback and use user registers i think
    void arm_stmdbw(); // store multiple decrement before with writeback
    void arm_stmdaw(); // store multiple decrement after with writeback
    void arm_ldmibw(); // load multiple increment before with writeback
    void arm_ldmibuw(); // load multiple increment before with writeback and use user registers i think
    void arm_ldmdbuw(); // load multiple decrement before with writeback and use user registers i think
    void arm_ldmdbw(); // load multiple decrement before with writeback
    void arm_ldmdaw(); // load multiple decrement after with writeback
    void arm_ldmdauw(); // load multiple decrement after with writeback and use user registers i think
    void arm_stmibw(); // store multiple increment before with writeback


    // ARM9 exclusive instructions
    void arm_clz(); 

    // shift stuff
    u32 arm_lli(); // LSL with a 5 bit immediate shift amount
    u32 arm_llis(); // LSL with a 5 bit immediate shift amount
    u32 arm_llrs(); // with a shift amount specified by the bottom byte of the register specified by bits 8..11

    u32 arm_lri(); // LSR with a 5 bit immediate shift amount
    u32 arm_lris();
    u32 arm_lrrs();

    u32 arm_ari(); // ASR with a 5 bit immediate shift amount
    u32 arm_aris(); // ASR with a 5 bit immediate shift amount
    u32 arm_arrs();

    u32 arm_rris();
    u32 arm_rri();

    // load/store shifts
    u32 arm_rpll(); // use in load store to shift a register rm with lsl
    u32 arm_rplr(); // use in load store to shift a register rm with lsr
    u32 arm_rpar(); // use in load store to shift a register rm with asr
    u32 arm_rprr(); // use in load store to shift a register rm with ror
};