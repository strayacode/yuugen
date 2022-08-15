#include "Common/Memory.h"
#include "Core/Core.h"
#include "Core/ARM/ARM7/Memory.h"

ARM7Memory::ARM7Memory(System& system) : MemoryBase(Arch::ARMv4), system(system) {
    bios = load_bios<0x4000>("../bios/bios7.bin");

    build_mmio();
}

void ARM7Memory::reset() {
    update_memory_map(0, 0xFFFFFFFF);
    memset(arm7_wram, 0, 0x10000);
}

void ARM7Memory::update_memory_map(u32 low_addr, u32 high_addr) {
    for (u64 addr = low_addr; addr < high_addr; addr += PAGE_SIZE) {
        // get the pagetable index
        int index = addr >> PAGE_BITS;
        switch (addr >> 24) {
        case 0x00:
            read_page_table[index] = &bios[addr & 0x3FFF];
            write_page_table[index] = nullptr;
            break;
        case 0x02:
            read_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
            write_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
            break;
        case 0x03:
            if (addr < 0x03800000) {
                switch (system.wramcnt) {
                case 0:
                    read_page_table[index] = &arm7_wram[addr & 0xFFFF];
                    write_page_table[index] = &arm7_wram[addr & 0xFFFF];
                    break;
                case 1:
                    read_page_table[index] = &system.shared_wram[addr & 0x3FFF];
                    write_page_table[index] = &system.shared_wram[addr & 0x3FFF];
                    break;
                case 2:
                    read_page_table[index] = &system.shared_wram[(addr & 0x3FFF) + 0x4000];
                    write_page_table[index] = &system.shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 3:
                    read_page_table[index] = &system.shared_wram[addr & 0x7FFF];
                    write_page_table[index] = &system.shared_wram[addr & 0x7FFF];
                    break;
                }
            } else {
                read_page_table[index] = &arm7_wram[addr & 0xFFFF];
                write_page_table[index] = &arm7_wram[addr & 0xFFFF];
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

u8 ARM7Memory::slow_read_byte(u32 addr) {
    return slow_read<u8>(addr);
}

u16 ARM7Memory::slow_read_half(u32 addr) {
    return slow_read<u16>(addr);
}

u32 ARM7Memory::slow_read_word(u32 addr) {
    return slow_read<u32>(addr);
}

void ARM7Memory::slow_write_byte(u32 addr, u8 data) {
    slow_write<u8>(addr, data);
}

void ARM7Memory::slow_write_half(u32 addr, u16 data) {
    slow_write<u16>(addr, data);
}

void ARM7Memory::slow_write_word(u32 addr, u32 data) {
    slow_write<u32>(addr, data);
}

template <typename T>
T ARM7Memory::slow_read(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        if (addr < 0x04000520) [[likely]] {
            return mmio.read<T>(addr);
        }

        if constexpr (sizeof(T) == 4) {
            if (addr == 0x04100000) {
                return system.ipc.read_ipcfiforecv(0);
            }
        }
        
        log_warn("[ARM7Memory] unhandled mmio read %08x", addr);

        return 0;
    case 0x06:
        return system.video_unit.vram.read_arm7<T>(addr);
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (!(system.EXMEMCNT & (1 << 7))) {
            return 0;
        }

        // otherwise return openbus
        return (~static_cast<T>(0));
    }

    log_fatal("[ARM7Memory] handle %lu-bit read %08x", sizeof(T) * 8, addr);
}

template <typename T>
void ARM7Memory::slow_write(u32 addr, T data) {
    switch (addr >> 24) {
    case 0x00:
        // ignore all bios writes
        return;
    case 0x04:
        mmio.write<T>(addr, data);
        return;
    case 0x06:
        system.video_unit.vram.write_arm7<T>(addr, data);
        return;
    case 0x08: case 0x09:
        return;
    }

    log_fatal("[ARM7Memory] handle %lu-bit write %08x = %08x", sizeof(T) * 8, addr, data);
}

void ARM7Memory::build_mmio() {
    mmio.register_mmio<u8>(
        0x04000241,
        mmio.direct_read<u8>(&system.wramcnt),
        mmio.invalid_write<u8>()
    );

    mmio.register_mmio<u8>(
        0x04000300,
        mmio.direct_read<u8>(&system.postflg9, 0x1),
        mmio.direct_write<u8>(&system.postflg9, 0x1)
    );

    system.video_unit.build_mmio(mmio, Arch::ARMv4);
    system.ipc.build_mmio(mmio, Arch::ARMv4);
    system.dma[0].build_mmio(mmio, Arch::ARMv4);
    system.spu.build_mmio(mmio);
    system.cpu_core[0].build_mmio(mmio);

    log_debug("[ARM7Memory] mmio handlers registered");
}