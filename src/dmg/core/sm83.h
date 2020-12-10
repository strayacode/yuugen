#pragma once
#include <dmg/core/memory.h>
#include <dmg/common/types.h>

class SM83 {
public:
    DMGMemory memory;

    void step();
    SM83();
private:
    
    u8 opcode;
    int cycles;

    struct registers {
        u8 a;
        u8 b;
        u8 c;
        u8 d;
        u8 e;
        u8 f;
        u8 h;
        u8 l;

        u16 pc;
        u16 sp;
    } regs;

    void read_instruction();
    void execute_instruction();
    void tick();
    void debug_registers();

    // sm83 instructions
    void ld_sp_u16();
    u8 xor_byte(u8 r1, u8 r2);

    enum {
        Z_FLAG = 7,
        N_FLAG = 6,
        H_FLAG = 5,
        C_FLAG = 4,
    };

    // flag getters and setters
    bool get_flag(u8 index);

    void set_flag(u8 index, u8 value);

};