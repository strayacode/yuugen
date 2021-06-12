#include <core/arm/memory.h>
#include <core/core.h>

void Memory::UpdateARM7MemoryMap(u32 low_addr, u32 high_addr) {
    // TODO: fix shared memory mapping problem
    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;
        switch (addr >> 24) {
        case 0x00:
            arm7_read_page_table[index] = &arm7_bios[addr & 0x3FFF];
            arm7_write_page_table[index] = nullptr;
            break;
        case 0x02:
            arm7_read_page_table[index] = &main_memory[addr & 0x3FFFFF];
            arm7_write_page_table[index] = &main_memory[addr & 0x3FFFFF];
            break;
        case 0x03:
            if (addr < 0x03800000) {
                switch (WRAMCNT) {
                case 0:
                    arm7_read_page_table[index] = &arm7_wram[addr & 0xFFFF];
                    arm7_write_page_table[index] = &arm7_wram[addr & 0xFFFF];
                    break;
                case 1:
                    arm7_read_page_table[index] = &arm7_wram[addr & 0xFFFF];
                    arm7_write_page_table[index] = &arm7_wram[addr & 0xFFFF];
                    break;
                case 2:
                    arm7_read_page_table[index] = &shared_wram[(addr & 0x3FFF) + 0x4000];
                    arm7_write_page_table[index] = &shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 3:
                    arm7_read_page_table[index] = &shared_wram[addr & 0x7FFF];
                    arm7_write_page_table[index] = &shared_wram[addr & 0x7FFF];
                    break;
                }
            } else {
                arm7_read_page_table[index] = &arm7_wram[addr & 0xFFFF];
                arm7_write_page_table[index] = &arm7_wram[addr & 0xFFFF];
            }
            break;
        default:
            // set as a nullptr, which indicates that we should do a regular read / write
            arm7_read_page_table[index] = nullptr;
            arm7_write_page_table[index] = nullptr;
            break;
        }
    }
}

template auto Memory::ARM7FastRead(u32 addr) -> u8;
template auto Memory::ARM7FastRead(u32 addr) -> u16;
template auto Memory::ARM7FastRead(u32 addr) -> u32;
template <typename T>
auto Memory::ARM7FastRead(u32 addr) -> T {
    // TODO: this is slightly broken it seems fix later
    addr &= ~(sizeof(T) - 1);

    T return_value = 0;

    int index = addr >> 12;

    if (arm7_read_page_table[index]) {
        memcpy(&return_value, &arm7_read_page_table[index][addr & 0xFFF], sizeof(T));
    } else {
        switch (addr >> 24) {
        case REGION_SHARED_WRAM:
            if (addr < 0x03800000) {
                switch (WRAMCNT) {
                case 0:
                    // 0kb allocated to arm7
                    memcpy(&return_value, &arm7_wram[addr & 0xFFFF], sizeof(T));
                    break;
                case 1:
                    // first block of 16kb allocated to arm7
                    memcpy(&return_value, &shared_wram[addr & 0x3FFF], sizeof(T));
                    break;
                case 2:
                    // second block of 16kb allocated to arm7
                    memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], sizeof(T));
                    break;
                case 3:
                    // 32kb allocated to arm7
                    memcpy(&return_value, &shared_wram[addr & 0x7FFF], sizeof(T));
                    break;
                }
            } else {
                memcpy(&return_value, &arm7_wram[addr & 0xFFFF], sizeof(T));
            }
            break;
        case REGION_IO:
            if constexpr (std::is_same_v<T, u8>) {
                return ARM7ReadByteIO(addr);
            } else if constexpr (std::is_same_v<T, u16>) {
                return ARM7ReadHalfIO(addr);
            } else if constexpr (std::is_same_v<T, u32>) {
                return ARM7ReadWordIO(addr);
            }
            break;
        case REGION_VRAM:
            return_value = core->gpu.ReadARM7<T>(addr);
            break;
        case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
            // check if the arm9 has access rights to the gba slot
            // if not return 0
            if (!(EXMEMCNT & (1 << 7))) {
                return 0;
            }
            // otherwise return openbus (0xFFFFFFFF)
            return 0xFF * sizeof(T);
        default:
            log_fatal("[ARM7] undefined %ld-bit read %08x", sizeof(T) * 8, addr);
        }
    }

    return return_value;
}

template void Memory::ARM7FastWrite(u32 addr, u8 data);
template void Memory::ARM7FastWrite(u32 addr, u16 data);
template void Memory::ARM7FastWrite(u32 addr, u32 data);
template <typename T>
void Memory::ARM7FastWrite(u32 addr, T data) {
    addr &= ~(sizeof(T) - 1);

    int index = addr >> 12;

    if (arm7_write_page_table[index]) {
        memcpy(&arm7_read_page_table[index][addr & 0xFFF], &data, sizeof(T));
    } else {
        switch (addr >> 24) {
        case REGION_SHARED_WRAM:
            if (addr < 0x03800000) {
                switch (WRAMCNT) {
                case 0:
                    // 0kb allocated to arm7
                    memcpy(&arm7_wram[addr & 0xFFFF], &data, sizeof(T));
                    break;
                case 1:
                    // first block of 16kb allocated to arm7
                    memcpy(&shared_wram[addr & 0x3FFF], &data, sizeof(T));
                    break;
                case 2:
                    // second block of 16kb allocated to arm7
                    memcpy(&shared_wram[(addr & 0x3FFF) + 0x4000], &data, sizeof(T));
                    break;
                case 3:
                    // 32kb allocated to arm7
                    memcpy(&shared_wram[addr & 0x7FFF], &data, sizeof(T));
                    break;
                }
            } else {
                memcpy(&arm7_wram[addr & 0xFFFF], &data, sizeof(T));
            }

            break;
        case REGION_IO:
            if constexpr (std::is_same_v<T, u8>) {
                return ARM7WriteByteIO(addr, data);
            } else if constexpr (std::is_same_v<T, u16>) {
                return ARM7WriteHalfIO(addr, data);
            } else if constexpr (std::is_same_v<T, u32>) {
                return ARM7WriteWordIO(addr, data);
            }
            break;
        case REGION_VRAM:
            core->gpu.WriteARM7<T>(addr, data);
            break;
        case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
            // for now do nothing lol
            break;
        }
    }
}

template auto Memory::ARM7Read(u32 addr) -> u8;
template auto Memory::ARM7Read(u32 addr) -> u16;
template auto Memory::ARM7Read(u32 addr) -> u32;
template <typename T>
auto Memory::ARM7Read(u32 addr) -> T {
    addr &= ~(sizeof(T) - 1);

    T return_value = 0;

    switch (addr >> 24) {
    case REGION_ARM7_BIOS:
        memcpy(&return_value, &arm7_bios[addr & 0x3FFF], sizeof(T));
        break;
    case REGION_MAIN_MEMORY:
        memcpy(&return_value, &main_memory[addr & 0x3FFFFF], sizeof(T));
        break;
    case REGION_SHARED_WRAM:
        if (addr < 0x03800000) {
            switch (WRAMCNT) {
            case 0:
                // 0kb allocated to arm7
                memcpy(&return_value, &arm7_wram[addr & 0xFFFF], sizeof(T));
                break;
            case 1:
                // first block of 16kb allocated to arm7
                memcpy(&return_value, &shared_wram[addr & 0x3FFF], sizeof(T));
                break;
            case 2:
                // second block of 16kb allocated to arm7
                memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], sizeof(T));
                break;
            case 3:
                // 32kb allocated to arm7
                memcpy(&return_value, &shared_wram[addr & 0x7FFF], sizeof(T));
                break;
            }
        } else {
            memcpy(&return_value, &arm7_wram[addr & 0xFFFF], sizeof(T));
        }
        break;
    case REGION_IO:
        if constexpr (std::is_same_v<T, u8>) {
            return ARM7ReadByteIO(addr);
        } else if constexpr (std::is_same_v<T, u16>) {
            return ARM7ReadHalfIO(addr);
        } else if constexpr (std::is_same_v<T, u32>) {
            return ARM7ReadWordIO(addr);
        }
        break;
    case REGION_VRAM:
        return_value = core->gpu.ReadARM7<T>(addr);
        break;
    case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (!(EXMEMCNT & (1 << 7))) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFF * sizeof(T);
    default:
        log_fatal("[ARM7] undefined %ld-bit read %08x", sizeof(T) * 8, addr);
    }

    return return_value;
}

template void Memory::ARM7Write(u32 addr, u8 data);
template void Memory::ARM7Write(u32 addr, u16 data);
template void Memory::ARM7Write(u32 addr, u32 data);
template <typename T>
void Memory::ARM7Write(u32 addr, T data) {
    addr &= ~(sizeof(T) - 1);

    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        memcpy(&main_memory[addr & 0x3FFFFF], &data, sizeof(T));
        break;
    case REGION_SHARED_WRAM:
        if (addr < 0x03800000) {
            switch (WRAMCNT) {
            case 0:
                // 0kb allocated to arm7
                memcpy(&arm7_wram[addr & 0xFFFF], &data, sizeof(T));
                break;
            case 1:
                // first block of 16kb allocated to arm7
                memcpy(&shared_wram[addr & 0x3FFF], &data, sizeof(T));
                break;
            case 2:
                // second block of 16kb allocated to arm7
                memcpy(&shared_wram[(addr & 0x3FFF) + 0x4000], &data, sizeof(T));
                break;
            case 3:
                // 32kb allocated to arm7
                memcpy(&shared_wram[addr & 0x7FFF], &data, sizeof(T));
                break;
            }
        } else {
            memcpy(&arm7_wram[addr & 0xFFFF], &data, sizeof(T));
        }

        break;
    case REGION_IO:
        if constexpr (std::is_same_v<T, u8>) {
            return ARM7WriteByteIO(addr, data);
        } else if constexpr (std::is_same_v<T, u16>) {
            return ARM7WriteHalfIO(addr, data);
        } else if constexpr (std::is_same_v<T, u32>) {
            return ARM7WriteWordIO(addr, data);
        }
        break;
    case REGION_VRAM:
        core->gpu.WriteARM7<T>(addr, data);
        break;
    case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
        // for now do nothing lol
        break;
    }

    // since theres not really any other regions to handle we can ignore undefined writes :shrug:
}