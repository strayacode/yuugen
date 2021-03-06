#include <core/arm/memory.h>
#include <core/core.h>


void Memory::UpdateARM9MemoryMap(u32 low_addr, u32 high_addr) {
    // for mapping itcm and dtcm:
    // itcm has higher priority than dtcm
    // do UpdateARM9MemoryMap for old itcm and dtcm range, this will make the pages now not in the range
    // as nullptr. then do another UpdateARM9MemoryMap to make sure the itcm and range is fully covered

    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;

        if (core->cp15.GetITCMReadEnabled() && (addr < core->cp15.GetITCMSize())) {
            arm9_read_page_table[index] = &core->cp15.itcm[addr & 0x7FFF];
        } else if (core->cp15.GetDTCMReadEnabled() && in_range(core->cp15.GetDTCMBase(), core->cp15.GetDTCMSize())) {
            arm9_read_page_table[index] = &core->cp15.dtcm[(addr - core->cp15.GetDTCMBase()) & 0x3FFF];
        } else {
            switch (addr >> 24) {
            case 0x02:
                arm9_read_page_table[index] = &main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (WRAMCNT) {
                case 0:
                    arm9_read_page_table[index] = &shared_wram[addr & 0x7FFF];
                    break;
                case 1:
                    arm9_read_page_table[index] = &shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 2:
                    arm9_read_page_table[index] = &shared_wram[addr & 0x3FFF];
                    break;
                case 3:
                    // just set to a nullptr to indicate we should return 0
                    arm9_read_page_table[index] = nullptr;
                    break;
                }
                break;
            case 0xFF:
                if ((addr & 0xFFFF0000) == 0xFFFF0000) {
                    arm9_read_page_table[index] = &arm9_bios[addr & 0x7FFF];
                } else {
                    arm9_read_page_table[index] = nullptr;
                }

                break;
            default:
                // set as a nullptr, which indicates that we should do a regular read / write
                arm9_read_page_table[index] = nullptr;
                break;
            }
        }
    }

    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;

        if (core->cp15.GetITCMWriteEnabled() && (addr < core->cp15.GetITCMSize())) {
            arm9_write_page_table[index] = &core->cp15.itcm[addr & 0x7FFF];
        } else if (core->cp15.GetDTCMWriteEnabled() && in_range(core->cp15.GetDTCMBase(), core->cp15.GetDTCMSize())) {
            arm9_write_page_table[index] = &core->cp15.dtcm[(addr - core->cp15.GetDTCMBase()) & 0x3FFF];
        } else {
            switch (addr >> 24) {
            case 0x02:
                arm9_write_page_table[index] = &main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (WRAMCNT) {
                case 0:
                    arm9_write_page_table[index] = &shared_wram[addr & 0x7FFF];
                    break;
                case 1:
                    arm9_write_page_table[index] = &shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 2:
                    arm9_write_page_table[index] = &shared_wram[addr & 0x3FFF];
                    break;
                case 3:
                    // just set to a nullptr to indicate we should return 0
                    arm9_write_page_table[index] = nullptr;
                    break;
                }
                break;
            default:
                // set as a nullptr, which indicates that we should do a regular read / write
                arm9_write_page_table[index] = nullptr;
                break;
            }
        }
    }
}

template auto Memory::ARM9FastRead(u32 addr) -> u8;
template auto Memory::ARM9FastRead(u32 addr) -> u16;
template auto Memory::ARM9FastRead(u32 addr) -> u32;
template <typename T>
auto Memory::ARM9FastRead(u32 addr) -> T {
    addr &= ~(sizeof(T) - 1);

    T return_value = 0;

    int index = addr >> 12;
    if (arm9_read_page_table[index]) {
        memcpy(&return_value, &arm9_read_page_table[index][addr & 0xFFF], sizeof(T));
    } else {
        // default back to regular read
        switch (addr >> 24) {
        case REGION_IO:
            if constexpr (std::is_same_v<T, u8>) {
                return ARM9ReadByteIO(addr);
            } else if constexpr (std::is_same_v<T, u16>) {
                return ARM9ReadHalfIO(addr);
            } else if constexpr (std::is_same_v<T, u32>) {
                return ARM9ReadWordIO(addr);
            }
            break;
        case REGION_PALETTE_RAM:
            if ((addr & 0x7FF) < 400) {
                // this is the first block which is assigned to engine a
                memcpy(&return_value, &core->gpu.engine_a.palette_ram[addr & 0x3FF], sizeof(T));
            } else {
                // write to engine b's palette ram
                memcpy(&return_value, &core->gpu.engine_b.palette_ram[addr & 0x3FF], sizeof(T));
            }
            break;
        case REGION_VRAM:
            return core->gpu.ReadVRAM<T>(addr);
        case REGION_OAM:
            if ((addr & 0x7FF) < 0x400) {
                // this is the first block of oam which is 1kb and is assigned to engine a
                return_value = core->gpu.engine_a.ReadOAM<T>(addr);
            } else {
                // write to engine b's palette ram
                return_value = core->gpu.engine_b.ReadOAM<T>(addr);
            }

            break;
        case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
            // check if the arm9 has access rights to the gba slot
            // if not return 0
            if (EXMEMCNT & (1 << 7)) {
                return 0;
            }
            // otherwise return openbus (0xFFFFFFFF)
            return 0xFF * sizeof(T);
        }
    }

    return return_value;
}

template void Memory::ARM9FastWrite(u32 addr, u8 data);
template void Memory::ARM9FastWrite(u32 addr, u16 data);
template void Memory::ARM9FastWrite(u32 addr, u32 data);
template <typename T>
void Memory::ARM9FastWrite(u32 addr, T data) {
    addr &= ~(sizeof(T) - 1);

    int index = addr >> 12;

    if (arm9_write_page_table[index]) {
        memcpy(&arm9_write_page_table[index][addr & 0xFFF], &data, sizeof(T));
    } else {
        switch (addr >> 24) {
        case REGION_IO:
            if constexpr (std::is_same_v<T, u8>) {
                return ARM9WriteByteIO(addr, data);
            } else if constexpr (std::is_same_v<T, u16>) {
                return ARM9WriteHalfIO(addr, data);
            } else if constexpr (std::is_same_v<T, u32>) {
                return ARM9WriteWordIO(addr, data);
            }
            break;
        case REGION_PALETTE_RAM:
            if ((addr & 0x7FF) < 0x400) {
                // this is the first block of oam which is 1kb and is assigned to engine a
                core->gpu.engine_a.WritePaletteRAM<T>(addr, data);
            } else {
                // write to engine b's palette ram
                core->gpu.engine_b.WritePaletteRAM<T>(addr, data);
            }

            break;
        case REGION_VRAM:
            core->gpu.WriteVRAM<T>(addr, data);
            break;
        case REGION_OAM:
            if (sizeof(T) == 1) {
                log_fatal("[ARM9] 8-bit oam write is undefined behaviour");
            }

            if ((addr & 0x7FF) < 0x400) {
                // this is the first block of oam which is 1kb and is assigned to engine a
                core->gpu.engine_a.WriteOAM<T>(addr, data);
            } else {
                // write to engine b's palette ram
                core->gpu.engine_b.WriteOAM<T>(addr, data);
            }

            break;
        case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
            // for now do nothing lol
            break;
        }
    }
}

template auto Memory::ARM9Read(u32 addr) -> u8;
template auto Memory::ARM9Read(u32 addr) -> u16;
template auto Memory::ARM9Read(u32 addr) -> u32;
template <typename T>
auto Memory::ARM9Read(u32 addr) -> T {
    addr &= ~(sizeof(T) - 1);

    T return_value = 0;

    if (core->cp15.GetITCMReadEnabled() && (addr < core->cp15.GetITCMSize())) {
        memcpy(&return_value, &core->cp15.itcm[addr & 0x7FFF], sizeof(T));
    } else if (core->cp15.GetDTCMReadEnabled() && in_range(core->cp15.GetDTCMBase(), core->cp15.GetDTCMSize())) {
        memcpy(&return_value, &core->cp15.dtcm[(addr - core->cp15.GetDTCMBase()) & 0x3FFF], sizeof(T));
    } else {
        switch (addr >> 24) {
        case REGION_MAIN_MEMORY:
            memcpy(&return_value, &main_memory[addr & 0x3FFFFF], sizeof(T));
            break;
        case REGION_SHARED_WRAM:
            switch (WRAMCNT) {
            case 0:
                memcpy(&return_value, &shared_wram[addr & 0x7FFF], sizeof(T));
                break;
            case 1:
                memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], sizeof(T));
                break;
            case 2:
                memcpy(&return_value, &shared_wram[addr & 0x3FFF], sizeof(T));
                break;
            case 3:
                return 0;
            }
            break;
        case REGION_IO:
            if constexpr (std::is_same_v<T, u8>) {
                return ARM9ReadByteIO(addr);
            } else if constexpr (std::is_same_v<T, u16>) {
                return ARM9ReadHalfIO(addr);
            } else if constexpr (std::is_same_v<T, u32>) {
                return ARM9ReadWordIO(addr);
            }
            break;
        case REGION_PALETTE_RAM:
            if ((addr & 0x7FF) < 400) {
                // this is the first block which is assigned to engine a
                memcpy(&return_value, &core->gpu.engine_a.palette_ram[addr & 0x3FF], sizeof(T));
            } else {
                // write to engine b's palette ram
                memcpy(&return_value, &core->gpu.engine_b.palette_ram[addr & 0x3FF], sizeof(T));
            }
            break;
        case REGION_VRAM:
            return core->gpu.ReadVRAM<T>(addr);
        case REGION_OAM:
            if ((addr & 0x7FF) < 0x400) {
                // this is the first block of oam which is 1kb and is assigned to engine a
                return_value = core->gpu.engine_a.ReadOAM<T>(addr);
            } else {
                // write to engine b's palette ram
                return_value = core->gpu.engine_b.ReadOAM<T>(addr);
            }

            break;
        case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
            // check if the arm9 has access rights to the gba slot
            // if not return 0
            if (EXMEMCNT & (1 << 7)) {
                return 0;
            }
            // otherwise return openbus (0xFFFFFFFF)
            return 0xFF * sizeof(T);
        case REGION_ARM9_BIOS:
            memcpy(&return_value, &arm9_bios[addr & 0x7FFF], sizeof(T));
            break;
        }
    }

    return return_value;
}

template void Memory::ARM9Write(u32 addr, u8 data);
template void Memory::ARM9Write(u32 addr, u16 data);
template void Memory::ARM9Write(u32 addr, u32 data);
template <typename T>
void Memory::ARM9Write(u32 addr, T data) {
    addr &= ~(sizeof(T) - 1);

    if (core->cp15.GetITCMWriteEnabled() && (addr < core->cp15.GetITCMSize())) {
        memcpy(&core->cp15.itcm[addr & 0x7FFF], &data, sizeof(T));
        return;
    } else if (core->cp15.GetDTCMWriteEnabled() && in_range(core->cp15.GetDTCMBase(), core->cp15.GetDTCMSize())) {
        memcpy(&core->cp15.dtcm[(addr - core->cp15.GetDTCMBase()) & 0x3FFF], &data, sizeof(T));
        return;
    } else {
        switch (addr >> 24) {
        case REGION_MAIN_MEMORY:
            memcpy(&main_memory[addr & 0x3FFFFF], &data, sizeof(T));
            break;
        case REGION_SHARED_WRAM:
            switch (WRAMCNT) {
            case 0:
                // 32kb allocated to arm9
                memcpy(&shared_wram[addr & 0x7FFF], &data, sizeof(T));
                break;
            case 1:
                // 2nd 16kb allocated to arm9
                memcpy(&shared_wram[(addr & 0x3FFF) + 0x4000], &data, sizeof(T));
                break;
            case 2:
                // 1st 16kb allocated to arm9
                memcpy(&shared_wram[addr & 0x3FFF], &data, sizeof(T));
                break;
            case 3:
                // 0kb allocated to arm9
                break;
            default:
                log_fatal("handle");
            }
            break;
        case REGION_IO:
            if constexpr (std::is_same_v<T, u8>) {
                return ARM9WriteByteIO(addr, data);
            } else if constexpr (std::is_same_v<T, u16>) {
                return ARM9WriteHalfIO(addr, data);
            } else if constexpr (std::is_same_v<T, u32>) {
                return ARM9WriteWordIO(addr, data);
            }
            break;
        case REGION_PALETTE_RAM:
            if ((addr & 0x7FF) < 0x400) {
                // this is the first block of oam which is 1kb and is assigned to engine a
                core->gpu.engine_a.WritePaletteRAM<T>(addr, data);
            } else {
                // write to engine b's palette ram
                core->gpu.engine_b.WritePaletteRAM<T>(addr, data);
            }

            break;
        case REGION_VRAM:
            core->gpu.WriteVRAM<T>(addr, data);
            break;
        case REGION_OAM:
            if (sizeof(T) == 1) {
                log_fatal("[ARM9] 8-bit oam write is undefined behaviour");
            }

            if ((addr & 0x7FF) < 0x400) {
                // this is the first block of oam which is 1kb and is assigned to engine a
                core->gpu.engine_a.WriteOAM<T>(addr, data);
            } else {
                // write to engine b's palette ram
                core->gpu.engine_b.WriteOAM<T>(addr, data);
            }

            break;
        case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
            // for now do nothing lol
            break;
        }
    }
}