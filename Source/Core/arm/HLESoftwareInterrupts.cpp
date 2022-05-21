#include "Common/Log.h"
#include "Core/arm/cpu_core.h"

void CPUCore::generate_software_interrupt_table() {
    software_interrupt_table.fill(&CPUCore::unknown_software_interrupt);

    if (arch == CPUArch::ARMv5) {
        register_software_interrupt(0x03, &CPUCore::software_interrupt_delay);
    } else {
        register_software_interrupt(0x03, &CPUCore::software_interrupt_delay);
    }
}

void CPUCore::hle_software_interrupt() {
    u32 addr = regs.r[15];

    if (IsARM()) {
        addr -= 4;
    } else {
        addr -= 2;
    }

    software_interrupt_type = ReadByte(addr - 2);

    (this->*software_interrupt_table[software_interrupt_type])();
}

void CPUCore::unknown_software_interrupt() {
    log_fatal("CPUCore: handle hle software interrupt %02x at pc = %08x for arm%d", software_interrupt_type, regs.r[15], arch == CPUArch::ARMv5 ? 9 : 7);
}

void CPUCore::register_software_interrupt(int type, SoftwareInterrupt callback) {
    software_interrupt_table[type] = callback;
}

void CPUCore::software_interrupt_delay() {
    // each loop involves a sub (1 cycle) and branch (3 cycles) instruction 
    // finally we have a bx (3 cycles)
    int num_loops = regs.r[0];

    add_internal_cycles(num_loops * 4 + 3);
}