#include <core/arm/memory.h>
#include <core/core.h>

template auto Memory::ARM9Read(u32 addr) -> u8;
template auto Memory::ARM9Read(u32 addr) -> u16;
template auto Memory::ARM9Read(u32 addr) -> u32;
template <typename T>
auto Memory::ARM9Read(u32 addr) -> T {
    addr &= ~(sizeof(T) - 1);

    T return_value = 0;

    if (core->cp15.GetITCMEnabled() && (addr < core->cp15.GetITCMSize()) && !core->cp15.GetITCMLoadMode()) {
        memcpy(&return_value, &core->cp15.itcm[addr & 0x7FFF], sizeof(T));
    } else if (core->cp15.GetDTCMEnabled() && 
        in_range(core->cp15.GetDTCMBase(), core->cp15.GetDTCMSize()) && !core->cp15.GetDTCMLoadMode()) {
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
            if (sizeof(T) == 1) {
                log_fatal("[ARM9] 8-bit palette ram write is undefined behaviour");
            }

            if ((addr & 0x7FF) < 400) {
                // this is the first block which is assigned to engine a
                memcpy(&return_value, &core->gpu.engine_a.palette_ram[addr & 0x3FF], sizeof(T));
            } else {
                // write to engine b's palette ram
                memcpy(&return_value, &core->gpu.engine_b.palette_ram[addr & 0x3FF], sizeof(T));
            }

            break;
        case REGION_VRAM:
            // TODO: make vram memory handlers applicable to u8, u16 and u32
            if (addr >= 0x06800000) {
                return_value = core->gpu.ReadLCDC<T>(addr);
            } else if (in_range(0x06000000, 0x200000)) {
                return_value = core->gpu.ReadBGA<T>(addr);
            } else if (in_range(0x06200000, 0x200000)) {
                return_value = core->gpu.ReadBGB<T>(addr);
            } else if (in_range(0x06400000, 0x200000)) {
                return_value = core->gpu.ReadOBJA<T>(addr);
            } else if (in_range(0x06600000, 0x200000)) {
                return_value = core->gpu.ReadOBJB<T>(addr);
            } else {
                log_warn("[ARM9] Undefined %ld-bit vram read %08x", sizeof(T) * 8, addr);
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
        default:
            log_fatal("[ARM9] Undefined %ld-bit read %08x", sizeof(T) * 8, addr);
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
    
    if (core->cp15.GetITCMEnabled() && (addr < core->cp15.GetITCMSize())) {
        memcpy(&core->cp15.itcm[addr & 0x7FFF], &data, sizeof(T));
        return;
    } else if (core->cp15.GetDTCMEnabled() && 
        in_range(core->cp15.GetDTCMBase(), core->cp15.GetDTCMSize())) {

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
            if (sizeof(T) == 1) {
                log_fatal("[ARM9] 8-bit palette ram write is undefined behaviour");
            }

            for (long unsigned int i = 0; i < sizeof(T); i += 2) {
                if ((addr & 0x7FF) < 400) {
                    // this is the first block which is assigned to engine a
                    core->gpu.engine_a.WritePaletteRAM(addr + i, (data >> (i * 8)) & 0xFFFF);
                } else {
                    // write to engine b's palette ram
                    core->gpu.engine_b.WritePaletteRAM(addr + i, (data >> (i * 8)) & 0xFFFF);
                }
            }

            break;
        case REGION_VRAM:
            // if (sizeof(T) == 1) {
            //     log_fatal("[ARM9] 8-bit vram write is undefined behaviour");
            // }

            if (addr >= 0x06800000) {
                core->gpu.WriteLCDC<T>(addr, data);
            } else if (in_range(0x06000000, 0x200000)) {
                core->gpu.WriteBGA<T>(addr, data);
            } else if (in_range(0x06200000, 0x200000)) {
                core->gpu.WriteBGB<T>(addr, data);
            } else if (in_range(0x06400000, 0x200000)) {
                core->gpu.WriteOBJA<T>(addr, data);
            } else if (in_range(0x06600000, 0x200000)) {
                core->gpu.WriteOBJB<T>(addr, data);
            } else {
                log_warn("[ARM9] Undefined %ld-bit vram write %08x = %08x", sizeof(T) * 8, addr, data);
            }

            break;
        case REGION_OAM:
            if (sizeof(T) == 1) {
                log_fatal("[ARM9] 8-bit oam write is undefined behaviour");
            }

            for (long unsigned int i = 0; i < sizeof(T); i += 2) {
                // check memory address to see which engine to write to oam
                if ((addr & 0x3FF) < 0x400) {
                    // this is the first block of oam which is 1kb and is assigned to engine a
                    core->gpu.engine_a.WriteOAM(addr + i, (data >> (i * 8)) & 0xFFFF);
                } else {
                    // write to engine b's palette ram
                    core->gpu.engine_b.WriteOAM(addr + i, (data >> (i * 8)) & 0xFFFF);
                }
            }

            break;
        case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
            // for now do nothing lol
            break;
        default:
            log_fatal("[ARM9] Undefined %ld-bit write %08x = %08x", sizeof(T) * 8, addr, data);
        }
    }
}