#include <core/arm/arm9/memory.h>
#include <core/hw/hw.h>

ARM9Memory::ARM9Memory(HW* hw) : hw(hw) {

}

void ARM9Memory::Reset() {
    UpdateMemoryMap(0, 0xFFFFFFFF);
}

void ARM9Memory::UpdateMemoryMap(u32 low_addr, u32 high_addr) {
    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;

        if (hw->cp15.GetITCMReadEnabled() && (addr < hw->cp15.GetITCMSize())) {
            read_page_table[index] = &hw->cp15.itcm[addr & 0x7FFF];
        } else if (hw->cp15.GetDTCMReadEnabled() && in_range(hw->cp15.GetDTCMBase(), hw->cp15.GetDTCMSize())) {
            read_page_table[index] = &hw->cp15.dtcm[(addr - hw->cp15.GetDTCMBase()) & 0x3FFF];
        } else {
            switch (addr >> 24) {
            case 0x02:
                read_page_table[index] = &hw->main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (hw->WRAMCNT) {
                case 0:
                    read_page_table[index] = &hw->shared_wram[addr & 0x7FFF];
                    break;
                case 1:
                    read_page_table[index] = &hw->shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 2:
                    read_page_table[index] = &hw->shared_wram[addr & 0x3FFF];
                    break;
                case 3:
                    // just set to a nullptr to indicate we should return 0
                    read_page_table[index] = nullptr;
                    break;
                }
                break;
            case 0xFF:
                if ((addr & 0xFFFF0000) == 0xFFFF0000) {
                    read_page_table[index] = &hw->arm9_bios[addr & 0x7FFF];
                } else {
                    read_page_table[index] = nullptr;
                }

                break;
            default:
                // set as a nullptr, which indicates that we should do a regular read / write
                read_page_table[index] = nullptr;
                break;
            }
        }
    }

    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;

        if (hw->cp15.GetITCMWriteEnabled() && (addr < hw->cp15.GetITCMSize())) {
            write_page_table[index] = &hw->cp15.itcm[addr & 0x7FFF];
        } else if (hw->cp15.GetDTCMWriteEnabled() && in_range(hw->cp15.GetDTCMBase(), hw->cp15.GetDTCMSize())) {
            write_page_table[index] = &hw->cp15.dtcm[(addr - hw->cp15.GetDTCMBase()) & 0x3FFF];
            
        } else {
            
            switch (addr >> 24) {
            case 0x02:
                write_page_table[index] = &hw->main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (hw->WRAMCNT) {
                case 0:
                    write_page_table[index] = &hw->shared_wram[addr & 0x7FFF];
                    break;
                case 1:
                    write_page_table[index] = &hw->shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 2:
                    write_page_table[index] = &hw->shared_wram[addr & 0x3FFF];
                    break;
                case 3:
                    // just set to a nullptr to indicate we should return 0
                    write_page_table[index] = nullptr;
                    break;
                }
                break;
            default:
                // set as a nullptr, which indicates that we should do a regular read / write
                write_page_table[index] = nullptr;
                break;
            }
        }
    }
}

auto ARM9Memory::ReadByte(u32 addr) -> u8 {
    log_fatal("handle");
}

auto ARM9Memory::ReadHalf(u32 addr) -> u16 {
    log_fatal("handle");
}

auto ARM9Memory::ReadWord(u32 addr) -> u32 {
    log_fatal("handle");
}

void ARM9Memory::WriteByte(u32 addr, u8 data) {
    log_fatal("handle write to %08x", addr);
}

void ARM9Memory::WriteHalf(u32 addr, u16 data) {
    log_fatal("handle");
}

void ARM9Memory::WriteWord(u32 addr, u32 data) {
    log_fatal("handle");
}