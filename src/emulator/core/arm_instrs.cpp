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
		// log_warn("r14 is now: 0x%04x", get_reg(14));
	} 
	// offset is shifted left by 2 and sign extended to 32 bits
	u32 offset = (get_bit(23, opcode) ? 0xFF000000: 0) | ((opcode & 0xFFFFFF) * 4);
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
		u8 shift_amount = get_bit_range(8, 11, opcode) * 2;
		if (shift_amount != 0) {
			// perform a regular ror
			u32 result = (immediate >> shift_amount) | (immediate << (32 - shift_amount));
			// c flag is bit 31 of result as when op2 is rotated the last bit, bit 31 will correspond to the carry out
			set_condition_flag(C_FLAG, result >> 31);
			op2 = result;
		} else {
			op2 = immediate;
		}
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
		case 2:
			op2 = asr(rm, shift_amount);
			break;
		case 3:
			op2 = ror(rm, shift_amount);
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
	case 0b0001:
		// EOR: rd = op1 ^ op2
		set_reg(rd, _xor(op1, op2, s_bit));
		break;
	case 0b0010:
		// SUB: rd = op1 - op2
		set_reg(rd, sub(op1, op2, s_bit));
		break;
	case 0b0011:
		// RSB: rd = op2 - op1
		set_reg(rd, sub(op2, op1, s_bit));
		break;
	case 0b0100:
		// ADD: rd = op1 + op2
		set_reg(rd, add(op1, op2, s_bit));
		break;
	case 0b0101:
		// ADC: rd = op1 + op2 + carry
		set_reg(rd, sbc(op1, op2, s_bit));
	case 0b0110:
		// SBC: rd = op1 - op2 + carry - 1
		set_reg(rd, sbc(op1, op2, s_bit));
		break;
	case 0b0111:
		// RSC: rd = op2 - op1 + carry - 1
		set_reg(rd, sbc(op2, op1, s_bit));
		break;
	case 0b1000:
		// TST set condition codes on op1 & op2
		_and(op1, op2, s_bit);
		break;
	case 0b1001:
		// TEQ: set condition codes on op1 ^ op2
		_xor(op1, op2, s_bit);
		break;
	case 0b1010:
		// CMP: set condition codes on op1 - op2
		sub(op1, op2, s_bit);
		break;
	case 0b1011:
		// CMN: set condition codes on op1 + op2
		add(op1, op2, s_bit);
		break;
	case 0b1100:
		// ORR: rd = op1 | op2
		set_reg(rd, orr(op1, op2, s_bit));
		break;
	case 0b1101:
		// MOV: rd = op2
		set_reg(rd, mov(op2, s_bit));
		break;
	case 0b1110:
		// BIC: rd = op1 & ~op2
		set_reg(rd, bic(op1, op2, s_bit));
		break;
	case 0b1111:
		// MVN: rd = ~op2
		set_reg(rd, mov(~op2, s_bit));
		break;
	default:
		log_fatal("[ARM] instruction type %d not implemented yet in data processsing\n", instruction_type);
		break;
	}
	if (rd == 15) {
		if (s_bit) {
			
			// load the spsr according to the current mode into cpsr
			regs.cpsr = get_spsr();
		}
		flush_pipeline();
	} else {
		regs.r15 += 4;
	}
}

u32 ARM::sub(u32 op1, u32 op2, bool set_flags) {
	u32 result = op1 - op2;
	// log_debug("%d 0x%04x", op1, op2);
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

u32 ARM::orr(u32 op1, u32 op2, bool set_flags) {
	u32 result = op1 | op2;
	if (set_flags) {
		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 31);
	}
	return result;
}

u32 ARM::adc(u32 op1, u32 op2, bool set_flags) {
	u64 result = (u64)op1 + (u64)op2 + (u64)get_condition_flag(C_FLAG);
	u32 result32 = (u32)result;
	if (set_flags) {
		// set carry flag to bit 32 of result
		set_condition_flag(C_FLAG, result >> 32);

		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 31);

		set_condition_flag(V_FLAG, (~(op1 ^ op2) & (op2 ^ result32)) >> 31);
	}
	return result32;
}

u32 ARM::sbc(u32 op1, u32 op2, bool set_flags) {
	// u32 c_flag = get_condition_flag(C_FLAG);
	u32 c_flag_result = get_condition_flag(C_FLAG) - 1;
	u32 result = op1 - op2 + c_flag_result;
	if (set_flags) {
		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 31);
		// understanding: for signed overflow either both ops positive and result negative or both ops negative result positive
		set_condition_flag(V_FLAG, ((op1 ^ op2) & (op1 ^ result)) >> 31);
		set_condition_flag(C_FLAG, op1 >= op2 - c_flag_result);
	}	
	return result;
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
		u32 rm = opcode & 0xF;
		u8 shift_type = get_bit_range(5, 6, opcode);
		u8 shift_amount = get_bit_range(4, 11, opcode);
		switch (shift_type) {
		case 0:
			op2 = lsl(rm, shift_amount);
			break;
		case 1:
			op2 = lsr(rm, shift_amount);
			break;
		case 2:
			op2 = asr(rm, shift_amount);
			break;
		case 3:
			op2 = ror(rm, shift_amount);
			break;
		default:
			log_fatal("[ARM] in data processing shift type %d is not implemented yet\n", shift_type);
		}
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
				write_byte(result, (u8)get_reg(rd));
			} else {
				// word transfer
				write_word(result, get_reg(rd));
			}
		} else {
			// post indexing
			if (byte_word_bit) {
				// byte transfer
				write_byte(get_reg(rn), (u8)get_reg(rd));
			} else {
				// word transfer
				write_word(get_reg(rn), get_reg(rd));
			}
			// write back to base register
			set_reg(rn, result);
		}
	}

	// check if rd = r15 in load
	if (load_store_bit) {
		
		
	}

	if (rd == rn) {
		log_warn("handle ldr opcode: %08x", opcode);
		// flush_pipeline();
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
			if (rd == 15) {
				log_warn("handle strh: r15: 0x%08x", regs.r15);
				// flush_pipeline();
			}
		} else {
			// store into memory
			write_halfword(address, get_reg(rd));
		}
		break;
	default:
		log_warn("sh %d not implemented yet!\n", sh);
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

void ARM::arm_halfword_data_transfer_register() {
	u8 pre_post_bit = get_bit(24, opcode);
	u8 up_down_bit = get_bit(23, opcode);
	u8 write_back_bit = get_bit(21, opcode);
	u8 load_store_bit = get_bit(20, opcode);
	u8 rn = get_bit_range(16, 19, opcode);
	
	u8 rd = get_bit_range(12, 15, opcode);
	u8 sh = get_bit_range(5, 6, opcode);
	u16 offset = get_reg(opcode & 0xF);
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
			if (rd == 15) {
				log_warn("handle ldhr r15: 0x%08x", regs.r15);
				// flush_pipeline();
			}
		} else {
			// store into memory
			write_halfword(address, get_reg(rd));
		}
		break;
	default:
		log_fatal("sh %d not implemented yet! opcode: 0x%04x", sh, opcode);
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
	if (rn == 15) {
		log_warn("rn = r15 in branch exchange is undefined!");
	}
	if ((rn & 1) == 1) {
		log_fatal("oh no thumb");
	}
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
		// load from memory
		if (up_down_bit) {
			// add offset to base
			for (int i = 0; i < 16; i++) {
				if (get_bit(i, register_list)) {
					if (pre_post_bit) {
						// pre increment the address
						addr += 4;
						// read word from memory address and store in register
						set_reg(i, read_word(addr));
					} else {
						// read word from memory address and store in register
						set_reg(i, read_word(addr));
						// post increment the address
						addr += 4;
					}
				}
			}
		} else {
			// subtract offset from base
			for (int i = 15; i >= 0; i--) {
				if (get_bit(i, register_list)) {
					if (pre_post_bit) {
						// pre decrement the address
						addr -= 4;
						// read word from memory address and store in register
						set_reg(i, read_word(addr));
					} else {
						// read word from memory address and store in register
						set_reg(i, read_word(addr));
						// post decrement the address
						addr -= 4;
					}
				}
			}
		}
	} else {
		// store into memory
		if (up_down_bit) {
			// add offset to base
			for (int i = 0; i < 16; i++) {
				if (get_bit(i, register_list)) {
					if (pre_post_bit) {
						// pre increment the address
						addr += 4;
						// write register to address
						write_word(addr, get_reg(i));
					} else {
						// write register to address
						write_word(addr, get_reg(i));
						// post increment the address
						addr += 4;
					}
				}
			}
		} else {
			// subtract offset from base
			for (int i = 15; i >= 0; i--) {
				if (get_bit(i, register_list)) {
					if (pre_post_bit) {
						// pre decrement the address
						addr -= 4;
						// write register to address
						write_word(addr, get_reg(i));
					} else {
						// write register to address
						write_word(addr, get_reg(i));
						// post decrement the address
						addr -= 4;
					}
				}
			}
		}
	}

	if (writeback_bit) {
		// write the address back into rn
		set_reg(rn, addr);
	}

	regs.r15 += 4;
}

void ARM::arm_psr_transfer() {
	u32 identifier = get_bit_range(16, 21, opcode);
	if (identifier == 41) {
		// msr transfer register contents to psr
		u32 rm = get_reg(opcode & 0xF);
		bool destination_psr = get_bit(22, opcode);
		if (destination_psr) {
			// write to currently banked spsr
			set_spsr(rm);
		} else {
			// write to cpsr
			regs.cpsr = rm;
		}
	} else {
		log_fatal("identifier %d for arm psr transfer not implemented!", identifier);
	}
}

void ARM::arm_single_data_swap() {
	log_fatal("need to implement single data swap");
}

void ARM::arm_multiply() {
	u8 accumulate_bit = get_bit(21, opcode);
	u8 s_bit = get_bit(20, opcode);
	u8 rd = get_bit_range(16, 19, opcode);
	u8 rn = get_bit_range(12, 15, opcode);
	u8 rs = get_bit_range(8, 11, opcode);
	u8 rm = opcode & 0xF;
	u32 result;
	if (accumulate_bit) {
		// multiply and accumulate
		result = (get_reg(rm) * get_reg(rs)) + get_reg(rn);
	} else {
		// multiply 
		result = get_reg(rm) * get_reg(rs);
	}
	if (s_bit) {
		// set flags accordingly
		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 31);
		// carry idk lol
		// overflow not affected
	}
	set_reg(rd, result);
	// log_fatal("need to implement multiply");
}

void ARM::arm_multiply_long() {
	// TODO: add support for signed multiply long i think
	u8 u_bit = get_bit(22, opcode);
	u8 accumulate_bit = get_bit(21, opcode);
	u8 s_bit = get_bit(20, opcode);
	u8 rd_hi = get_bit_range(16, 19, opcode);
	u8 rd_lo = get_bit_range(12, 15, opcode);
	u8 rs = get_bit_range(8, 11, opcode);
	u8 rm = opcode & 0xF;
	u64 result;
	if (accumulate_bit) {
		u64 rd_hilo = ((u64)get_reg(rd_hi)) << 32 | (u64)get_reg(rd_lo);
		// multiply and accumulate
		result = (get_reg(rm) * get_reg(rs)) + rd_hilo;
	} else {
		// multiply only
		result = get_reg(rm) * get_reg(rs);
	}
	
	if (s_bit) {
		// set flags
		set_condition_flag(Z_FLAG, result == 0);
		set_condition_flag(N_FLAG, result >> 63);
		// c and v flags are meaningless lol
	}

	// lower 32 bits of the 64 bit result are written to rd lo
	set_reg(rd_lo, result & 0xFFFFFFFF);
	// upper 32 bits of the 64 bit result are written to rd hi
	set_reg(rd_hi, result >> 32);
	
	// log_fatal("need to implement multiply long");
}

void ARM::arm_undefined() {
	log_warn("[ARM] arm instruction 0x%08x is undefined!", opcode);
}

void ARM::arm_unimplemented() {
	log_fatal("[ARM] arm instruction 0x%08x is unimplenented!", opcode);
}



