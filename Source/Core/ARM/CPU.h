#pragma once

#include <memory>
#include <array>
#include "Common/Types.h"
#include "Core/ARM/ExecutorInterface.h"
#include "Core/ARM/MemoryBase.h"
#include "Core/ARM/CoprocessorBase.h"
#include "Core/ARM/ARMTypes.h"
#include "Core/ARM/MMIO.h"

enum class ExecutorType {
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

class Interpreter;

// TODO: have the CPU class inherit the ExecutorInterface
class CPU {
public:
    CPU(MemoryBase& memory, CoprocessorBase&, Arch arch);

    void reset();
    void build_mmio(MMIO& mmio);
    void direct_boot(u32 entrypoint);
    void firmware_boot();
    void run(u64 target); 
    void select_executor(ExecutorType executor_type);
    
    void send_interrupt(InterruptType interrupt_type);
    void halt();
    bool is_halted();

private:
    friend class Interpreter;

    std::unique_ptr<ExecutorInterface> m_executor;
    MemoryBase& m_memory;
    CoprocessorBase& m_coprocessor;

    union StatusRegister {
        struct {
            u32 : 32;
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
