#pragma once

#include <common/types.h>
#include <common/arithmetic.h>
#include <common/log.h>
#include <stdio.h>

class NDS;

class ARM {
public:
	ARM(NDS *nds, int cpu_id);

	void firmware_boot();
	void direct_boot();
	void step();

	

private:
	NDS *nds;

	FILE *buffer;
	
	struct cpu_registers {
		// these are the 16 general purpose registers that are used in instructions
		u32 r[16];
		// 6 registers banks with registers r8-r14
		u32 r_banked[6][7] = {};

		// current program status register
		u32 cpsr;

		u32 spsr;

		u32 spsr_banked[6] = {};

	} regs;

	int counter = 0;

	// holds the opcode of the current instruction idk
	u32 opcode;
	

	// 0 = arm7, 1 = arm9
	int cpu_id;

	enum cpu_architectures {
		ARMv4 = 0,
		ARMv5 = 1,
	};

	enum cpu_modes {
		USR = 0x10,
		FIQ = 0x11,
		IRQ = 0x12,
		SVC = 0x13,
		ABT = 0x17,
		UND = 0x1B,
		SYS = 0x1F,
	};

	enum condition_codes {
        // each number specifies the bit to access in cpsr
        N_FLAG = 31,
        Z_FLAG = 30,
        C_FLAG = 29,
        V_FLAG = 28,
    };

	enum register_banks {
		BANK_USR = 0,
		BANK_FIQ = 1,
		BANK_IRQ = 2,
		BANK_SVC = 3,
		BANK_ABT = 4,
		BANK_UND = 5,
	};

	

	

	// holds 2 opcodes. the first is the currently executing and the second is the one being decoded
	u32 pipeline[2];


	u8 get_register_bank(u8 cpu_mode);

	void update_mode(u8 cpu_mode);

	void arm_flush_pipeline();
	void thumb_flush_pipeline();

	u8 read_byte(u32 addr);
	u16 read_halfword(u32 addr);
	u32 read_word(u32 addr);

	void write_byte(u32 addr, u8 data);
	void write_halfword(u32 addr, u16 data);
	void write_word(u32 addr, u32 data);



	
	
	void execute_instruction();

	bool get_condition_flag(int condition_flag);
	void set_condition_flag(int condition_flag, bool data);
	bool condition_evaluate();

	bool is_arm();


	// arm instructions

	// alu instructions

	// data processing
	void arm_cmp(u32 op2);
	void arm_mov(u32 op2);
	void arm_teq(u32 op2);
	void arm_add(u32 op2);
	void arm_adds(u32 op2);
	void arm_movs(u32 op2);
	void arm_bic(u32 op2);
	void arm_bics(u32 op2);
	void arm_tst(u32 op2);
	void arm_subs(u32 op2);
	void arm_sub(u32 op2);
	void arm_ands(u32 op2);
	void arm_and(u32 op2);
	void arm_eor(u32 op2);
	void arm_eors(u32 op2);
	void arm_adcs(u32 op2);
	void arm_adc(u32 op2);
	void arm_mvn(u32 op2);
	void arm_cmn(u32 op2);
	void arm_orr(u32 op2);
	void arm_orrs(u32 op2);
	void arm_rscs(u32 op2);
	void arm_sbcs(u32 op2);
	void arm_rsb(u32 op2);
	void arm_swp();
	void arm_swpb();


	// multiplication
	void arm_mlas();
	void arm_muls();
	void arm_umulls();
	void arm_smulls();
	void arm_umlals();
	void arm_smlals();

	// armv5 instructions
	void arm_clz();



	// alu helpers
	u32 arm_imm_data_processing();
	u32 arm_imms_data_processing();

	// alu shift stuff
	u32 arm_lli();
	u32 arm_llis();
	u32 arm_lri();
	u32 arm_lris();
	u32 arm_aris();
	u32 arm_ari();
	u32 arm_llrs();
	u32 arm_arrs();
	u32 arm_rri();
	u32 arm_rris();

	// branch instructions
	void arm_b();
	void arm_bl();
	void arm_bx();
	void arm_blx_offset();
	void arm_blx_reg();
	void arm_swi();

	// transfer instructions
	void arm_ldrb_pre(u32 op2);
	void arm_ldrb_post(u32 op2);
	void arm_strb_pre(u32 op2);
	void arm_strb_post(u32 op2);
	void arm_str_pre(u32 op2);
	void arm_str_post(u32 op2);
	void arm_ldr_pre(u32 op2);
	void arm_ldr_post(u32 op2);
	void arm_msr_reg();
	void arm_msr_imm();
	void arm_mrs_cpsr();
	void arm_mrs_spsr();
	void arm_strh_pre(u32 op2);
	void arm_strh_post(u32 op2);
	void arm_ldrh_pre(u32 op2);
	void arm_ldrsb_pre(u32 op2);
	void arm_mcr_reg();
	void arm_mrc_reg();
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
    void arm_ldmia();

    // armv5 instructions
    void arm_ldrd_post(u32 op2);
    // void arm_ldrd_pre()

    

	// transfer helpers
	u32 arm_imm_single_data_transfer();
	u32 arm_imm_halfword_signed_data_transfer();
	u32 arm_reg_halfword_signed_data_transfer();

	// load/store shifts
    u32 arm_rpll(); // use in load store to shift a register rm with lsl
    u32 arm_rplr(); // use in load store to shift a register rm with lsr
    u32 arm_rpar(); // use in load store to shift a register rm with asr
    u32 arm_rprr(); // use in load store to shift a register rm with ror


	// thumb instructions

	// alu instructions
	void thumb_sub_imm();
	void thumb_mov_imm();
	void thumb_add_imm();
	void thumb_lsl_imm();
	void thumb_lsr_imm();
	void thumb_cmp_reg();
	void thumb_mvn_reg();
	void thumb_movh();
	void thumb_addsp_imm();
	void thumb_addpc_reg();
	void thumb_cmp_imm();
	void thumb_orr_reg();
	void thumb_add_imm3();
	void thumb_add_reg();
	void thumb_lsl_reg();
	void thumb_addsp_reg();
	void thumb_and_reg();
	void thumb_eor_reg();
	void thumb_lsr_reg();
	void thumb_asr_imm();
	void thumb_sub_reg();
	void thumb_addh();
	void thumb_mul_reg();
	void thumb_neg_reg();
	void thumb_ror_reg();
	void thumb_tst_reg();
	void thumb_sub_imm3();

	// branch instructions
	void thumb_bgt();
	void thumb_blt();
	void thumb_blx_reg();
	void thumb_bx_reg();
	void thumb_bl_setup();
	void thumb_bl_offset();
	void thumb_bne();
	void thumb_blx_offset();
	void thumb_b();
	void thumb_bcc();
	void thumb_beq();
	void thumb_bhi();
	void thumb_bpl();
	void thumb_bcs();
	void thumb_bmi();

	// transfer instructions
	void thumb_ldrpc_imm();
	void thumb_str_reg();
	void thumb_push_lr();
	void thumb_strh_imm5();
	void thumb_ldrh_imm5();
	void thumb_str_imm5();
	void thumb_strb_imm5();
	void thumb_strsp_reg();
	void thumb_pop();
	void thumb_pop_pc();
	void thumb_ldrsp_reg();
	void thumb_ldrb_imm5();
	void thumb_push();
	void thumb_ldrh_reg();
	void thumb_ldr_imm5();
	void thumb_stmia_reg();
	void thumb_ldmia_reg();
	void thumb_ldr_reg();
};