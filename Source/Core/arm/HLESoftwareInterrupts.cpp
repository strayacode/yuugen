#include "Common/Log.h"
#include "Core/arm/cpu_core.h"

void CPUCore::generate_software_interrupt_table() {
    software_interrupt_table.fill(&CPUCore::unknown_software_interrupt);

    if (arch == CPUArch::ARMv5) {
        register_software_interrupt(0x03, &CPUCore::software_interrupt_delay);
        register_software_interrupt(0x05, &CPUCore::software_interrupt_vblank_interrupt_wait);
        register_software_interrupt(0x0B, &CPUCore::software_interrupt_copy);
        register_software_interrupt(0x0F, &CPUCore::software_interrupt_is_debugger);
    } else {
        register_software_interrupt(0x03, &CPUCore::software_interrupt_delay);
        register_software_interrupt(0x05, &CPUCore::software_interrupt_vblank_interrupt_wait);
        register_software_interrupt(0x0B, &CPUCore::software_interrupt_copy);
        register_software_interrupt(0x0F, &CPUCore::software_interrupt_is_debugger);
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
    int num_loops = regs.r[0];

    add_internal_cycles(num_loops * 4);
}

void CPUCore::software_interrupt_copy() {
    u32 source = regs.r[0];
    u32 destination = regs.r[1];

    // arm7 can't read or write from bios
    if (arch == CPUArch::ARMv4 && source < 0x4000 && destination < 0x4000) {
        return;
    }

    u32 count = regs.r[2] & 0x1FFFFF;
    bool fixed = (regs.r[2] >> 24) & 0x1;
    bool word_transfer = (regs.r[2] >> 26) & 0x1;

    if (word_transfer) {
        for (u32 i = 0; i < count; i += 4) {
            u32 addr = source;

            if (!fixed) {
                addr += i;
            }

            u32 data = ReadWord(addr);
            WriteWord(destination + i, data);
        }
    } else {
        for (u32 i = 0; i < count; i += 2) {
            u32 addr = source;

            if (!fixed) {
                addr += i;
            }

            u16 data = ReadHalf(addr);
            WriteHalf(destination + i, data);
        }
    }
}

void CPUCore::software_interrupt_is_debugger() {
    // on regular console report 0
    regs.r[0] = 0;
}

void CPUCore::software_interrupt_vblank_interrupt_wait() {
    regs.r[0] = 1;
    regs.r[1] = 1;
}

void CPUCore::software_interrupt_interrupt_wait() {
    // TODO: implement this
}