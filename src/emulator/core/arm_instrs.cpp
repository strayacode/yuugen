#include <emulator/core/arm.h>
#include <emulator/common/arithmetic.h>
#include <emulator/emulator.h>
#include <stdio.h>
#include <stdlib.h>
#include <emulator/common/log.h>

void ARM::arm_branch() {
    // execute branch
	u8 link_bit = get_bit(24, opcode);
	if (link_bit) {
		// branch with link
		// write the old pc which contains the address of the instruction after branch with link to the banked r14
		set_reg(14, regs.r15 - 4);
	} 
	// offset is shifted left by 2 and sign extended to 32 bits
	u32 offset = (get_bit(23, opcode) ? 0xFF000000: 0) | ((opcode & 0xFFFFFF) << 2);
	regs.r15 += offset;
	flush_pipeline();
}

void ARM::arm_data_processing() {
	u8 i_bit = get_bit(25, opcode);
	u8 s_bit = get_bit(20, opcode);
	u8 instruction_type = get_bit_range(21, 24, opcode);
	// TODO: implement condition code changing
	u32 rn = get_bit_range(16, 19, opcode);
	u32 op1 = get_reg(rn);
	u8 rd = get_bit_range(12, 15, opcode);
	
	u32 op2;
	if (i_bit) {
		// shift the unsigned 8 bit immediate
		u8 immediate = opcode & 0xFF;
		u8 shift = get_bit_range(8, 11, opcode);
		op2 = rotate_right(immediate, shift * 2);
	} else {
		u8 shift_type = get_bit_range(5, 6, opcode);
		u8 shift_amount;
		u32 rm = get_reg(opcode & 0xF);
		// decoding depends on bit 4
		if ((opcode & 0x10) == 0) {
			// bits 7..11 specify the shift amount
			shift_amount = get_bit_range(7, 11, opcode);
		} else {
			// bits 8..11 specify rs and the bottom byte is used for the shift amount
			shift_amount = get_reg(get_bit_range(8, 11, opcode)) & 0xFF;
		}
		switch (shift_type) {
		case 0:
			op2 = lsl(rm, shift_amount);
			break;
		case 1:
			op2 = lsr(rm, shift_amount);
			break;
		default:
			log_fatal("[ARM] in data processing shift type %d is not implemented yet\n", shift_type);
		}
		if (rn == regs.r15) {
			op1 += 4;
		}
		if (rm == regs.r15) {
			op2 += 4;
		}
	}
	switch (instruction_type) {
	case 0b0000:
		// AND: rd = op1 & op2
		set_reg(rd, _and(op1, op2, s_bit));
		break;
	case 0b0010:
		// SUB: rd = op1 - op2
		set_reg(rd, sub(op1, op2, s_bit));
		break;
	case 0b0100:
		// ADD: rd = op1 + op2
		set_reg(rd, add(op1, op2, s_bit));
		break;
	case 0b1001:
		// TEQ: set condition codes on op1 ^ op2
		_xor(op1, op2, s_bit);
		break;
	case 0b1010:
		// CMP: set condition codes on op1 - op2
		sub(op1, op2, s_bit);
		break;
	case 0b1101:
		// MOV: rd = op2
		set_reg(rd, mov(op2, s_bit));
		break;
	case 0b1110:
		// BIC: rd = op1 & ~op2
		set_reg(rd, bic(op1, op2, s_bit));
		break;
	default:
		log_fatal("[ARM] instruction type %d not implemented yet in data processsing\n", instruction_type);
		break;
	}
	if (rd == 15) {
		if (s_bit) {
			// load the spsr according to the current mode into cpsr
			regs.cpsr = get_spsr();
			flush_pipeline();
		}
	} else {
		regs.r15 += 4;
	}
	
}

u32 ARM::sub(u32 op1, u32 op2, bool set_flags) {
	u32 result = op1 - op2;
	if (set_flags) {
		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 31);
		// understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
		set_condition_flag(V_FLAG, ((op1 ^ op2) & (op1 ^ result)) >> 31);
		set_condition_flag(C_FLAG, op1 >= op2);
	}	
	return result;
}

u32 ARM::add(u32 op1, u32 op2, bool set_flags) {
	u64 result = (u64)op1 + (u64)op2;
	u32 result32 = (u32)result;
	if (set_flags) {
		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 31);
		// understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
		set_condition_flag(V_FLAG, (~(op1 ^ op2) & (op2 ^ result32)) >> 31);
	}	
	return result32;
}

u32 ARM::mov(u32 op2, bool set_flags) {
	// op1 is ignored in mov
	if (set_flags) {
		set_condition_flag(Z_FLAG, op2 == 0);
		set_condition_flag(N_FLAG, op2 >> 31);
	}
	// sorta useless lol
	return op2;
}

u32 ARM::_xor(u32 op1, u32 op2, bool set_flags) {
	u32 result = op1 ^ op2;
	if (set_flags) {
		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 31);
	}
	return result;
}

u32 ARM::bic(u32 op1, u32 op2, bool set_flags) {
	u32 result = op1 & ~op2;
	if (set_flags) {
		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 31);
	}
	return result;
}

u32 ARM::_and(u32 op1, u32 op2, bool set_flags) {
	// #ifdef counter
	// printf("z flag: %d", get_condition_flag(Z_FLAG));
	u32 result = op1 & op2;
	if (set_flags) {
		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 31);
	}
	return result;
}

void ARM::arm_single_data_transfer() {
	u8 i_bit = get_bit(25, opcode);
	u8 pre_post_bit = get_bit(24, opcode);
	u8 up_down_bit = get_bit(23, opcode);
	u8 byte_word_bit = get_bit(22, opcode);
	u8 write_back_bit = get_bit(21, opcode);
	u8 load_store_bit = get_bit(20, opcode);
	u8 rn = get_bit_range(16, 19, opcode);
	u8 rd = get_bit_range(12, 15, opcode);
	u32 op2;
	if (i_bit) {
		printf("uhh single data transfer i=1 not implemented yet\n");
		exit(1);
	} else {
		op2 = opcode & 0xFFF;
	}
	u32 result;
	if (up_down_bit) {
		result = get_reg(rn) + op2;
	} else {
		result = get_reg(rn) - op2;
	}

	// write the result to rn
	if (write_back_bit) {
		set_reg(rn, result);
	}

	if (load_store_bit) {
		// pre indexing
		if (pre_post_bit) {
			if (byte_word_bit) {
				// byte transfer
				set_reg(rd, read_byte(result));
			} else {
				// word transfer
				set_reg(rd, read_word(result));
			}

		} else {
			// post indexing
			if (byte_word_bit) {
				// byte transfer
				set_reg(rd, read_byte(get_reg(rn)));
			} else {
				// word transfer
				set_reg(rd, read_word(get_reg(rn)));
			}
			set_reg(rn, result);
		}
	} else {
		if (pre_post_bit) {
			// pre indexing
			if (byte_word_bit) {
				// byte transfer
				write_byte(result, get_reg(rd) & 0xFF);
			} else {
				// word transfer
				write_word(result, get_reg(rd));
			}
		} else {
			// post indexing
			if (byte_word_bit) {
				// byte transfer
				write_byte(get_reg(rn), get_reg(rd) & 0xFF);
			} else {
				// word transfer
				write_word(get_reg(rn), get_reg(rd));
			}
			// write back to base register
			set_reg(rn, result);
		}
	}
	regs.r15 += 4;
}

void ARM::arm_halfword_data_transfer_immediate() {
	
	u8 pre_post_bit = get_bit(24, opcode);
	u8 up_down_bit = get_bit(23, opcode);
	u8 write_back_bit = get_bit(21, opcode);
	u8 load_store_bit = get_bit(20, opcode);
	u8 rn = get_bit_range(16, 19, opcode);
	
	u8 rd = get_bit_range(12, 15, opcode);
	u8 sh = get_bit_range(5, 6, opcode);
	u16 offset = (get_bit_range(8, 11, opcode) << 4 | get_bit_range(0, 3, opcode));
	u32 address = get_reg(rn);
	if (pre_post_bit) {
		address += up_down_bit ? offset : -offset;
	}
	switch (sh) {
	case 0b01:
		// reads or writes a halfword value
		if (load_store_bit) {
			// load from memory
			set_reg(rd, read_halfword(address)); 
		} else {
			// store into memory
			write_halfword(address, get_reg(rd));
		}
		break;
	default:
		log_fatal("sh %d not implemented yet!\n", sh);
	}
	
	// post indexing always writes back to rn
	if (!pre_post_bit) {
		set_reg(rn, get_reg(rn) + (up_down_bit ? offset : -offset));
	} else if (write_back_bit) {
		// however writeback can also occur for pre indexing
		set_reg(rn, address);
	}
	regs.r15 += 4;
}

void ARM::arm_branch_exchange() {
	// if ((opcode & 1) == 1) {
	// 	log_fatal("oh no thumb");
	// }
	u32 rn = get_reg(opcode & 0xF);
	// if ((rn & 1) == 1) {
	// 	log_fatal("oh no thumb");
	// }
	regs.r15 = rn;
	flush_pipeline();
}

void ARM::arm_block_data_transfer() {
	u8 rn = get_bit_range(16, 19, opcode);
	u8 pre_post_bit = get_bit(24, opcode);
	u8 up_down_bit = get_bit(23, opcode);
	u8 psr_force_user_bit = get_bit(22, opcode);
	u8 writeback_bit = get_bit(21, opcode);
	u8 load_store_bit = get_bit(20, opcode);
	u16 register_list = opcode & 0xFFFF;
	u32 addr = get_reg(rn);
	if (load_store_bit) {
		if (pre_post_bit) {
			// pre indexing
			// add or subtract 4 bytes before loading value from memory into register
			if (up_down_bit) {
				for (int i = 0; i < 16; i++) {
					if (get_bit(i, register_list)) {
						// pre increment the address
						addr += 4;
						// read word from memory address and store in register
						set_reg(i, read_word(addr));
					}
				}
			} else {
				for (int i = 15; i >= 0; i--) {
					if (get_bit(i, register_list)) {
						// pre decrement the address
						addr -= 4;
						// read word from memory address and store in register
						set_reg(i, read_word(addr));
					}
				}
			}
		} else {
			// post indexing
			if (up_down_bit) {
				for (int i = 0; i < 16; i++) {
					if (get_bit(i, register_list)) {
						// read word from memory address and store in register
						set_reg(i, read_word(addr));

						// pre increment the address
						addr += 4;
					}
				}
			} else {
				for (int i = 15; i >= 0; i--) {
					if (get_bit(i, register_list)) {
						// read word from memory address and store in register
						set_reg(i, read_word(addr));

						// post decrement the address
						addr -= 4;
					}
				}
			}
		}
	} else {
		if (pre_post_bit) {
			// pre indexing
			// automatically add or subtract 4 bytes from addr
			if (up_down_bit) {
				for (int i = 0; i < 16; i++) {
					if (get_bit(i, register_list)) {
						// pre increment the address
						addr += 4;
						// write register to address
						write_word(addr, get_reg(i));

					}
				}
			} else {
				for (int i = 15; i >= 0; i--) {
					if (get_bit(i, register_list)) {
						// pre decrement the address
						addr -= 4;
						// write register to address
						write_word(addr, get_reg(i));

					}
				}
			}
		} else {
			// post indexing
			if (up_down_bit) {
				for (int i = 0; i < 16; i++) {
					if (get_bit(i, register_list)) {
						// write register to address
						write_word(addr, get_reg(i));
						// increment address after
						addr += 4;
					}
				}
			} else {
				for (int i = 15; i >= 0; i--) {
					if (get_bit(i, register_list)) {
						// write register to address
						write_word(addr, get_reg(i));
						// post decrement
						addr -= 4;
					}
				}
			}
		}
		if (writeback_bit) {
			// write the address back into rn
			set_reg(rn, addr);
		}
		
	}
	regs.r15 += 4;
}

void ARM::arm_undefined() {
	log_fatal("[ARM] arm instruction 0x%08x is undefined!", opcode);
}



