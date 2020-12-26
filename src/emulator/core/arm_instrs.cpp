#include <emulator/core/arm.h>
#include <emulator/common/arithmetic.h>
#include <emulator/emulator.h>
#include <stdio.h>
#include <stdlib.h>

void ARM::arm_branch() {
	// printf("branch\n");
    if (evaluate_condition()) {
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
}

void ARM::arm_data_processing() {
	if (evaluate_condition()) {
		u8 i_bit = get_bit(25, opcode);
		u8 instruction_type = get_bit_range(21, 24, opcode);
		// TODO: implement condition code changing
		u32 rn = get_bit_range(16, 19, opcode);
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
			op2 = get_reg(opcode & 0xF);
			// decoding depends on bit 4
			if ((opcode & 0x10) == 0) {
				// bits 7..11 specify the shift amount
				shift_amount = get_bit_range(7, 11, opcode);
			} else {
				// bits 8..11 specify rs and the bottom byte is used for the shift amount
				shift_amount = get_reg(get_bit_range(8, 11, opcode)) & 0xFF;
			}
			switch (shift_type) {
			default:
				printf("[ARM] in data processing shift type %d is not implemented yet\n", shift_type);
				exit(1);
			}
		}
		switch (instruction_type) {
		case 0b0010:
			// SUB: rd = op1 - op2
			set_reg(rd, get_reg(rn) - op2);
			break;
		case 0b0100:
			// ADD: rd = op1 + op2
			set_reg(rd, get_reg(rn) + op2);
			break;
		case 0b1101:
			// MOV: rd = op2
			set_reg(rd, op2);
			break;
		default:
			printf("[ARM] instruction type %d not implemented yet in data processsing\n", instruction_type);
			emulator->running = false;
			break;
		}
	}
	regs.r15 += 4;
}

void ARM::arm_single_data_transfer() {
	// printf("single data transfer\n");
	if (evaluate_condition()) {
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
		if (load_store_bit) {
			printf("load from memory not implemented yet\n");
			exit(1);
			
		} else {
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
	}
	regs.r15 += 4;
}

void ARM::arm_halfword_data_transfer_immediate() {
	// printf("strh or ldrh\n");
	if (evaluate_condition()) {
		u8 pre_post_bit = get_bit(24, opcode);
		u8 up_down_bit = get_bit(23, opcode);
		u8 write_back_bit = get_bit(21, opcode);
		u8 load_store_bit = get_bit(20, opcode);
		u8 rn = get_bit_range(16, 19, opcode);
		u8 rd = get_bit_range(12, 15, opcode);
		u8 sh = get_bit_range(5, 6, opcode);
		u16 offset = (get_bit_range(8, 11, opcode) << 4 | get_bit_range(0, 3, opcode));
		switch (sh) {
		case 0b01:
			// unsigned halfword doesnt do anything
			break;
		default:
			printf("sh %d not implemented yet!\n", sh);
			break;
		}
		u32 result;
		if (up_down_bit) {
			result = get_reg(rn) + offset;
		} else {
			result = get_reg(rn) - offset;
		}
		if (write_back_bit) {
			set_reg(rn, result);
		}
		if (load_store_bit) {
			printf("load from memory not implemented yet!\n");
			exit(1);
		} else {
			if (pre_post_bit) {
				write_halfword(result, get_reg(rd));
			} else {
				write_halfword(get_reg(rn), get_reg(rd));
				// write back to base register
				set_reg(rn, result);
			}
		}
	}
	regs.r15 += 4;
}

void ARM::arm_undefined() {
    printf("[ARM] arm instruction 0x%08x is undefined!\n", opcode);
    emulator->running = false;
}

