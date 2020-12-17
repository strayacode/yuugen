#include <emulator/common/types.h>
#include <emulator/common/arithmetic.h>
#include <stdio.h>

void disassemble_branch(u32 opcode) {
    printf("b\n");
}

void disassemble_data_processing_immediate(u32 opcode) {
    u8 instruction_type = get_bit_range(21, 24, opcode);
    switch (instruction_type) {
    case 0b1101:
        // mov
        u8 rd = get_bit_range(12, 15, opcode);
        u8 shift = get_bit_range(8, 11, opcode);
        u8 immediate = opcode & 0xFF;
        u32 op2 = rotate_right(immediate, shift * 2);
        printf("mov r%d, #%d\n", rd, op2);
        break;
    }
}

void disassemble_instruction(u32 opcode) {
    if (get_bit_range(25, 27, opcode) == 0b101) {
        disassemble_branch(opcode);
        return;
    } else if (get_bit_range(25, 27, opcode) == 0b001) {
        disassemble_data_processing_immediate(opcode);
        return;
    }
}