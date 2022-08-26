#include "Core/ARM/CPUBase.h"
#include "Core/ARM/ARM7/Memory.h"
#include "Core/ARM/ARM7/Coprocessor.h"
#include "Core/ARM/ARM9/Memory.h"
#include "Core/ARM/ARM9/Coprocessor.h"
#include "Core/ARM/Interpreter/Interpreter.h"

CPUBase::CPUBase(MemoryBase& memory, CoprocessorBase& coprocessor, Arch arch) : m_memory(memory), m_coprocessor(coprocessor), m_arch(arch) {}

void CPUBase::reset() {
    m_gpr.fill(0);

    for (int i = 0; i < 6; i++) {
        m_gpr_banked[i].fill(0);
        m_spsr_banked[i].data = 0;
    }

    m_cpsr.data = 0xD3;
    switch_mode(MODE_SVC);

    m_ime = 0;
    m_ie = 0;
    m_irf = 0;
    m_halted = false;

    m_timestamp = 0;
}

void CPUBase::build_mmio(MMIO& mmio) {
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

void CPUBase::direct_boot(u32 entrypoint) {
    m_gpr[12] = m_gpr[14] = m_gpr[15] = entrypoint;

    // armv4/armv5 specific
    if (m_arch == Arch::ARMv4) {
        m_gpr[13] = 0x0380FD80;
        m_gpr_banked[BANK_IRQ][5] = 0x0380FF80;
        m_gpr_banked[BANK_SVC][5] = 0x0380FFC0;
    } else if (m_arch == Arch::ARMv5) {
        m_gpr[13] = 0x03002F7C;
        m_gpr_banked[BANK_IRQ][5] = 0x03003F80;
        m_gpr_banked[BANK_SVC][5] = 0x03003FC0;
    }

    // enter system mode
    m_cpsr.data = 0xDF;
    switch_mode(MODE_SYS);

    arm_flush_pipeline();
}

void CPUBase::firmware_boot() {

}

void CPUBase::send_interrupt(InterruptType interrupt_type) {
    m_irf |= (1 << static_cast<int>(interrupt_type));

    if (m_ie & (1 << static_cast<int>(interrupt_type))) {
        // on the arm9 ime needs to be set, while for the arm7
        // it can be ignored
        if (m_ime || m_arch == Arch::ARMv4) {
            m_halted = false;
        }
    }
}

void CPUBase::halt() {
    m_halted = true;
}

bool CPUBase::is_halted() {
    return m_halted;
}

void CPUBase::switch_mode(u8 mode) {

}
