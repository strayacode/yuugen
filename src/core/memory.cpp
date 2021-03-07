#include <core/memory.h>
#include <core/core.h>

Memory::Memory(Core* core) : core(core) {
    
}

void Memory::Reset() {
    memset(main_memory, 0, 0x400000);
    memset(arm7_wram, 0, 0x10000);
    memset(shared_wram, 0, 0x8000);
    memset(arm7_bios, 0, 0x4000);
    memset(arm9_bios, 0, 0x8000);

    LoadARM7BIOS();
    LoadARM9BIOS();

    WRAMCNT = 3;
}

u8 Memory::ARM7ReadByte(u32 addr) {
    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        return main_memory[addr & 0x3FFFFF];
        break;
    case REGION_SHARED_WRAM:
        if (addr < 0x03800000) {
            switch (WRAMCNT) {
            case 0:
                // 32kb allocated to arm9 and none to arm7 so in this case we write to arm7 wram
                return arm7_wram[addr & 0xFFFF];
                break;
            case 1:
                // in a 32kb block the first 16kb are allocated to the arm7 and the next 16kb are allocated to the arm9
                return shared_wram[addr & 0x3FFF];
                break;
            case 2:
                // in a 32kb block the first 16kb are allocated to the arm9 and the next 16kb are allocated to the arm7
                return shared_wram[(addr & 0x3FFF) + 0x4000];
                break;
            case 3:
                // 0 kb is allocated to arm9 and 32kb is allocated to arm7
                return shared_wram[addr & 0x7FFF];
                break;
            default:
                log_fatal("handle");
            }
            break;
        } else {
            // write to arm7 wram as addr >= 0x03800000
            return arm7_wram[addr & 0xFFFF];
            break;
        }
        break;
    default:
        log_fatal("unimplemented arm7 byte read at address 0x%08x\n", addr);
    }
}

u16 Memory::ARM7ReadHalfword(u32 addr) {
    addr &= ~1;

    u16 return_value = 0;

    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        memcpy(&return_value, &main_memory[addr & 0x3FFFFF], 2);
        break;
    case REGION_SHARED_WRAM:
        if (addr < 0x03800000) {
            switch (WRAMCNT) {
            case 0:
                // 32kb allocated to arm9 and none to arm7 so in this case we write to arm7 wram
                memcpy(&return_value, &arm7_wram[addr & 0xFFFF], 2);
                break;
            case 1:
                // in a 32kb block the first 16kb are allocated to the arm7 and the next 16kb are allocated to the arm9
                memcpy(&return_value, &shared_wram[addr & 0x3FFF], 2);
                break;
            case 2:
                // in a 32kb block the first 16kb are allocated to the arm9 and the next 16kb are allocated to the arm7
                memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], 2);
                break;
            case 3:
                // 0 kb is allocated to arm9 and 32kb is allocated to arm7
                memcpy(&return_value, &shared_wram[addr & 0x7FFF], 2);
                break;
            default:
                log_fatal("handle");
            }
            break;
        } else {
            // write to arm7 wram as addr >= 0x03800000
            memcpy(&return_value, &arm7_wram[addr & 0xFFFF], 2);
            break;
        }
        break;
    case REGION_IO:
        switch (addr) {
        case 0x04000004:
            return core->gpu.DISPSTAT7;
        case 0x04000130:
            return core->input.KEYINPUT;
        case 0x04000180:
            return core->ipc.ReadIPCSYNC7();
        case 0x04000184:
            return core->ipc.IPCFIFOCNT7;
        default:
            log_fatal("unimplemented arm7 halfword io read at address 0x%08x", addr);
        }
        break;
    default:
        log_fatal("unimplemented arm7 halfword read at address 0x%08x\n", addr);
    }

    return return_value;
}

u32 Memory::ARM7ReadWord(u32 addr) {
    addr &= ~3;

    u32 return_value = 0;

    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        memcpy(&return_value, &main_memory[addr & 0x3FFFFF], 4);
        break;
    case REGION_SHARED_WRAM:
        if (addr < 0x03800000) {
            switch (WRAMCNT) {
            case 0:
                // 32kb allocated to arm9 and none to arm7 so in this case we write to arm7 wram
                memcpy(&return_value, &arm7_wram[addr & 0xFFFF], 4);
                break;
            case 1:
                // in a 32kb block the first 16kb are allocated to the arm7 and the next 16kb are allocated to the arm9
                memcpy(&return_value, &shared_wram[addr & 0x3FFF], 4);
                break;
            case 2:
                // in a 32kb block the first 16kb are allocated to the arm9 and the next 16kb are allocated to the arm7
                memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], 4);
                break;
            case 3:
                // 0 kb is allocated to arm9 and 32kb is allocated to arm7
                memcpy(&return_value, &shared_wram[addr & 0x7FFF], 4);
                break;
            default:
                log_fatal("handle");
            }
            break;
        } else {
            // write to arm7 wram as addr >= 0x03800000
            memcpy(&return_value, &arm7_wram[addr & 0xFFFF], 4);
            break;
        }
        break;
    case REGION_IO:
        switch (addr) {
        case 0x04000180:
            return core->ipc.ReadIPCSYNC7();
        case 0x04000208:
            return core->interrupt[0].IME & 0x1;
        case 0x04000210:
            return core->interrupt[0].IE;
        case 0x04000214:
            return core->interrupt[0].IF;
        case 0x04100000:
            // just return the first item in the fifo xd
            return core->ipc.fifo7[0];
        default:
            log_fatal("unimplemented arm7 word io read at address 0x%08x", addr);
        }
        break;
    default:
        log_fatal("unimplemented arm7 word read at address 0x%08x\n", addr);
    }

    return return_value;
}

void Memory::ARM7WriteByte(u32 addr, u8 data) {
    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        main_memory[addr & 0x3FFFFF] = data;
        break;
    case REGION_SHARED_WRAM:
        if (addr < 0x03800000) {
            switch (WRAMCNT) {
            case 0:
                // 32kb allocated to arm9 and none to arm7 so in this case we write to arm7 wram
                arm7_wram[addr & 0xFFFF] = data;
                break;
            case 1:
                // in a 32kb block the first 16kb are allocated to the arm7 and the next 16kb are allocated to the arm9
                shared_wram[addr & 0x3FFF] = data;
                break;
            case 2:
                // in a 32kb block the first 16kb are allocated to the arm9 and the next 16kb are allocated to the arm7
                shared_wram[(addr & 0x3FFF) + 0x4000] = data;
                break;
            case 3:
                // 0 kb is allocated to arm9 and 32kb is allocated to arm7
                shared_wram[addr & 0x7FFF] = data;
                break;
            default:
                log_fatal("handle");
            }
            break;
        } else {
            // write to arm7 wram as addr >= 0x03800000
            arm7_wram[addr & 0xFFFF] = data;
            break;
        }
        break;
    default:
        log_fatal("unimplemented arm7 byte write at address 0x%08x with data 0x%02x\n", addr, data);
    }
}

void Memory::ARM7WriteHalfword(u32 addr, u16 data) {
    addr &= ~1;

    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        memcpy(&main_memory[addr & 0x3FFFFF], &data, 2);
        break;
    case REGION_IO:
        switch (addr) {
        case 0x04000184:
            core->ipc.WriteIPCFIFOCNT7(data);
            break;
        case 0x04000208:
            core->interrupt[0].IME = data & 0x1;
            break;
        default:
            log_fatal("unimplemented arm7 halfword io write at address 0x%08x with data 0x%04x", addr, data);
        }
        break;
    default:
        log_fatal("unimplemented arm7 halfword write at address 0x%08x with data 0x%04x\n", addr, data);
    }
}

void Memory::ARM7WriteWord(u32 addr, u32 data) {
    addr &= ~3;

    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        memcpy(&main_memory[addr & 0x3FFFFF], &data, 4);
        break;
    case REGION_SHARED_WRAM:
        if (addr < 0x03800000) {
            switch (WRAMCNT) {
            case 0:
                // 32kb allocated to arm9 and none to arm7 so in this case we write to arm7 wram
                memcpy(&arm7_wram[addr & 0xFFFF], &data, 4);
                break;
            case 1:
                // in a 32kb block the first 16kb are allocated to the arm7 and the next 16kb are allocated to the arm9
                memcpy(&shared_wram[addr & 0x3FFF], &data, 4);
                break;
            case 2:
                // in a 32kb block the first 16kb are allocated to the arm9 and the next 16kb are allocated to the arm7
                memcpy(&shared_wram[(addr & 0x3FFF) + 0x4000], &data, 4);
                break;
            case 3:
                // 0 kb is allocated to arm9 and 32kb is allocated to arm7
                memcpy(&shared_wram[addr & 0x7FFF], &data, 4);
                break;
            default:
                log_fatal("handle");
            }
            break;
        } else {
            // write to arm7 wram as addr >= 0x03800000
            memcpy(&arm7_wram[addr & 0xFFFF], &data, 4);
            break;
        }
        break;
    case REGION_IO:
        switch (addr) {
        case 0x04000180:
            core->ipc.WriteIPCSYNC7(data);
            break;
        case 0x04000208:
            core->interrupt[0].IME = data & 0x1;
            break;
        case 0x04000210:
            core->interrupt[0].IE = data;
            break;
        case 0x04000214:
            core->interrupt[0].IF = ~(data);
            break;
        default:
            log_fatal("unimplemented arm7 word io write at address 0x%08x with data 0x%08x", addr, data);
        }
        break;
    default:
        log_fatal("unimplemented arm7 word write at address 0x%08x with data 0x%08x\n", addr, data);
    }
}

u8 Memory::ARM9ReadByte(u32 addr) {
    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        return main_memory[addr & 0x3FFFFF];
    case REGION_IO:
        switch (addr) {
        case 0x04000208:
            return core->interrupt[1].IME & 0x1;
        case 0x04004000:
            return 0;
        default:
            log_fatal("unimplemented arm9 byte io read at address 0x%08x", addr);
        }
        break;
    default:
        log_fatal("unimplemented arm9 byte read at address 0x%08x\n", addr);
    }
}

u16 Memory::ARM9ReadHalfword(u32 addr) {
    addr &= ~1;

    u16 return_value = 0;

    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        memcpy(&return_value, &main_memory[addr & 0x3FFFFF], 2);
        break;
    case REGION_IO:
        switch (addr) {
        case 0x04000004:
            return core->gpu.DISPSTAT9;
        case 0x04000130:
            return core->input.KEYINPUT;
        case 0x04000180:
            return core->ipc.ReadIPCSYNC9();
        case 0x04000184:
            return core->ipc.IPCFIFOCNT9;
        default:
            log_fatal("unimplemented arm9 halfword io read at address 0x%08x", addr);
        }
        break;
    case REGION_ARM9_BIOS:
        memcpy(&return_value, &arm9_bios[addr & 0x7FFF], 2);
        break;
    default:
        log_fatal("unimplemented arm9 halfword read at address 0x%08x\n", addr);
    }

    return return_value;
}

u32 Memory::ARM9ReadWord(u32 addr) {
    addr &= ~3;

    u32 return_value = 0;
    // printf("itcm enabled %d dtcm enabled %d\n", core->cp15.GetITCMEnabled(), core->cp15.GetDTCMEnabled());
    // printf("itcm size: %08x\n", core->cp15.GetITCMSize());
    // printf("dtcm base: %08x\n", core->cp15.GetDTCMBase());
    if (core->cp15.GetITCMEnabled() && (addr < core->cp15.GetITCMSize())) {
        memcpy(&return_value, &core->cp15.itcm[addr & 0x7FFF], 4);
    } else if (core->cp15.GetDTCMEnabled() && 
        in_range(core->cp15.GetDTCMBase(), core->cp15.GetDTCMBase() + core->cp15.GetDTCMSize(), addr)) {
        memcpy(&return_value, &core->cp15.dtcm[(addr - core->cp15.GetDTCMBase()) & 0x3FFF], 4);
    } else {
        switch (addr >> 24) {
        case REGION_MAIN_MEMORY:
            memcpy(&return_value, &main_memory[addr & 0x3FFFFF], 4);
            break;
        case REGION_SHARED_WRAM:
            switch (WRAMCNT) {
            case 0:
                // 32 kb is allocated to arm9 and 0kb is allocated to arm7
                memcpy(&return_value, &shared_wram[addr & 0x7FFF], 4);
                break;
            case 1:
                // in a 32kb block the first 16kb are allocated to the arm7 and the next 16kb are allocated to the arm9
                memcpy(&return_value, &shared_wram[(addr & 0x3FFF) + 0x4000], 4);
                break;
            case 2:
                // in a 32kb block the first 16kb are allocated to the arm9 and the next 16kb are allocated to the arm7
                memcpy(&return_value, &shared_wram[addr & 0x3FFF], 4);
                break;
            case 3:
                // in this case the wram area is empty (undefined)
                return 0;
            default:
                log_fatal("handle");
            }
            break;
        case REGION_IO:
            switch (addr) {
            case 0x040000DC:
                return core->dma[1].channel[3].DMACNT;
            case 0x040000EC:
                return core->dma[1].DMAFILL[3];
            case 0x04000180:
                return core->ipc.ReadIPCSYNC9();
            case 0x04000208:
                return core->interrupt[1].IME & 0x1;
            case 0x04000210:
                return core->interrupt[1].IE;
            case 0x04000214:
                return core->interrupt[1].IF;
            case 0x04000240:
                return ((core->gpu.VRAMCNT_D << 24) | (core->gpu.VRAMCNT_C << 16) | (core->gpu.VRAMCNT_B << 8) | (core->gpu.VRAMCNT_A));
            case 0x04100000:
                // just return the first item in the fifo xd
                return core->ipc.fifo9[0];
            case 0x04004000:
                return 0;
            case 0x04004008:
                return 0;
            default:
                log_fatal("unimplemented arm9 word io read at address 0x%08x", addr);
            }
            break;
        case REGION_ARM9_BIOS:
            memcpy(&return_value, &arm9_bios[addr & 0x7FFF], 4);
            break;
        default:
            log_fatal("unimplemented arm9 word read at address 0x%08x\n", addr);
        }
    }

    return return_value;
}

void Memory::ARM9WriteByte(u32 addr, u8 data) {
    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        main_memory[addr & 0x3FFFFF] = data;
        break;
    case REGION_IO:
        switch (addr) {
        case 0x04000208:
            core->interrupt[1].IME = data & 0x1;
            break;
        case 0x04000240:
            core->gpu.VRAMCNT_A = data;
            break;
        case 0x04000241:
            core->gpu.VRAMCNT_B = data;
            break;
        case 0x04000242:
            core->gpu.VRAMCNT_C = data;
            break;
        case 0x04000243:
            core->gpu.VRAMCNT_D = data;
            break;
        case 0x04000244:
            core->gpu.VRAMCNT_E = data;
            break;
        case 0x04000245:
            core->gpu.VRAMCNT_F = data;
            break;
        case 0x04000246:
            core->gpu.VRAMCNT_G = data;
            break;
        case 0x04000248:
            core->gpu.VRAMCNT_H = data;
            break;
        case 0x04000249:
            core->gpu.VRAMCNT_I = data;
            break;
        default:
            log_fatal("unimplemented arm9 byte io write at address 0x%08x with data 0x%02x", addr, data);
        }
        break;
    default:
        log_fatal("unimplemented arm9 byte write at address 0x%08x with data 0x%02x", addr, data);
    }
}

void Memory::ARM9WriteHalfword(u32 addr, u16 data) {
    addr &= ~1;

    switch (addr >> 24) {
    case REGION_MAIN_MEMORY:
        memcpy(&main_memory[addr & 0x3FFFFF], &data, 2);
        break;
    case REGION_IO:
        switch (addr) {
        case 0x04000004:
            core->gpu.WriteDISPSTAT9(data);
            break;
        case 0x040000D0:
            core->dma[1].WriteLength(2, data);
            break;
        case 0x04000100:
            core->timers[1].WriteCounter(0, data);
            break;
        case 0x04000102:
            core->timers[1].WriteControl(0, data);
            break;
        case 0x04000104:
            core->timers[1].WriteCounter(1, data);
            break;
        case 0x04000106:
            core->timers[1].WriteControl(1, data);
            break;
        case 0x04000108:
            core->timers[1].WriteCounter(2, data);
            break;
        case 0x0400010A:
            core->timers[1].WriteControl(2, data);
            break;
        case 0x0400010C:
            core->timers[1].WriteCounter(3, data);
            break;
        case 0x0400010E:
            core->timers[1].WriteControl(3, data);
            break;
        case 0x04000180:
            core->ipc.WriteIPCSYNC9(data);
            break;
        case 0x04000184:
            core->ipc.WriteIPCFIFOCNT9(data);
            break;
        case 0x04000208:
            core->interrupt[1].IME = data & 0x1;
            break;
        case 0x04000304:
            core->gpu.POWCNT1 = data;
            break;
        default:
            log_fatal("unimplemented arm9 halfword io write at address 0x%08x with data 0x%04x", addr, data);
        }
        break;
    case REGION_VRAM:
        if (addr >= 0x06800000) {
            core->gpu.WriteLCDC(addr, data);
        } else {
            log_fatal("handle at address 0x%08x", addr);
        }
        break;
    default:
        log_fatal("unimplemented arm9 halfword write at address 0x%08x with data 0x%04x", addr, data);
    }
}

void Memory::ARM9WriteWord(u32 addr, u32 data) {
    addr &= ~3;

    if (core->cp15.GetITCMEnabled() && (addr < core->cp15.GetITCMSize())) {
        memcpy(&core->cp15.itcm[addr & 0x7FFF], &data, 4);
        // done with the write
        return;
    } else if (core->cp15.GetDTCMEnabled() && 
        in_range(core->cp15.GetDTCMBase(), core->cp15.GetDTCMBase() + core->cp15.GetDTCMSize(), addr)) {

        memcpy(&core->cp15.dtcm[(addr - core->cp15.GetDTCMBase()) & 0x3FFF], &data, 4);
        
        // done with the write
        return;
    } else {
        switch (addr >> 24) {
        case REGION_MAIN_MEMORY:
            memcpy(&main_memory[addr & 0x3FFFFF], &data, 4);
            break;
        case REGION_IO:
            switch (addr) {
            case 0x04000000:
                core->gpu.engine_a.DISPCNT = data;
                break;
            case 0x04000004:
                core->gpu.WriteDISPSTAT9(data & 0xFFFF);
                break;
            case 0x04000008:
                core->gpu.engine_a.BGCNT[0] = data & 0xFFFF;
                core->gpu.engine_a.BGCNT[1] = data >> 16;
                break;
            case 0x0400000C:
                core->gpu.engine_a.BGCNT[2] = data & 0xFFFF;
                core->gpu.engine_a.BGCNT[3] = data >> 16;
                break;
            case 0x04000010:
                core->gpu.engine_a.BGHOFS[0] = data & 0xFFFF;
                core->gpu.engine_a.BGVOFS[0] = data >> 16;
                break;
            case 0x04000014:
                core->gpu.engine_a.BGHOFS[1] = data & 0xFFFF;
                core->gpu.engine_a.BGVOFS[1] = data >> 16;
                break;
            case 0x04000018:
                core->gpu.engine_a.BGHOFS[2] = data & 0xFFFF;
                core->gpu.engine_a.BGVOFS[2] = data >> 16;
                break;
            case 0x0400001C:
                core->gpu.engine_a.BGHOFS[3] = data & 0xFFFF;
                core->gpu.engine_a.BGVOFS[3] = data >> 16;
                break;
            case 0x04000020:
                core->gpu.engine_a.BG2P[0] = data & 0xFFFF;
                core->gpu.engine_a.BG2P[1] = data >> 16;
                break;
            case 0x04000024:
                core->gpu.engine_a.BG2P[2] = data & 0xFFFF;
                core->gpu.engine_a.BG2P[3] = data >> 16;
                break;
            case 0x04000028:
                core->gpu.engine_a.BG2X = data;
                break;
            case 0x0400002C:
                core->gpu.engine_a.BG2Y = data;
                break;
            case 0x04000030:
                core->gpu.engine_a.BG3P[0] = data & 0xFFFF;
                core->gpu.engine_a.BG3P[1] = data >> 16;
                break;
            case 0x04000034:
                core->gpu.engine_a.BG3P[2] = data & 0xFFFF;
                core->gpu.engine_a.BG3P[3] = data >> 16;
                break;
            case 0x04000038:
                core->gpu.engine_a.BG3X = data;
                break;
            case 0x0400003C:
                core->gpu.engine_a.BG3Y = data;
                break;
            case 0x04000040:
                core->gpu.engine_a.WINH[0] = data & 0xFFFF;
                core->gpu.engine_a.WINH[1] = data >> 16;
                break;
            case 0x04000044:
                core->gpu.engine_a.WINV[0] = data & 0xFFFF;
                core->gpu.engine_a.WINV[1] = data >> 16;
                break;
            case 0x04000048:
                core->gpu.engine_a.WININ = data & 0xFFFF;
                core->gpu.engine_a.WINOUT = data >> 16;
                break;
            case 0x0400004C:
                core->gpu.engine_a.MOSAIC = data;
                break;
            case 0x04000050:
                core->gpu.engine_a.BLDCNT = data & 0xFFFF;
                core->gpu.engine_a.BLDALPHA = data >> 16;
                break;
            case 0x040000B0:
                core->dma[1].channel[0].source = data;
                break;
            case 0x040000B4:
                core->dma[1].channel[0].destination = data;
                break;
            case 0x040000B8:
                core->dma[1].WriteDMACNT(0, data);
                break;
            case 0x040000BC:
                core->dma[1].channel[1].source = data;
                break;
            case 0x040000C0:
                core->dma[1].channel[1].destination = data;
                break;
            case 0x040000C4:
                core->dma[1].WriteDMACNT(1, data);
                break;
            case 0x040000C8:
                core->dma[1].channel[2].source = data;
                break;
            case 0x040000CC:
                core->dma[1].channel[2].destination = data;
                break;
            case 0x040000D0:
                core->dma[1].WriteDMACNT(2, data);
                break;
            case 0x040000D4:
                core->dma[1].channel[3].source = data;
                break;
            case 0x040000D8:
                core->dma[1].channel[3].destination = data;
                break;
            case 0x040000DC:
                core->dma[1].WriteDMACNT(3, data);
                break;
            case 0x040000EC:
                core->dma[1].DMAFILL[3] = data;
                break;
            case 0x04000180:
                core->ipc.WriteIPCSYNC9(data);
                break;
            case 0x04000208:
                core->interrupt[1].IME = data & 0x1;
                break;
            case 0x04000210:
                core->interrupt[1].IE = data;
                break;
            case 0x04000214:
                // when writing to IF a value of 1 actually resets the bit to acknowledge the interrupt while writing 0 has no change
                core->interrupt[1].IF &= ~(data);
                break;
            case 0x04000240:
                // sets vramcnt_a, vramcnt_b, vramcnt_c and vramcnt_d
                core->gpu.VRAMCNT_A = data & 0xFF;
                core->gpu.VRAMCNT_B = (data >> 8) & 0xFF;
                core->gpu.VRAMCNT_C = (data >> 16) & 0xFF;
                core->gpu.VRAMCNT_D = (data >> 24) & 0xFF;
                break;
            case 0x04000304:
                core->gpu.POWCNT1 = data;
                break;
            case 0x04001000:
                core->gpu.engine_b.DISPCNT = data;
                break;
            case 0x04001008:
                core->gpu.engine_b.BGCNT[0] = data & 0xFFFF;
                core->gpu.engine_b.BGCNT[1] = data >> 16;
                break;
            case 0x0400100C:
                core->gpu.engine_b.BGCNT[2] = data & 0xFFFF;
                core->gpu.engine_b.BGCNT[3] = data >> 16;
                break;
            case 0x04001010:
                core->gpu.engine_b.BGHOFS[0] = data & 0xFFFF;
                core->gpu.engine_b.BGVOFS[0] = data >> 16;
                break;
            case 0x04001014:
                core->gpu.engine_b.BGHOFS[1] = data & 0xFFFF;
                core->gpu.engine_b.BGVOFS[1] = data >> 16;
                break;
            case 0x04001018:
                core->gpu.engine_b.BGHOFS[2] = data & 0xFFFF;
                core->gpu.engine_b.BGVOFS[2] = data >> 16;
                break;
            case 0x0400101C:
                core->gpu.engine_b.BGHOFS[3] = data & 0xFFFF;
                core->gpu.engine_b.BGVOFS[3] = data >> 16;
                break;
            case 0x04001020:
                core->gpu.engine_b.BG2P[0] = data & 0xFFFF;
                core->gpu.engine_b.BG2P[1] = data >> 16;
                break;
            case 0x04001024:
                core->gpu.engine_b.BG2P[2] = data & 0xFFFF;
                core->gpu.engine_b.BG2P[3] = data >> 16;
                break;
            case 0x04001028:
                core->gpu.engine_b.BG2X = data;
                break;
            case 0x0400102C:
                core->gpu.engine_b.BG2Y = data;
                break;
            case 0x04001030:
                core->gpu.engine_b.BG3P[0] = data & 0xFFFF;
                core->gpu.engine_b.BG3P[1] = data >> 16;
                break;
            case 0x04001034:
                core->gpu.engine_b.BG3P[2] = data & 0xFFFF;
                core->gpu.engine_b.BG3P[3] = data >> 16;
                break;
            case 0x04001038:
                core->gpu.engine_b.BG3X = data;
                break;
            case 0x0400103C:
                core->gpu.engine_b.BG3Y = data;
                break;
            case 0x04001040:
                core->gpu.engine_b.WINH[0] = data & 0xFFFF;
                core->gpu.engine_b.WINH[1] = data >> 16;
                break;
            case 0x04001044:
                core->gpu.engine_b.WINV[0] = data & 0xFFFF;
                core->gpu.engine_b.WINV[1] = data >> 16;
                break;
            case 0x04001048:
                core->gpu.engine_b.WININ = data & 0xFFFF;
                core->gpu.engine_b.WINOUT = data >> 16;
                break;
            case 0x0400104C:
                core->gpu.engine_b.MOSAIC = data;
                break;
            case 0x04001050:
                core->gpu.engine_b.BLDCNT = data & 0xFFFF;
                core->gpu.engine_b.BLDALPHA = data >> 16;
                break;
            case 0x04001054:
                core->gpu.engine_b.BLDY = data;
                break;
            case 0x04001058:
                return;
            default:
                log_fatal("unimplemented arm9 word io write at address 0x%08x with data 0x%08x", addr, data);
            }
            break;
        case REGION_PALETTE_RAM:
            case 0x05000000:
            // write to palette ram
            // check depending on the address which engines palette ram to write to
            if ((addr & 0x3FF) < 400) {
                // this is the first block which is assigned to engine a
                core->gpu.engine_a.WritePaletteRAM(addr, data & 0xFFFF);
                core->gpu.engine_a.WritePaletteRAM(addr + 2, data >> 16);
            } else {
                // write to engine b's palette ram
                core->gpu.engine_b.WritePaletteRAM(addr, data & 0xFFFF);
                core->gpu.engine_b.WritePaletteRAM(addr + 2, data >> 16);
            }
            break;
        case REGION_VRAM:
            if (addr >= 0x06800000) {
                core->gpu.WriteLCDC(addr, data & 0xFFFF);
                core->gpu.WriteLCDC(addr + 2, data >> 16);
            } else {
                log_fatal("handle at address 0x%08x", addr);
            }
            break;
        case REGION_OAM:
            // check memory address to see which engine to write to oam
            if ((addr & 0x3FF) < 0x400) {
                // this is the first block of oam which is 1kb and is assigned to engine a
                core->gpu.engine_a.WriteOAM(addr, data & 0xFFFF);
                core->gpu.engine_a.WriteOAM(addr, data >> 16);
            } else {
                // write to engine b's palette ram
                core->gpu.engine_b.WriteOAM(addr, data & 0xFFFF);
                core->gpu.engine_b.WriteOAM(addr + 2, data >> 16);
            }
            break;
        default:
            log_fatal("unimplemented arm9 word write at address 0x%08x with data 0x%08x\n", addr, data);
        }
    }
}

void Memory::LoadARM7BIOS() {
    FILE* file_buffer = fopen("../bios/bios7.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("error when opening arm7 bios! make sure the file bios7.bin exists in the bios folder");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(arm7_bios, 0x4000, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("arm7 bios loaded successfully!");
}

void Memory::LoadARM9BIOS() {
    FILE* file_buffer = fopen("../bios/bios9.bin", "rb");
    if (file_buffer == NULL) {
        log_fatal("error when opening arm9 bios! make sure the file bios9.bin exists in the bios folder");
    }
    fseek(file_buffer, 0, SEEK_END);
    fseek(file_buffer, 0, SEEK_SET);
    fread(arm9_bios, 0x8000, 1, file_buffer);
    fclose(file_buffer);  
    log_debug("arm9 bios loaded successfully!");
}


