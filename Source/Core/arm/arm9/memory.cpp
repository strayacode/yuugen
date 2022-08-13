#include <cassert>
#include "Common/Log.h"
#include "Common/Memory.h"
#include "Core/arm/arm9/memory.h"
#include "Core/Core.h"

ARM9Memory::ARM9Memory(System& system) : system(system) {
    bios = LoadBios<0x8000>("../bios/bios9.bin");

    build_mmio();
}

void ARM9Memory::Reset() {
    UpdateMemoryMap(0, 0xFFFFFFFF);
}

void ARM9Memory::UpdateMemoryMap(u32 low_addr, u32 high_addr) {
    // for mapping itcm and dtcm:
    // itcm has higher priority than dtcm
    // do UpdateARM9MemoryMap for old itcm and dtcm range, this will make the pages now not in the range
    // as nullptr. then do another UpdateARM9MemoryMap to make sure the itcm and range is fully covered
    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;

        if (system.cp15.GetITCMReadEnabled() && (addr < system.cp15.GetITCMSize())) {
            read_page_table[index] = &system.cp15.itcm[addr & 0x7FFF];
        } else if (system.cp15.GetDTCMReadEnabled() && Common::in_range(system.cp15.GetDTCMBase(), system.cp15.GetDTCMBase() + system.cp15.GetDTCMSize(), addr)) {
            read_page_table[index] = &system.cp15.dtcm[(addr - system.cp15.GetDTCMBase()) & 0x3FFF];
        } else {
            switch (addr >> 24) {
            case 0x02:
                read_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (system.wramcnt) {
                case 0:
                    read_page_table[index] = &system.shared_wram[addr & 0x7FFF];
                    break;
                case 1:
                    read_page_table[index] = &system.shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 2:
                    read_page_table[index] = &system.shared_wram[addr & 0x3FFF];
                    break;
                case 3:
                    // just set to a nullptr to indicate we should return 0
                    read_page_table[index] = nullptr;
                    break;
                }
                break;
            case 0xFF:
                if ((addr & 0xFFFF0000) == 0xFFFF0000) {
                    read_page_table[index] = &bios[addr & 0x7FFF];
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

        if (system.cp15.GetITCMWriteEnabled() && (addr < system.cp15.GetITCMSize())) {
            write_page_table[index] = &system.cp15.itcm[addr & 0x7FFF];
        } else if (system.cp15.GetDTCMWriteEnabled() && Common::in_range(system.cp15.GetDTCMBase(), system.cp15.GetDTCMBase() + system.cp15.GetDTCMSize(), addr)) {
            write_page_table[index] = &system.cp15.dtcm[(addr - system.cp15.GetDTCMBase()) & 0x3FFF];
        } else {
            switch (addr >> 24) {
            case 0x02:
                write_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (system.wramcnt) {
                case 0:
                    write_page_table[index] = &system.shared_wram[addr & 0x7FFF];
                    break;
                case 1:
                    write_page_table[index] = &system.shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 2:
                    write_page_table[index] = &system.shared_wram[addr & 0x3FFF];
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

u8 ARM9Memory::ReadByte(u32 addr) {
    switch (addr >> 24) {
    case 0x05:
        return Common::read<u8>(system.video_unit.get_palette_ram(), addr & 0x7FF);
    case 0x06:
        return system.video_unit.vram.read_vram<u8>(addr);
    case 0x07:
        return Common::read<u8>(system.video_unit.get_oam(), addr & 0x7FF);
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (system.EXMEMCNT & (1 << 7)) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFF;
    default:
        log_fatal("handle byte read from %08x", addr);
    }
}

u16 ARM9Memory::ReadHalf(u32 addr) {
    switch (addr >> 24) {
    case 0x03:
        switch (system.wramcnt) {
        case 0:
            return Common::read<u16>(&system.shared_wram[addr & 0x7FFF], 0);
        case 1:
            return Common::read<u16>(&system.shared_wram[(addr & 0x3FFF) + 0x4000], 0);
        case 2:
            return Common::read<u16>(&system.shared_wram[addr & 0x3FFF], 0);
        case 3:
            return 0;
        }
        break;
    case 0x05:
        return Common::read<u16>(system.video_unit.get_palette_ram(), addr & 0x7FF);
    case 0x06:
        return system.video_unit.vram.read_vram<u16>(addr);
    case 0x07:
        return Common::read<u16>(system.video_unit.get_oam(), addr & 0x7FF);
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (system.EXMEMCNT & (1 << 7)) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFFFF;
    default:
        log_fatal("handle half read from %08x", addr);
        break;
    }

    log_fatal("ARM9: handle half read %08x", addr);

    return 0;
}

u32 ARM9Memory::ReadWord(u32 addr) {
    switch (addr >> 24) {
    case 0x03:
        switch (system.wramcnt) {
        case 0:
            return Common::read<u32>(&system.shared_wram[addr & 0x7FFF], 0);
        case 1:
            return Common::read<u32>(&system.shared_wram[(addr & 0x3FFF) + 0x4000], 0);
        case 2:
            return Common::read<u32>(&system.shared_wram[addr & 0x3FFF], 0);
        case 3:
            return 0;
        }
        break;
    case 0x05:
        return Common::read<u32>(system.video_unit.get_palette_ram(), addr & 0x7FF);
    case 0x06:
        return system.video_unit.vram.read_vram<u32>(addr);
    case 0x07:
        return Common::read<u16>(system.video_unit.get_oam(), addr & 0x7FF);
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (system.EXMEMCNT & (1 << 7)) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFFFFFFFF;
    case 0x0A:
        return 0;
    }

    log_fatal("ARM9: handle word read %08x", addr);

    return 0;
}

void ARM9Memory::WriteByte(u32 addr, u8 data) {
    switch (addr >> 24) {
    case 0x04:
        mmio_write<u8>(addr, data);
        return;
    case 0x05:
        Common::write<u8>(system.video_unit.get_palette_ram(), addr & 0x7FF, data);
        return;
    case 0x06:
        system.video_unit.vram.write_vram<u8>(addr, data);
        return;
    }

    log_fatal("ARM9: handle byte write %08x = %02x", addr, data);
}

void ARM9Memory::WriteHalf(u32 addr, u16 data) {
    switch (addr >> 24) {
    case 0x05:
        Common::write<u16>(system.video_unit.get_palette_ram(), addr & 0x7FF, data);
        return;
    case 0x06:
        system.video_unit.vram.write_vram<u16>(addr, data);
        return;
    case 0x07:
        Common::write<u16>(system.video_unit.get_oam(), addr & 0x7FF, data);
        return;
    case 0x08: case 0x09:
        // for now do nothing lol
        return;
    }

    log_fatal("ARM9: handle half write %08x = %04x", addr, data);
}

void ARM9Memory::WriteWord(u32 addr, u32 data) {
    switch (addr >> 24) {
    case 0x05:
        Common::write<u32>(system.video_unit.get_palette_ram(), addr & 0x7FF, data);
        return;
    case 0x06:
        system.video_unit.vram.write_vram<u32>(addr, data);
        return;
    case 0x07:
        Common::write<u32>(system.video_unit.get_oam(), addr & 0x7FF, data);
        return;
    case 0x08: case 0x09:
        // for now do nothing lol
        return;
    }

    log_fatal("ARM9: handle word write %08x = %08x", addr, data);
}

template <typename T>
void ARM9Memory::register_mmio(u32 addr, ReadCallback<T> read_callback, WriteCallback<T> write_callback) {
    ReadHandler<T>& read_handler = get_read_handler<T>(addr);
    WriteHandler<T>& write_handler = get_write_handler<T>(addr);

    read_handler.callback = read_callback;
    read_handler.mapped = true;

    write_handler.callback = write_callback;
    write_handler.mapped = true;

    read8[0x247].callback = read_callback;
}

void ARM9Memory::build_mmio() {
    register_mmio<u8>(
        0x04000247,
        invalid_read<u8>(),
        complex_write<u8>([this](u32, u8 data) {
            system.write_wramcnt(data);
        })
    );

    register_mmio<u8>(
        0x04000300,
        direct_read<u8>(&system.postflg9, 0x3),
        direct_write<u8>(&system.postflg9, 0x3)
    );

    log_debug("[ARM9Memory] mmio handlers registered");
}