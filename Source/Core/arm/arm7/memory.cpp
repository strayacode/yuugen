#include "Common/Memory.h"
#include "Core/Core.h"
#include "Core/arm/arm7/memory.h"

ARM7Memory::ARM7Memory(System& system) : MemoryBase(Arch::ARMv4), system(system) {
    bios = LoadBios<0x4000>("../bios/bios7.bin");

    build_mmio();
}

void ARM7Memory::Reset() {
    UpdateMemoryMap(0, 0xFFFFFFFF);
    memset(arm7_wram, 0, 0x10000);
}

void ARM7Memory::UpdateMemoryMap(u32 low_addr, u32 high_addr) {
    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;
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

u8 ARM7Memory::ReadByte(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio.read<u8>(addr);
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (!(system.EXMEMCNT & (1 << 7))) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFF;
    }

    log_fatal("ARM7: handle byte read %08x", addr);

    return 0;
}

u16 ARM7Memory::ReadHalf(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio.read<u16>(addr);
    case 0x06:
        return system.video_unit.vram.read_arm7<u16>(addr);
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (!(system.EXMEMCNT & (1 << 7))) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFFFF;
    }

    if (Common::in_range(0x04800000, 0x04900000, addr)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    log_fatal("ARM7: handle half read %08x", addr);

    return 0;
}

u32 ARM7Memory::ReadWord(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        return mmio.read<u32>(addr);
    case 0x06:
        return system.video_unit.vram.read_arm7<u32>(addr);
    }

    if (Common::in_range(0x04000400, 0x04000500, addr)) {
        return system.spu.read_word(addr);
    }

    log_fatal("ARM7: handle word read %08x", addr);

    return 0;
}

void ARM7Memory::WriteByte(u32 addr, u8 data) {
    switch (addr >> 24) {
    case 0x00:
        // ignore all bios writes
        return;
    case 0x04:
        mmio.write<u8>(addr, data);
        return;
    case 0x06:
        system.video_unit.vram.write_arm7<u8>(addr, data);
        return;
    }

    if (Common::in_range(0x04000400, 0x04000500, addr)) {
        system.spu.write_byte(addr, data);
        return;
    }

    log_fatal("ARM7: handle byte write %08x = %02x", addr, data);
}

void ARM7Memory::WriteHalf(u32 addr, u16 data) {
    switch (addr >> 24) {
    case 0x00:
        // ignore all bios writes
        break;
    case 0x04:
        mmio.write<u16>(addr, data);
        return;
    }

    log_fatal("ARM7: handle half write %08x = %04x", addr, data);
}

void ARM7Memory::WriteWord(u32 addr, u32 data) {
    switch (addr >> 24) {
    case 0x04:
        mmio.write<u32>(addr, data);
        return;
    case 0x06:
        system.video_unit.vram.write_arm7<u32>(addr, data);
        return;
    case 0x08: case 0x09:
        // for now do nothing lol
        return;
    }

    log_fatal("ARM7: handle word write %08x = %08x", addr, data);
}

void ARM7Memory::build_mmio() {
    mmio.register_mmio<u8>(
        0x04000300,
        mmio.direct_read<u8>(&system.postflg9, 0x1),
        mmio.direct_write<u8>(&system.postflg9, 0x1)
    );

    system.spu.build_mmio(mmio);
    system.ipc.build_mmio(mmio, Arch::ARMv4);
    system.cpu_core[0].build_mmio(mmio);

    log_debug("[ARM7Memory] mmio handlers registered");
}