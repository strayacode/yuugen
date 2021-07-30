#include <core/arm/arm7/memory.h>
#include <core/hw/hw.h>

ARM7Memory::ARM7Memory(HW* hw) : hw(hw) {

}

void ARM7Memory::Reset() {
    UpdateMemoryMap(0, 0xFFFFFFFF);
}

void ARM7Memory::UpdateMemoryMap(u32 low_addr, u32 high_addr) {
    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        int index = addr >> 12;
        switch (addr >> 24) {
        case 0x00:
            if (addr < 0x4000) {
                read_page_table[index] = &hw->arm7_bios[addr & 0x3FFF];
            } else {
                read_page_table[index] = nullptr;
            }

            write_page_table[index] = nullptr;
            
            break;
        case 0x02:
            read_page_table[index] = &hw->main_memory[addr & 0x3FFFFF];
            write_page_table[index] = &hw->main_memory[addr & 0x3FFFFF];
            break;
        case 0x03:
            if (addr < 0x03800000) {
                switch (hw->WRAMCNT) {
                case 0:
                    read_page_table[index] = &hw->arm7_wram[addr & 0xFFFF];
                    write_page_table[index] = &hw->arm7_wram[addr & 0xFFFF];
                    break;
                case 1:
                    read_page_table[index] = &hw->shared_wram[addr & 0x3FFF];
                    write_page_table[index] = &hw->shared_wram[addr & 0x3FFF];
                    break;
                case 2:
                    read_page_table[index] = &hw->shared_wram[(addr & 0x3FFF) + 0x4000];
                    write_page_table[index] = &hw->shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 3:
                    read_page_table[index] = &hw->shared_wram[addr & 0x7FFF];
                    write_page_table[index] = &hw->shared_wram[addr & 0x7FFF];
                    break;
                }
            } else {
                read_page_table[index] = &hw->arm7_wram[addr & 0xFFFF];
                write_page_table[index] = &hw->arm7_wram[addr & 0xFFFF];
            }

            break;
        default:
            // set as a nullptr, which indicates that we should do a regular read / write
            read_page_table[index] = nullptr;
            write_page_table[index] = nullptr;
            break;
        }
    }
}

auto ARM7Memory::ReadByte(u32 addr) -> u8 {
    log_fatal("handle");
}

auto ARM7Memory::ReadHalf(u32 addr) -> u16 {
    log_fatal("handle");
}

auto ARM7Memory::ReadWord(u32 addr) -> u32 {
    log_fatal("handle");
}

void ARM7Memory::WriteByte(u32 addr, u8 data) {
    log_fatal("handle");
}

void ARM7Memory::WriteHalf(u32 addr, u16 data) {
    log_fatal("handle");
}

void ARM7Memory::WriteWord(u32 addr, u32 data) {
    log_fatal("handle");
}