#pragma once
#include <emulator/common/types.h>
// #include <emulator/emulator.h>

class Emulator;

typedef struct {
    u32 r[16];
} regs_t;


typedef struct {
    u32 r8_fiq;
    u32 r9_fiq;
    u32 r10_fiq;
    u32 r11_fiq;
    u32 r12_fiq;
    u32 r13_fiq;
    u32 r14_fiq;
    u32 spsr_fiq;
} fiq_regs_t;

typedef struct {
    u32 r13_svc;
    u32 r14_svc;
    u32 spsr_svc;
} svc_regs_t;

typedef struct {
    u32 r13_abt;
    u32 r14_abt;
    u32 spsr_abt;
} abt_regs_t;

typedef struct {
    u32 r13_irq;
    u32 r14_irq;
    u32 spsr_irq;
} irq_regs_t;

typedef struct {
    u32 r13_und;
    u32 r14_und;
    u32 spsr_und;
} und_regs_t;

class ARM {
public:
    regs_t regs;
    fiq_regs_t fiq_regs;
    svc_regs_t svc_regs;
    abt_regs_t abt_regs;
    irq_regs_t irq_regs;
    und_regs_t und_regs;
    
    u32 opcode;

    u32 get_pc();
    void set_pc(u32 value);
private:
    

    
};

class ARM9 : public ARM {
public:
    ARM9(Emulator *emulator);

private:
    Emulator *emulator;
    
    void read_opcode();
};

class ARM7 : public ARM {
public:
    ARM7(Emulator *emulator);
private:
    Emulator *emulator;

    void read_opcode();
    

};

