#pragma once

#include <common/types.h>

class GBA_ARM {
public:
	void direct_boot();
private:
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
};