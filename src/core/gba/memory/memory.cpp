#include "core/gba/memory/memory.h"
#include "core/gba/gba.h"

GBAMemory::GBAMemory(GBA& gba) : gba(gba) {

}

void GBAMemory::Reset() {
    bios.fill(0);
    iwram.fill(0);
    ewram.fill(0);
    UpdateMemoryMap(0, 0xFFFFFFFF);
}

void GBAMemory::UpdateMemoryMap(u32 low_addr, u32 high_addr) {
    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;

        switch (addr >> 24) {
        case 0x00:
            if (addr < 0x4000) {
                read_page_table[index] = &bios[addr];
            } else {
                read_page_table[index] = nullptr;
            }

            write_page_table[index] = nullptr;
            break;
        case 0x02:
            read_page_table[index] = &ewram[addr & 0x3FFFF];
            write_page_table[index] = &ewram[addr & 0x3FFFF];
            break;
        case 0x03:
            read_page_table[index] = &iwram[addr & 0x7FFF];
            write_page_table[index] = &iwram[addr & 0x7FFF];
            break;
        case 0x08:
            read_page_table[index] = &gba.cartridge.rom[addr - 0x08000000];
            write_page_table[index] = nullptr;
            break;
        default:
            read_page_table[index] = nullptr;
            write_page_table[index] = nullptr;
        }
    }
}

u8 GBAMemory::ReadByte(u32 addr) {
    switch (addr >> 24) {
    default:
        log_fatal("[ARM7] Undefined 8-bit read %08x", addr);
    }
}

u16 GBAMemory::ReadHalf(u32 addr) {
    u16 return_value = 0;

    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        // case 0x04000004:
        //     return core.gpu.dispstat;
        // case 0x04000130:
        //     return core.input.keyinput;
        default:
            log_fatal("[ARM7] Undefined 16-bit io read %08x", addr);
        }
    default:
        log_fatal("[ARM7] Undefined 16-bit io read %08x", addr);
    }

    return return_value;
}

u32 GBAMemory::ReadWord(u32 addr) {
    u32 return_value = 0;

    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        // case 0x04000004:
        //     return (core.gpu.vcount << 16) | (core.gpu.dispstat);
        default:
            log_fatal("[ARM7] Undefined 32-bit io read %08x", addr);
        }
    default:
        log_fatal("handle %08x", addr);
    }

    return return_value;
}

void GBAMemory::WriteByte(u32 addr, u8 data) {
    switch (addr >> 24) {
    default:
        log_fatal("[ARM7] Undefined 8-bit write %08x = %08x", addr, data);
    }
}

void GBAMemory::WriteHalf(u32 addr, u16 data) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        // case 0x04000000:
        //     core.gpu.dispcnt = data;
        //     break;
        default:
            log_fatal("[ARM7] Undefined 16-bit io write %08x = %08x", addr, data);
        }
        break;
    default:
        log_fatal("[ARM7] Undefined 16-bit write %08x = %08x", addr, data);
    }
}

void GBAMemory::WriteWord(u32 addr, u32 data) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        // case 0x04000000:
        //     core.gpu.dispcnt = data & 0xFFFF;
        //     break;
        // case 0x04000208:
        //     core.cpu_core->ime = data & 0x1;
        //     break;
        default:
            log_fatal("[ARM7] Undefined 32-bit io write %08x = %08x", addr, data);
        }

        break;
    default:
        log_fatal("[ARM7] Undefined 32-bit write %08x = %08x", addr, data);
    }
}