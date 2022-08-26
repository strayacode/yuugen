#pragma once

#include <memory>
#include <array>
#include "Common/Types.h"
#include "Core/ARM/MemoryBase.h"
#include "Core/ARM/CoprocessorBase.h"
#include "Core/ARM/ARMTypes.h"
#include "Core/ARM/MMIO.h"

enum class CPUBackend {
    Interpreter,
};

enum class InterruptType {
    VBlank = 0,
    HBlank = 1,
    VCounter = 2,
    Timer0 = 3,
    Timer1 = 4,
    Timer2 = 5,
    Timer3 = 6,
    RTC = 7,
    DMA0 = 8,
    DMA1 = 9,
    DMA2 = 10,
    DMA3 = 11,
    Input = 12,
    IPCSync = 16,
    IPCSendEmpty = 17,
    IPCReceiveNonEmpty = 18,
    CartridgeTransfer = 19,
    GXFIFO = 21,
    SPI = 23,  
};

enum Mode {
    MODE_USR = 0x10,
    MODE_FIQ = 0x11,
    MODE_IRQ = 0x12,
    MODE_SVC = 0x13,
    MODE_ABT = 0x17,
    MODE_UND = 0x1B,
    MODE_SYS = 0x1F, // "privileged" user mode
};

enum Bank {
    BANK_USR = 0,
    BANK_FIQ = 1,
    BANK_IRQ = 2,
    BANK_SVC = 3,
    BANK_ABT = 4,
    BANK_UND = 5,
};

enum Condition {
    CONDITION_EQ = 0,
    CONDITION_NE = 1,
    CONDITION_CS = 2,
    CONDITION_CC = 3,
    CONDITION_MI = 4,
    CONDITION_PL = 5,
    CONDITION_VS = 6,
    CONDITION_VC = 7,
    CONDITION_HI = 8,
    CONDITION_LS = 9,
    CONDITION_GE = 10,
    CONDITION_LT = 11,
    CONDITION_GT = 12,
    CONDITION_LE = 13,
    CONDITION_AL = 14,
    CONDITION_NV = 15,
};

class CPUBase {
public:
    CPUBase(MemoryBase& memory, CoprocessorBase& coprocessor, Arch arch);

    virtual ~CPUBase() = default;

    // runs the backend until target
    virtual void run(u64 target) = 0;

    virtual void arm_flush_pipeline() = 0;
    virtual void thumb_flush_pipeline() = 0;

    void reset();
    void build_mmio(MMIO& mmio);
    void direct_boot(u32 entrypoint);
    void firmware_boot();
    
    void send_interrupt(InterruptType interrupt_type);
    void halt();
    bool is_halted();

private:
    void switch_mode(u8 mode);
    bool is_arm();

    bool is_privileged();
    bool has_spsr();
    u32 spsr();
    int bank(u8 mode);

    friend class Interpreter;

    MemoryBase& m_memory;
    CoprocessorBase& m_coprocessor;

    union StatusRegister {
        struct {
            u8 mode : 5;
            bool t : 1;
            bool f : 1;
            bool i : 1;
            u32 : 19;
            bool q : 1;
            bool v : 1;
            bool c : 1;
            bool z : 1;
            bool n : 1;
        };

        u32 data;
    };

    // general purpose registers
    std::array<u32, 16> m_gpr;

    // general purpose registers from r8-r14 for each mode
    std::array<std::array<u32, 7>, 6> m_gpr_banked;

    // current process status register
    StatusRegister m_cpsr;

    // saved process status registers for each mode
    std::array<StatusRegister, 6> m_spsr_banked;

    // interrupt state
    u32 m_irf;
    u32 m_ie;
    u32 m_ime;
    bool m_halted;

    u64 m_timestamp = 0;

    Arch m_arch;
};
