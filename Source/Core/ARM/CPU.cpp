#include "Core/ARM/CPU.h"
#include "Core/ARM/ARM7/Memory.h"
#include "Core/ARM/ARM7/Coprocessor.h"
#include "Core/ARM/ARM9/Memory.h"
#include "Core/ARM/ARM9/Coprocessor.h"
#include "Core/ARM/Interpreter/Interpreter.h"

CPU::CPU(MemoryBase& memory, CoprocessorBase& coprocessor, Arch arch) : m_memory(memory), m_coprocessor(coprocessor), m_arch(arch) {}

void CPU::reset() {

}

void CPU::build_mmio(MMIO& mmio) {
    mmio.register_mmio<u8>(
        0x04000208,
        mmio.direct_read<u8>(&m_ime, 0x1),
        mmio.direct_write<u8>(&m_ime, 0x1)
    );

    mmio.register_mmio<u16>(
        0x04000208,
        mmio.direct_read<u16>(&m_ime, 0x1),
        mmio.direct_write<u16>(&m_ime, 0x1)
    );

    mmio.register_mmio<u32>(
        0x04000208,
        mmio.direct_read<u32>(&m_ime, 0x1),
        mmio.direct_write<u32>(&m_ime, 0x1)
    );

    mmio.register_mmio<u32>(
        0x04000210,
        mmio.direct_read<u32>(&m_ie),
        mmio.direct_write<u32>(&m_ie)
    );

    mmio.register_mmio<u32>(
        0x04000214,
        mmio.direct_read<u32>(&m_irf),
        mmio.complex_write<u32>([this](u32, u32 data) {
            m_irf &= ~data;
        })
    );
}

void CPU::direct_boot(u32 entrypoint) {

}

void CPU::firmware_boot() {

}

void CPU::run(u64 target) {
    while (m_timestamp < target) {
        if (m_halted) {
            m_timestamp = target;
            return;
        }

        m_timestamp += m_executor->run(target);
    }
}

void CPU::select_executor(ExecutorType executor_type) {
    switch (executor_type) {
    case ExecutorType::Interpreter:
        m_executor = std::make_unique<Interpreter>(*this);
        break;
    default:
        log_fatal("[CPU] handle unknown executor");
    }
}


void CPU::send_interrupt(InterruptType interrupt_type) {
    m_irf |= (1 << static_cast<int>(interrupt_type));

    if (m_ie & (1 << static_cast<int>(interrupt_type))) {
        // on the arm9 ime needs to be set, while for the arm7
        // it can be ignored
        if (m_ime || m_arch == Arch::ARMv4) {
            m_halted = false;
        }
    }
}

void CPU::halt() {
    m_halted = true;
}

bool CPU::is_halted() {
    return m_halted;
}
