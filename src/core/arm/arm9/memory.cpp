#include <common/log.h>
#include <core/arm/arm9/memory.h>
#include <core/core.h>

ARM9Memory::ARM9Memory(System& system) : system(system) {
    bios = LoadBios<0x8000>("../bios/bios9.bin");
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
        } else if (system.cp15.GetDTCMReadEnabled() && in_range(system.cp15.GetDTCMBase(), system.cp15.GetDTCMSize())) {
            read_page_table[index] = &system.cp15.dtcm[(addr - system.cp15.GetDTCMBase()) & 0x3FFF];
        } else {
            switch (addr >> 24) {
            case 0x02:
                read_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (system.WRAMCNT) {
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
        } else if (system.cp15.GetDTCMWriteEnabled() && in_range(system.cp15.GetDTCMBase(), system.cp15.GetDTCMSize())) {
            write_page_table[index] = &system.cp15.dtcm[(addr - system.cp15.GetDTCMBase()) & 0x3FFF];
        } else {
            switch (addr >> 24) {
            case 0x02:
                write_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (system.WRAMCNT) {
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
    case 0x04:
        switch (addr) {
        case 0x040001A8:
        case 0x040001A9:
        case 0x040001AA:
        case 0x040001AB:
        case 0x040001AC:
        case 0x040001AD:
        case 0x040001AE:
        case 0x040001AF:
            // recieve a cartridge command and store in the buffer
            return system.cartridge.ReadCommand(addr - 0x040001A8);
        case 0x04000208:
            return system.cpu_core[1].ime & 0x1;
        case 0x04000300:
            return system.POSTFLG9;
        case 0x04004000:
            return 0;
        default:
            log_fatal("[ARM9] Undefined 8-bit io read %08x", addr);
        }
    case 0x05:
        // TODO: change to templated version
        if ((addr & 0x7FF) < 400) {
            // this is the first block which is assigned to engine a
            return system.gpu.engine_a.palette_ram[addr & 0x3FF];
        } else {
            // write to engine b's palette ram
            return system.gpu.engine_b.palette_ram[addr & 0x3FF];
        }
        break;
    case 0x06:
        return system.gpu.ReadVRAM<u8>(addr);
    case 0x07:
        if ((addr & 0x7FF) < 0x400) {
            return system.gpu.engine_a.ReadOAM<u8>(addr);
        } else {
            return system.gpu.engine_b.ReadOAM<u8>(addr);
        }

        break;
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
    u16 return_value = 0;

    switch (addr >> 24) {
    case 0x03:
        switch (system.WRAMCNT) {
        case 0:
            memcpy(&return_value, &system.shared_wram[addr & 0x7FFF], 2);
            break;
        case 1:
            memcpy(&return_value, &system.shared_wram[(addr & 0x3FFF) + 0x4000], 2);
            break;
        case 2:
            memcpy(&return_value, &system.shared_wram[addr & 0x3FFF], 2);
            break;
        case 3:
            return 0;
        }
        break;
    case 0x04:
        switch (addr) {
        case 0x04000000:
            return system.gpu.engine_a.DISPCNT & 0xFFFF;
        case 0x04000002:
            return system.gpu.engine_a.DISPCNT >> 16;
        case 0x04000004:
            return system.gpu.DISPSTAT9;
        case 0x04000006:
            return system.gpu.VCOUNT;
        case 0x04000008:
            return system.gpu.engine_a.BGCNT[0];
        case 0x0400000A:
            return system.gpu.engine_a.BGCNT[1];
        case 0x0400000C:
            return system.gpu.engine_a.BGCNT[2];
        case 0x0400000E:
            return system.gpu.engine_a.BGCNT[3];
        case 0x04000044:
            return system.gpu.engine_a.WINV[0];
        case 0x04000046:
            return system.gpu.engine_a.WINV[1];
        case 0x04000048:
            return system.gpu.engine_a.WININ;
        case 0x0400004A:
            return system.gpu.engine_a.WINOUT;
        case 0x04000050:
            return system.gpu.engine_a.BLDCNT;
        case 0x04000060:
            return system.gpu.render_engine.disp3dcnt;
        case 0x0400006C:
            return system.gpu.engine_a.MASTER_BRIGHT;
        case 0x040000BA:
            return system.dma[1].ReadDMACNT_H(0);
        case 0x040000C6:
            return system.dma[1].ReadDMACNT_H(1);
        case 0x040000D2:
            return system.dma[1].ReadDMACNT_H(2);
        case 0x040000DE:
            return system.dma[1].ReadDMACNT_H(3);
        case 0x040000EC:
            return system.dma[1].DMAFILL[3] & 0xFFFF;
        case 0x04000100:
            return system.timers[1].ReadTMCNT_L(0);
        case 0x04000104:
            return system.timers[1].ReadTMCNT_L(1);
        case 0x04000108:
            return system.timers[1].ReadTMCNT_L(2);
        case 0x0400010C:
            return system.timers[1].ReadTMCNT_L(3);
        case 0x04000130:
            return system.input.KEYINPUT;
        case 0x04000180:
            return system.ipc.ReadIPCSYNC9();
        case 0x04000184:
            return system.ipc.IPCFIFOCNT9;
        case 0x040001A0:
            return system.cartridge.AUXSPICNT;
        case 0x040001A2:
            return system.cartridge.AUXSPIDATA;
        case 0x04000204:
            return system.EXMEMCNT;
        case 0x04000208:
            return system.cpu_core[1].ime & 0x1;
        case 0x04000280:
            return system.maths_unit.DIVCNT;
        case 0x040002B0:
            return system.maths_unit.SQRTCNT;
        case 0x04000300:
            return system.POSTFLG9;
        case 0x04000304:
            return system.gpu.POWCNT1;
        case 0x04000320:
            return 0;
        case 0x04000604:
            return system.gpu.render_engine.polygon_ram_size;
        case 0x04000606:
            return system.gpu.render_engine.vertex_ram_size;
        case 0x04001000:
            return system.gpu.engine_b.DISPCNT & 0xFFFF;
        case 0x04001008:
            return system.gpu.engine_b.BGCNT[0];
        case 0x0400100A:
            return system.gpu.engine_b.BGCNT[1];
        case 0x0400100C:
            return system.gpu.engine_b.BGCNT[2];
        case 0x0400100E:
            return system.gpu.engine_b.BGCNT[3];
        case 0x04001044:
            return system.gpu.engine_b.WINV[0];
        case 0x04001046:
            return system.gpu.engine_b.WINV[1];
        case 0x04001048:
            return system.gpu.engine_b.WININ;
        case 0x0400104A:
            return system.gpu.engine_b.WINOUT;
        case 0x04001050:
            return system.gpu.engine_b.BLDCNT;
        case 0x0400106C:
            return system.gpu.engine_b.MASTER_BRIGHT;
        case 0x04004004:
            return 0;
        case 0x04004010:
            return 0;
        default:
            log_fatal("[ARM9] Undefined 16-bit io read %08x", addr);
        }
    case 0x05:
        // TODO: change to templated version
        if ((addr & 0x7FF) < 400) {
            // this is the first block which is assigned to engine a
            memcpy(&return_value, &system.gpu.engine_a.palette_ram[addr & 0x3FF], 2);
        } else {
            // write to engine b's palette ram
            memcpy(&return_value, &system.gpu.engine_b.palette_ram[addr & 0x3FF], 2);
        }
        break;
    case 0x06:
        return system.gpu.ReadVRAM<u16>(addr);
    case 0x07:
        if ((addr & 0x7FF) < 0x400) {
            // this is the first block of oam which is 1kb and is assigned to engine a
            return_value = system.gpu.engine_a.ReadOAM<u16>(addr);
        } else {
            // write to engine b's palette ram
            return_value = system.gpu.engine_b.ReadOAM<u16>(addr);
        }

        break;
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

    return return_value;
}

u32 ARM9Memory::ReadWord(u32 addr) {
    u32 return_value = 0;

    if ((addr >= 0x04000640) && (addr < 0x04000680)) {
        return system.gpu.geometry_engine.ReadClipMatrix(addr);
    }

    if ((addr >= 0x04000680) && (addr < 0x040006A4)) {
        return system.gpu.geometry_engine.ReadVectorMatrix(addr);
    }

    switch (addr >> 24) {
    case 0x03:
        switch (system.WRAMCNT) {
        case 0:
            memcpy(&return_value, &system.shared_wram[addr & 0x7FFF], 4);
            break;
        case 1:
            memcpy(&return_value, &system.shared_wram[(addr & 0x3FFF) + 0x4000], 4);
            break;
        case 2:
            memcpy(&return_value, &system.shared_wram[addr & 0x3FFF], 4);
            break;
        case 3:
            return 0;
        }
        break;
    case 0x04:
        switch (addr) {
        case 0x04000000:
            return system.gpu.engine_a.DISPCNT;
        case 0x04000004:
            return (system.gpu.VCOUNT << 16) | (system.gpu.DISPSTAT9);
        case 0x04000008:
            return (system.gpu.engine_a.BGCNT[1] << 16) | (system.gpu.engine_a.BGCNT[0]);
        case 0x0400000C:
            return (system.gpu.engine_a.BGCNT[3] << 16) | (system.gpu.engine_a.BGCNT[2]);
        case 0x04000010:
            return (system.gpu.engine_a.BGHOFS[0] << 16) | (system.gpu.engine_a.BGVOFS[0]);
        case 0x04000014:
            return (system.gpu.engine_a.BGHOFS[1] << 16) | (system.gpu.engine_a.BGVOFS[1]);
        case 0x04000018:
            return (system.gpu.engine_a.BGHOFS[2] << 16) | (system.gpu.engine_a.BGVOFS[2]);
        case 0x0400001C:
            return (system.gpu.engine_a.BGHOFS[3] << 16) | (system.gpu.engine_a.BGVOFS[3]);
        case 0x04000048:
            return (system.gpu.engine_a.WINOUT << 16) | system.gpu.engine_a.WININ;
        case 0x0400004C:
            return 0;
        case 0x04000050:
            return (system.gpu.engine_a.BLDALPHA << 16) | (system.gpu.engine_a.BLDCNT);
        case 0x04000054:
            return system.gpu.engine_a.BLDY;
        case 0x04000058:
            return 0;
        case 0x0400005C:
            return 0;
        case 0x04000064:
            return 0;
        case 0x04000068:
            return 0;
        case 0x0400006C:
            return 0;
        case 0x040000B0:
            return system.dma[1].channel[0].source;
        case 0x040000B4:
            return system.dma[1].channel[0].destination;
        case 0x040000B8:
            return system.dma[1].ReadDMACNT(0);
        case 0x040000BC:
            return system.dma[1].channel[1].source;
        case 0x040000C0:
            return system.dma[1].channel[1].destination;
        case 0x040000C4:
            return system.dma[1].ReadDMACNT(1);
        case 0x040000C8:
            return system.dma[1].channel[2].source;
        case 0x040000CC:
            return system.dma[1].channel[2].destination;
        case 0x040000D0:
            return system.dma[1].ReadDMACNT(2);
        case 0x040000D4:
            return system.dma[1].channel[3].source;
        case 0x040000D8:
            return system.dma[1].channel[3].destination;
        case 0x040000DC:
            return system.dma[1].ReadDMACNT(3);
        case 0x040000E0:
            return system.dma[1].DMAFILL[0];
        case 0x040000E4:
            return system.dma[1].DMAFILL[1];
        case 0x040000E8:
            return system.dma[1].DMAFILL[2];
        case 0x040000EC:
            return system.dma[1].DMAFILL[3];
        case 0x04000100:
            return system.timers[1].ReadTMCNT(0);
        case 0x04000180:
            return system.ipc.ReadIPCSYNC9();
        case 0x040001A4:
            return system.cartridge.ROMCTRL;
        case 0x04000208:
            return system.cpu_core[1].ime & 0x1;
        case 0x04000210:
            return system.cpu_core[1].ie;
        case 0x04000214:
            return system.cpu_core[1].irf;
        case 0x04000240:
            return ((system.gpu.VRAMCNT_D << 24) | (system.gpu.VRAMCNT_C << 16) | (system.gpu.VRAMCNT_B << 8) | (system.gpu.VRAMCNT_A));
        case 0x04000280:
            return system.maths_unit.DIVCNT;
        case 0x04000290:
            return system.maths_unit.DIV_NUMER & 0xFFFFFFFF;
        case 0x04000294:
            return system.maths_unit.DIV_NUMER >> 32;
        case 0x04000298:
            return system.maths_unit.DIV_DENOM & 0xFFFFFFFF;
        case 0x0400029C:
            return system.maths_unit.DIV_DENOM >> 32;
        case 0x040002A0:
            return system.maths_unit.DIV_RESULT & 0xFFFFFFFF;
        case 0x040002A4:
            return system.maths_unit.DIV_RESULT >> 32;
        case 0x040002A8:
            return system.maths_unit.DIVREM_RESULT & 0xFFFFFFFF;
        case 0x040002AC:
            return system.maths_unit.DIVREM_RESULT >> 32;
        case 0x040002B4:
            return system.maths_unit.SQRT_RESULT;
        case 0x040002B8:
            return system.maths_unit.SQRT_PARAM & 0xFFFFFFFF;
        case 0x040002BC:
            return system.maths_unit.SQRT_PARAM >> 32;
        case 0x040004A4:
            // is polygon attr even readable?
            return 0;
        case 0x04000600:
            return system.gpu.geometry_engine.ReadGXSTAT();
        case 0x04001000:
            return system.gpu.engine_b.DISPCNT;
        case 0x04001010:
            return (system.gpu.engine_b.BGHOFS[0] << 16) | (system.gpu.engine_b.BGVOFS[0]);
        case 0x04001014:
            return (system.gpu.engine_b.BGHOFS[1] << 16) | (system.gpu.engine_b.BGVOFS[1]);
        case 0x04001018:
            return (system.gpu.engine_b.BGHOFS[2] << 16) | (system.gpu.engine_b.BGVOFS[2]);
        case 0x0400101C:
            return (system.gpu.engine_b.BGHOFS[3] << 16) | (system.gpu.engine_b.BGVOFS[3]);
        case 0x04004000:
            return 0;
        case 0x04004008:
            return 0;
        case 0x04100000:
            return system.ipc.ReadFIFORECV9();
        case 0x04100010:
            return system.cartridge.ReadData();
        default:
            log_fatal("[ARM9] Undefined 32-bit io read %08x", addr);
        }
    case 0x05:
        if ((addr & 0x7FF) < 0x400) {
            system.gpu.engine_a.ReadPaletteRAM<u32>(addr);
        } else {
            system.gpu.engine_b.ReadPaletteRAM<u32>(addr);
        }

        break;
    case 0x06:
        return system.gpu.ReadVRAM<u32>(addr);
    case 0x07:
        if ((addr & 0x7FF) < 0x400) {
            return_value = system.gpu.engine_a.ReadOAM<u32>(addr);
        } else {
            return_value = system.gpu.engine_b.ReadOAM<u32>(addr);
        }

        break;
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
    default:
        log_fatal("handle word read from %08x", addr);
    }

    return return_value;
}

void ARM9Memory::WriteByte(u32 addr, u8 data) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x04000040:
            system.gpu.engine_a.WINH[0] = (system.gpu.engine_a.WINH[0] & ~0xFF) | data;
            break;
        case 0x04000041:
            system.gpu.engine_a.WINH[0] = (system.gpu.engine_a.WINH[0] & 0xFF) | (data << 8);
            break;
        case 0x04000044:
            system.gpu.engine_a.WINV[0] = (system.gpu.engine_a.WINV[0] & ~0xFF) | data;
            break;
        case 0x04000045:
            system.gpu.engine_a.WINV[0] = (system.gpu.engine_a.WINV[0] & 0xFF) | (data << 8);
            break;
        case 0x0400004C:
            system.gpu.engine_a.MOSAIC = (system.gpu.engine_a.MOSAIC & ~0xFF) | data;
            break;
        case 0x0400004D:
            system.gpu.engine_a.MOSAIC = (system.gpu.engine_a.MOSAIC & ~0xFF00) | (data << 8);
            break;
        case 0x0400004E:
            system.gpu.engine_a.MOSAIC = (system.gpu.engine_a.MOSAIC & ~0xFF0000) | (data << 16);
            break;
        case 0x0400004F:
            system.gpu.engine_a.MOSAIC = (system.gpu.engine_a.MOSAIC & ~0xFF000000) | (data << 24);
            break;
        case 0x040001A1:
            // write to the high byte of AUXSPICNT
            system.cartridge.AUXSPICNT = (system.cartridge.AUXSPICNT & 0xFF) | (data << 8);
            break;
        case 0x040001A8:
        case 0x040001A9:
        case 0x040001AA:
        case 0x040001AB:
        case 0x040001AC:
        case 0x040001AD:
        case 0x040001AE:
        case 0x040001AF:
            // recieve a cartridge command and store in the buffer
            system.cartridge.ReceiveCommand(data, addr - 0x040001A8);
            break;
        case 0x04000208:
            system.cpu_core[1].ime = data & 0x1;
            break;
        case 0x04000240:
            system.gpu.VRAMCNT_A = data;
            system.gpu.MapVRAM();
            break;
        case 0x04000241:
            system.gpu.VRAMCNT_B = data;
            system.gpu.MapVRAM();
            break;
        case 0x04000242:
            system.gpu.VRAMCNT_C = data;
            // if vramcnt_c has an mst of 2 and is now enabled,
            // then set bit 0 of VRAMSTAT (vram bank c allocated to the arm7)
            if ((data & (1 << 7)) && ((data & 0x7) == 2)) {
                // then set bit 0
                system.gpu.VRAMSTAT |= 1;
            } else {
                // reset bit 0
                system.gpu.VRAMSTAT &= ~1;
            }

            system.gpu.MapVRAM();
            break;
        case 0x04000243:
            system.gpu.VRAMCNT_D = data;
            // if vramcnt_d has an mst of 2 and is now enabled,
            // then set bit 0 of VRAMSTAT (vram bank d allocated to the arm7)
            if ((data & (1 << 7)) && ((data & 0x7) == 2)) {
                // then set bit 0
                system.gpu.VRAMSTAT |= (1 << 1);
            } else {
                // reset bit 0
                system.gpu.VRAMSTAT &= ~(1 << 1);
            }

            system.gpu.MapVRAM();
            break;
        case 0x04000244:
            system.gpu.VRAMCNT_E = data;
            system.gpu.MapVRAM();
            break;
        case 0x04000245:
            system.gpu.VRAMCNT_F = data;
            system.gpu.MapVRAM();
            break;
        case 0x04000246:
            system.gpu.VRAMCNT_G = data;
            system.gpu.MapVRAM();
            break;
        case 0x04000247:
            system.WriteWRAMCNT(data);
            break;
        case 0x04000248:
            system.gpu.VRAMCNT_H = data;
            system.gpu.MapVRAM();
            break;
        case 0x04000249:
            system.gpu.VRAMCNT_I = data;
            system.gpu.MapVRAM();
            break;
        case 0x04000300:
            system.POSTFLG9 = data;
            break;
        case 0x0400104C:
            system.gpu.engine_b.MOSAIC = (system.gpu.engine_b.MOSAIC & ~0xFF) | data;
            break;
        case 0x0400104D:
            system.gpu.engine_b.MOSAIC = (system.gpu.engine_b.MOSAIC & ~0xFF00) | (data << 8);
            break;
        case 0x0400104E:
            system.gpu.engine_b.MOSAIC = (system.gpu.engine_b.MOSAIC & ~0xFF0000) | (data << 16);
            break;
        case 0x0400104F:
            system.gpu.engine_b.MOSAIC = (system.gpu.engine_b.MOSAIC & ~0xFF000000) | (data << 24);
            break;
        default:
            log_fatal("[ARM9] Undefined 8-bit io write %08x = %02x", addr, data);
            break;
        }
        break;
    case 0x05:
        if ((addr & 0x7FF) < 0x400) {
            system.gpu.engine_a.WritePaletteRAM<u8>(addr, data);
        } else {
            system.gpu.engine_b.WritePaletteRAM<u8>(addr, data);
        }

        break;
    case 0x06:
        system.gpu.WriteVRAM<u8>(addr, data);
        break;
    default:
        log_fatal("handle byte write from %08x", addr);
    }
}

void ARM9Memory::WriteHalf(u32 addr, u16 data) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x04000000:
            system.gpu.engine_a.DISPCNT = (system.gpu.engine_a.DISPCNT & ~0xFFFF) | data;
            break;
        case 0x04000004:
            system.gpu.WriteDISPSTAT9(data);
            break;
        case 0x04000008:
            system.gpu.engine_a.BGCNT[0] = data;
            break;
        case 0x0400000A:
            system.gpu.engine_a.BGCNT[1] = data;
            break;
        case 0x0400000C:
            system.gpu.engine_a.BGCNT[2] = data;
            break;
        case 0x0400000E:
            system.gpu.engine_a.BGCNT[3] = data;
            break;
        case 0x04000010:
            system.gpu.engine_a.BGHOFS[0] = data;
            break;
        case 0x04000012:
            system.gpu.engine_a.BGVOFS[0] = data;
            break;
        case 0x04000014:
            system.gpu.engine_a.BGHOFS[1] = data;
            break;
        case 0x04000016:
            system.gpu.engine_a.BGVOFS[1] = data;
            break;
        case 0x04000018:
            system.gpu.engine_a.BGHOFS[2] = data;
            break;
        case 0x0400001A:
            system.gpu.engine_a.BGVOFS[2] = data;
            break;
        case 0x0400001C:
            system.gpu.engine_a.BGHOFS[3] = data;
            break;
        case 0x0400001E:
            system.gpu.engine_a.BGVOFS[3] = data;
            break;
        case 0x04000020:
            system.gpu.engine_a.BGPA[0] = data;
            break;
        case 0x04000022:
            system.gpu.engine_a.BGPB[0] = data;
            break;
        case 0x04000024:
            system.gpu.engine_a.BGPC[0] = data;
            break;
        case 0x04000026:
            system.gpu.engine_a.BGPD[0] = data;
            break;
        case 0x04000030:
            system.gpu.engine_a.BGPA[1] = data;
            break;
        case 0x04000032:
            system.gpu.engine_a.BGPB[1] = data;
            break;
        case 0x04000034:
            system.gpu.engine_a.BGPC[1] = data;
            break;
        case 0x04000036:
            system.gpu.engine_a.BGPD[1] = data;
            break;
        case 0x04000040:
            system.gpu.engine_a.WINH[0] = data;
            break;
        case 0x04000042:
            system.gpu.engine_a.WINH[1] = data;
            break;
        case 0x04000044:
            system.gpu.engine_a.WINV[0] = data;
            break;
        case 0x04000046:
            system.gpu.engine_a.WINV[1] = data;
            break;
        case 0x04000048:
            system.gpu.engine_a.WININ = data;
            break;
        case 0x0400004A:
            system.gpu.engine_a.WINOUT = data;
            break;
        case 0x0400004C:
            system.gpu.engine_a.MOSAIC = data;
            break;
        case 0x04000050:
            system.gpu.engine_a.BLDCNT = data;
            break;
        case 0x04000052:
            system.gpu.engine_a.BLDALPHA = data;
            break;
        case 0x04000054:
            system.gpu.engine_a.BLDY = data;
            break;
        case 0x04000060:
            system.gpu.render_engine.disp3dcnt = data;
            break;
        case 0x04000068:
            // DISP_MMEM_FIFO
            // handle later
            break;
        case 0x0400006C:
            // TODO: handle brightness properly later
            system.gpu.engine_a.MASTER_BRIGHT = data;
            break;
        case 0x040000B8:
            system.dma[1].WriteDMACNT_L(0, data);
            break;
        case 0x040000BA:
            system.dma[1].WriteDMACNT_H(0, data);
            break;
        case 0x040000C4:
            system.dma[1].WriteDMACNT_L(1, data);
            break;
        case 0x040000C6:
            system.dma[1].WriteDMACNT_H(1, data);
            break;
        case 0x040000D0:
            system.dma[1].WriteDMACNT_L(2, data);
            break;
        case 0x040000D2:
            system.dma[1].WriteDMACNT_H(2, data);
            break;
        case 0x040000DC:
            system.dma[1].WriteDMACNT_L(3, data);
            break;
        case 0x040000DE:
            system.dma[1].WriteDMACNT_H(3, data);
            break;
        case 0x04000100:
            system.timers[1].WriteTMCNT_L(0, data);
            break;
        case 0x04000102:
            system.timers[1].WriteTMCNT_H(0, data);
            break;
        case 0x04000104:
            system.timers[1].WriteTMCNT_L(1, data);
            break;
        case 0x04000106:
            system.timers[1].WriteTMCNT_H(1, data);
            break;
        case 0x04000108:
            system.timers[1].WriteTMCNT_L(2, data);
            break;
        case 0x0400010A:
            system.timers[1].WriteTMCNT_H(2, data);
            break;
        case 0x0400010C:
            system.timers[1].WriteTMCNT_L(3, data);
            break;
        case 0x0400010E:
            system.timers[1].WriteTMCNT_H(3, data);
            break;
        case 0x04000130:
            system.input.KEYINPUT = data;
            break;
        case 0x04000180:
            system.ipc.WriteIPCSYNC9(data);
            break;
        case 0x04000184:
            system.ipc.WriteIPCFIFOCNT9(data);
            break;
        case 0x040001A0:
            system.cartridge.WriteAUXSPICNT(data);
            break;
        case 0x040001A2:
            system.cartridge.WriteAUXSPIDATA(data);
            break;
        case 0x04000204:
            system.EXMEMCNT = data;
            break;
        case 0x04000208:
            system.cpu_core[1].ime = data & 0x1;
            break;
        case 0x04000248:
            system.gpu.VRAMCNT_H = data & 0xFF;
            system.gpu.VRAMCNT_I = data >> 8;

            system.gpu.MapVRAM();
            break;
        case 0x04000280:
            system.maths_unit.DIVCNT = data;
            system.maths_unit.StartDivision();
            break;
        case 0x040002B0:
            system.maths_unit.SQRTCNT = data;
            system.maths_unit.StartSquareRoot();
            break;
        case 0x04000300:
            system.POSTFLG9 = data;
            break;
        case 0x04000304:
            system.gpu.POWCNT1 = data;
            break;
        case 0x04000330: case 0x04000331: case 0x04000332: case 0x04000333:
        case 0x04000334: case 0x04000335: case 0x04000336: case 0x04000337:
        case 0x04000338: case 0x04000339: case 0x0400033A: case 0x0400033B:
        case 0x0400033C: case 0x0400033D: case 0x0400033E: case 0x0400033F:
            // system.gpu.render_engine.edge_colour[addr - 0x04000330] = data;
            break;
        case 0x04000340:
            system.gpu.render_engine.alpha_test_ref = data & 0xFF;
            break;
        case 0x04000354:
            system.gpu.render_engine.clear_depth = data;
            break;
        case 0x04000356:
            system.gpu.render_engine.clrimage_offset = data;
            break;
        case 0x0400035C:
            system.gpu.render_engine.fog_offset = data;
            break;
        case 0x04000360: case 0x04000361: case 0x04000362: case 0x04000363:
        case 0x04000364: case 0x04000365: case 0x04000366: case 0x04000367:
        case 0x04000368: case 0x04000369: case 0x0400036A: case 0x0400036B:
        case 0x0400036C: case 0x0400036D: case 0x0400036E: case 0x0400036F:
        case 0x04000370: case 0x04000371: case 0x04000372: case 0x04000373:
        case 0x04000374: case 0x04000375: case 0x04000376: case 0x04000377:
        case 0x04000378: case 0x04000379: case 0x0400037A: case 0x0400037B:
        case 0x0400037C: case 0x0400037D: case 0x0400037E: case 0x0400037F:
            // system.gpu.render_engine.FOG_TABLE[addr - 0x04000360] = data;
            break;
        case 0x04000380: case 0x04000381: case 0x04000382: case 0x04000383:
        case 0x04000384: case 0x04000385: case 0x04000386: case 0x04000387:
        case 0x04000388: case 0x04000389: case 0x0400038A: case 0x0400038B:
        case 0x0400038C: case 0x0400038D: case 0x0400038E: case 0x0400038F:
        case 0x04000390: case 0x04000391: case 0x04000392: case 0x04000393:
        case 0x04000394: case 0x04000395: case 0x04000396: case 0x04000397:
        case 0x04000398: case 0x04000399: case 0x0400039A: case 0x0400039B:
        case 0x0400039C: case 0x0400039D: case 0x0400039E: case 0x0400039F:
        case 0x040003A0: case 0x040003A1: case 0x040003A2: case 0x040003A3:
        case 0x040003A4: case 0x040003A5: case 0x040003A6: case 0x040003A7:
        case 0x040003A8: case 0x040003A9: case 0x040003AA: case 0x040003AB:
        case 0x040003AC: case 0x040003AD: case 0x040003AE: case 0x040003AF:
        case 0x040003B0: case 0x040003B1: case 0x040003B2: case 0x040003B3:
        case 0x040003B4: case 0x040003B5: case 0x040003B6: case 0x040003B7:
        case 0x040003B8: case 0x040003B9: case 0x040003BA: case 0x040003BB:
        case 0x040003BC: case 0x040003BD: case 0x040003BE: case 0x040003BF:
            // system.gpu.render_engine.TOON_TABLE[addr - 0x04000380] = data;
            break;
        case 0x04000610:
            system.gpu.geometry_engine.disp_1dot_depth = data;
            break;
        case 0x04001000:
            system.gpu.engine_b.DISPCNT = (system.gpu.engine_b.DISPCNT & ~0xFFFF) | data;
            break;
        case 0x04001008:
            system.gpu.engine_b.BGCNT[0] = data;
            break;
        case 0x0400100A:
            system.gpu.engine_b.BGCNT[1] = data;
            break;
        case 0x0400100C:
            system.gpu.engine_b.BGCNT[2] = data;
            break;
        case 0x0400100E:
            system.gpu.engine_b.BGCNT[3] = data;
            break;
        case 0x04001010:
            system.gpu.engine_b.BGHOFS[0] = data;
            break;
        case 0x04001012:
            system.gpu.engine_b.BGVOFS[0] = data;
            break;
        case 0x04001014:
            system.gpu.engine_b.BGHOFS[1] = data;
            break;
        case 0x04001016:
            system.gpu.engine_b.BGVOFS[1] = data;
            break;
        case 0x04001018:
            system.gpu.engine_b.BGHOFS[2] = data;
            break;
        case 0x0400101A:
            system.gpu.engine_b.BGVOFS[2] = data;
            break;
        case 0x0400101C:
            system.gpu.engine_b.BGHOFS[3] = data;
            break;
        case 0x0400101E:
            system.gpu.engine_b.BGVOFS[3] = data;
            break;
        case 0x04001020:
            system.gpu.engine_b.BGPA[0] = data;
            break;
        case 0x04001022:
            system.gpu.engine_b.BGPB[0] = data;
            break;
        case 0x04001024:
            system.gpu.engine_b.BGPC[0] = data;
            break;
        case 0x04001026:
            system.gpu.engine_b.BGPD[0] = data;
            break;
        case 0x04001030:
            system.gpu.engine_b.BGPA[1] = data;
            break;
        case 0x04001032:
            system.gpu.engine_b.BGPB[1] = data;
            break;
        case 0x04001034:
            system.gpu.engine_b.BGPC[1] = data;
            break;
        case 0x04001036:
            system.gpu.engine_b.BGPD[1] = data;
            break;
        case 0x04001040:
            system.gpu.engine_b.WINH[0] = data;
            break;
        case 0x04001042:
            system.gpu.engine_b.WINH[1] = data;
            break;
        case 0x04001044:
            system.gpu.engine_b.WINV[0] = data;
            break;
        case 0x04001046:
            system.gpu.engine_b.WINV[1] = data;
            break;
        case 0x04001048:
            system.gpu.engine_b.WININ = data;
            break;
        case 0x0400104A:
            system.gpu.engine_b.WINOUT = data;
            break;
        case 0x0400104C:
            system.gpu.engine_b.MOSAIC = data;
            break;
        case 0x04001050:
            system.gpu.engine_b.BLDCNT = data;
            break;
        case 0x04001052:
            system.gpu.engine_b.BLDALPHA = data;
            break;
        case 0x04001054:
            system.gpu.engine_b.BLDY = data;
            break;
        case 0x0400106C:
            // TODO: handle brightness properly later
            system.gpu.engine_b.MASTER_BRIGHT = data;
            break;
        default:
            log_fatal("[ARM9] Undefined 16-bit io write %08x = %04x", addr, data);
        }
        break;
    case 0x05:
        if ((addr & 0x7FF) < 0x400) {
            // this is the first block of oam which is 1kb and is assigned to engine a
            system.gpu.engine_a.WritePaletteRAM<u16>(addr, data);
        } else {
            // write to engine b's palette ram
            system.gpu.engine_b.WritePaletteRAM<u16>(addr, data);
        }

        break;
    case 0x06:
        system.gpu.WriteVRAM<u16>(addr, data);
        break;
    case 0x07:
        if ((addr & 0x7FF) < 0x400) {
            // this is the first block of oam which is 1kb and is assigned to engine a
            system.gpu.engine_a.WriteOAM<u16>(addr, data);
        } else {
            // write to engine b's palette ram
            system.gpu.engine_b.WriteOAM<u16>(addr, data);
        }

        break;
    case 0x08: case 0x09:
        // for now do nothing lol
        break;
    default:
        log_fatal("handle half write to %08x", addr);
    }
}

void ARM9Memory::WriteWord(u32 addr, u32 data) {
    if ((addr >= 0x04000400) && (addr < 0x04000440)) {
        system.gpu.geometry_engine.WriteGXFIFO(data);
        return;
    }

    if ((addr >= 0x04000440) && (addr < 0x040005CC)) {
        system.gpu.geometry_engine.QueueCommand(addr, data);
        return;
    }

    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x04000000:
            system.gpu.engine_a.DISPCNT = data;
            break;
        case 0x04000004:
            system.gpu.WriteDISPSTAT9(data & 0xFFFF);
            break;
        case 0x04000008:
            system.gpu.engine_a.BGCNT[0] = data & 0xFFFF;
            system.gpu.engine_a.BGCNT[1] = data >> 16;
            break;
        case 0x0400000C:
            system.gpu.engine_a.BGCNT[2] = data & 0xFFFF;
            system.gpu.engine_a.BGCNT[3] = data >> 16;
            break;
        case 0x04000010:
            system.gpu.engine_a.BGHOFS[0] = data & 0xFFFF;
            system.gpu.engine_a.BGVOFS[0] = data >> 16;
            break;
        case 0x04000014:
            system.gpu.engine_a.BGHOFS[1] = data & 0xFFFF;
            system.gpu.engine_a.BGVOFS[1] = data >> 16;
            break;
        case 0x04000018:
            system.gpu.engine_a.BGHOFS[2] = data & 0xFFFF;
            system.gpu.engine_a.BGVOFS[2] = data >> 16;
            break;
        case 0x0400001C:
            system.gpu.engine_a.BGHOFS[3] = data & 0xFFFF;
            system.gpu.engine_a.BGVOFS[3] = data >> 16;
            break;
        case 0x04000020:
            system.gpu.engine_a.BGPA[0] = data & 0xFFFF;
            system.gpu.engine_a.BGPB[0] = data >> 16;
            break;
        case 0x04000024:
            system.gpu.engine_a.BGPC[0] = data & 0xFFFF;
            system.gpu.engine_a.BGPD[0] = data >> 16;
            break;
        case 0x04000028:
            system.gpu.engine_a.WriteBGX(2, data);
            break;
        case 0x0400002C:
            system.gpu.engine_a.WriteBGY(2, data);
            break;
        case 0x04000030:
            system.gpu.engine_a.BGPA[1] = data & 0xFFFF;
            system.gpu.engine_a.BGPB[1] = data >> 16;
            break;
        case 0x04000034:
            system.gpu.engine_a.BGPC[1] = data & 0xFFFF;
            system.gpu.engine_a.BGPD[1] = data >> 16;
            break;
        case 0x04000038:
            system.gpu.engine_a.WriteBGX(3, data);
            break;
        case 0x0400003C:
            system.gpu.engine_a.WriteBGY(3, data);
            break;
        case 0x04000040:
            system.gpu.engine_a.WINH[0] = data & 0xFFFF;
            system.gpu.engine_a.WINH[1] = data >> 16;
            break;
        case 0x04000044:
            system.gpu.engine_a.WINV[0] = data & 0xFFFF;
            system.gpu.engine_a.WINV[1] = data >> 16;
            break;
        case 0x04000048:
            system.gpu.engine_a.WININ = data & 0xFFFF;
            system.gpu.engine_a.WINOUT = data >> 16;
            break;
        case 0x0400004C:
            system.gpu.engine_a.MOSAIC = data;
            break;
        case 0x04000050:
            system.gpu.engine_a.BLDCNT = data & 0xFFFF;
            system.gpu.engine_a.BLDALPHA = data >> 16;
            break;
        case 0x04000054:
            system.gpu.engine_a.BLDY = data;
            break;
        case 0x04000058: case 0x0400005C: case 0x04000060:
            break;
        case 0x04000064:
            system.gpu.DISPCAPCNT = data;
            break;
        case 0x04000068:
            // handle main memory display fifo later
            break;
        case 0x0400006C:
            system.gpu.engine_a.MASTER_BRIGHT = data & 0xFFFF;
            break;
        case 0x04000070: case 0x04000074: case 0x04000078: case 0x0400007C:
        case 0x04000080: case 0x04000084: case 0x04000088: case 0x0400008C:
        case 0x04000090: case 0x04000094: case 0x04000098: case 0x0400009C:
        case 0x040000A0: case 0x040000A4: case 0x040000A8: case 0x040000AC:
        case 0x040000F0: case 0x040000F4: case 0x040000F8: case 0x040000FC:
            // no clue why the firmware does this (will investigate later)
            break;
        case 0x040000B0:
            system.dma[1].channel[0].source = data;
            break;
        case 0x040000B4:
            system.dma[1].channel[0].destination = data;
            break;
        case 0x040000B8:
            system.dma[1].WriteDMACNT(0, data);
            break;
        case 0x040000BC:
            system.dma[1].channel[1].source = data;
            break;
        case 0x040000C0:
            system.dma[1].channel[1].destination = data;
            break;
        case 0x040000C4:
            system.dma[1].WriteDMACNT(1, data);
            break;
        case 0x040000C8:
            system.dma[1].channel[2].source = data;
            break;
        case 0x040000CC:
            system.dma[1].channel[2].destination = data;
            break;
        case 0x040000D0:
            system.dma[1].WriteDMACNT(2, data);
            break;
        case 0x040000D4:
            system.dma[1].channel[3].source = data;
            break;
        case 0x040000D8:
            system.dma[1].channel[3].destination = data;
            break;
        case 0x040000DC:
            system.dma[1].WriteDMACNT(3, data);
            break;
        case 0x040000E0:
            system.dma[1].DMAFILL[0] = data;
            break;
        case 0x040000E4:
            system.dma[1].DMAFILL[1] = data;
            break;
        case 0x040000E8:
            system.dma[1].DMAFILL[2] = data;
            break;
        case 0x040000EC:
            system.dma[1].DMAFILL[3] = data;
            break;
        case 0x040001A0:
            system.cartridge.WriteAUXSPICNT(data & 0xFFFF);
            system.cartridge.WriteAUXSPIDATA(data >> 16);
            break;
        case 0x040001A4:
            system.cartridge.WriteROMCTRL(data);
            break;
        case 0x040001A8:
            system.cartridge.ReceiveCommand(data & 0xFF, 0);
            system.cartridge.ReceiveCommand((data >> 8) & 0xFF, 1);
            system.cartridge.ReceiveCommand((data >> 16) & 0xFF, 2);
            system.cartridge.ReceiveCommand((data >> 24) & 0xFF, 3);
            break;
        case 0x040001AC:
            system.cartridge.ReceiveCommand(data & 0xFF, 4);
            system.cartridge.ReceiveCommand((data >> 8) & 0xFF, 5);
            system.cartridge.ReceiveCommand((data >> 16) & 0xFF, 6);
            system.cartridge.ReceiveCommand((data >> 24) & 0xFF, 7);
            break;
        case 0x04000180:
            system.ipc.WriteIPCSYNC9(data);
            break;
        case 0x04000188:
            system.ipc.WriteFIFOSEND9(data);
            break;
        case 0x04000208:
            system.cpu_core[1].ime = data & 0x1;
            break;
        case 0x04000210:
            system.cpu_core[1].ie = data;
            break;
        case 0x04000214:
            system.cpu_core[1].irf &= ~data;
            break;
        case 0x04000240:
            system.gpu.VRAMCNT_A = data & 0xFF;
            system.gpu.VRAMCNT_B = (data >> 8) & 0xFF;
            system.gpu.VRAMCNT_C = (data >> 16) & 0xFF;
            system.gpu.VRAMCNT_D = (data >> 24) & 0xFF;

            system.gpu.MapVRAM();
            break;
        case 0x04000244:
            // sets vramcnt_e, vramcnt_f, vramcnt_g and wramcnt
            system.gpu.VRAMCNT_E = data & 0xFF;
            system.gpu.VRAMCNT_F = (data >> 8) & 0xFF;
            system.gpu.VRAMCNT_G = (data >> 16) & 0xFF;
            system.WriteWRAMCNT((data >> 24) & 0xFF);

            system.gpu.MapVRAM();
            break;
        case 0x04000280:
            system.maths_unit.DIVCNT = data;
            system.maths_unit.StartDivision();
        case 0x04000290:
            // write to lower 32 bits of DIV_NUMER, starting a division
            system.maths_unit.DIV_NUMER = (system.maths_unit.DIV_NUMER & ~0xFFFFFFFF) | data;
            system.maths_unit.StartDivision();
            break;
        case 0x04000294:
            // write to upper 32 bits of DIV_NUMER, starting a division
            system.maths_unit.DIV_NUMER = (system.maths_unit.DIV_NUMER & 0xFFFFFFFF) | ((u64)data << 32);
            system.maths_unit.StartDivision();
            break;
        case 0x04000298:
            // write to lower 32 bits of DIV_DENOM, starting a division
            system.maths_unit.DIV_DENOM = (system.maths_unit.DIV_DENOM & ~0xFFFFFFFF) | data;
            system.maths_unit.StartDivision();
            break;
        case 0x0400029C:
            // write to upper 32 bits of DIV_DENOM, starting a division
            system.maths_unit.DIV_DENOM = (system.maths_unit.DIV_DENOM & 0xFFFFFFFF) | ((u64)data << 32);
            system.maths_unit.StartDivision();
            break;
        case 0x040002A0: case 0x040002A4: case 0x040002A8:
            break;
        case 0x040002B0:
            system.maths_unit.SQRTCNT = (system.maths_unit.SQRTCNT & ~0xFFFF) | data;
            system.maths_unit.StartSquareRoot();
            break;
        case 0x040002B8:
            // write to lower 32 bits of SQRT_PARAM
            system.maths_unit.SQRT_PARAM = (system.maths_unit.SQRT_PARAM & ~0xFFFFFFFF) | data;
            system.maths_unit.StartSquareRoot();
            break;
        case 0x040002BC:
            // write to upper 32 bits of SQRT_PARAM
            system.maths_unit.SQRT_PARAM = (system.maths_unit.SQRT_PARAM & 0xFFFFFFFF) | ((u64)data << 32);
            system.maths_unit.StartSquareRoot();
            break;
        case 0x04000304:
            system.gpu.POWCNT1 = data;
            break;
        case 0x04000330: case 0x04000331: case 0x04000332: case 0x04000333:
        case 0x04000334: case 0x04000335: case 0x04000336: case 0x04000337:
        case 0x04000338: case 0x04000339: case 0x0400033A: case 0x0400033B:
        case 0x0400033C: case 0x0400033D: case 0x0400033E: case 0x0400033F:
            // system.gpu.render_engine.edge_colour[addr - 0x04000330] = data;
            break;
        case 0x04000350:
            system.gpu.render_engine.clear_colour = data;
            break;
        case 0x04000358:
            system.gpu.render_engine.fog_colour = data;
            break;
        case 0x04000360: case 0x04000361: case 0x04000362: case 0x04000363:
        case 0x04000364: case 0x04000365: case 0x04000366: case 0x04000367:
        case 0x04000368: case 0x04000369: case 0x0400036A: case 0x0400036B:
        case 0x0400036C: case 0x0400036D: case 0x0400036E: case 0x0400036F:
        case 0x04000370: case 0x04000371: case 0x04000372: case 0x04000373:
        case 0x04000374: case 0x04000375: case 0x04000376: case 0x04000377:
        case 0x04000378: case 0x04000379: case 0x0400037A: case 0x0400037B:
        case 0x0400037C: case 0x0400037D: case 0x0400037E: case 0x0400037F:
            // system.gpu.render_engine.FOG_TABLE[addr - 0x04000360] = data;
            break;
        case 0x04000380: case 0x04000381: case 0x04000382: case 0x04000383:
        case 0x04000384: case 0x04000385: case 0x04000386: case 0x04000387:
        case 0x04000388: case 0x04000389: case 0x0400038A: case 0x0400038B:
        case 0x0400038C: case 0x0400038D: case 0x0400038E: case 0x0400038F:
        case 0x04000390: case 0x04000391: case 0x04000392: case 0x04000393:
        case 0x04000394: case 0x04000395: case 0x04000396: case 0x04000397:
        case 0x04000398: case 0x04000399: case 0x0400039A: case 0x0400039B:
        case 0x0400039C: case 0x0400039D: case 0x0400039E: case 0x0400039F:
        case 0x040003A0: case 0x040003A1: case 0x040003A2: case 0x040003A3:
        case 0x040003A4: case 0x040003A5: case 0x040003A6: case 0x040003A7:
        case 0x040003A8: case 0x040003A9: case 0x040003AA: case 0x040003AB:
        case 0x040003AC: case 0x040003AD: case 0x040003AE: case 0x040003AF:
        case 0x040003B0: case 0x040003B1: case 0x040003B2: case 0x040003B3:
        case 0x040003B4: case 0x040003B5: case 0x040003B6: case 0x040003B7:
        case 0x040003B8: case 0x040003B9: case 0x040003BA: case 0x040003BB:
        case 0x040003BC: case 0x040003BD: case 0x040003BE: case 0x040003BF:
            // system.gpu.render_engine.TOON_TABLE[addr - 0x04000380] = data;
            break;
        case 0x04000600:
            system.gpu.geometry_engine.WriteGXSTAT(data);
            break;
        case 0x04001000:
            system.gpu.engine_b.DISPCNT = data;
            break;
        case 0x04001004:
            break;
        case 0x04001008:
            system.gpu.engine_b.BGCNT[0] = data & 0xFFFF;
            system.gpu.engine_b.BGCNT[1] = data >> 16;
            break;
        case 0x0400100C:
            system.gpu.engine_b.BGCNT[2] = data & 0xFFFF;
            system.gpu.engine_b.BGCNT[3] = data >> 16;
            break;
        case 0x04001010:
            system.gpu.engine_b.BGHOFS[0] = data & 0xFFFF;
            system.gpu.engine_b.BGVOFS[0] = data >> 16;
            break;
        case 0x04001014:
            system.gpu.engine_b.BGHOFS[1] = data & 0xFFFF;
            system.gpu.engine_b.BGVOFS[1] = data >> 16;
            break;
        case 0x04001018:
            system.gpu.engine_b.BGHOFS[2] = data & 0xFFFF;
            system.gpu.engine_b.BGVOFS[2] = data >> 16;
            break;
        case 0x0400101C:
            system.gpu.engine_b.BGHOFS[3] = data & 0xFFFF;
            system.gpu.engine_b.BGVOFS[3] = data >> 16;
            break;
        case 0x04001020:
            system.gpu.engine_b.BGPA[0] = data & 0xFFFF;
            system.gpu.engine_b.BGPB[0] = data >> 16;
            break;
        case 0x04001024:
            system.gpu.engine_b.BGPC[0] = data & 0xFFFF;
            system.gpu.engine_b.BGPD[0] = data >> 16;
            break;
        case 0x04001028:
            system.gpu.engine_b.WriteBGX(2, data);
            break;
        case 0x0400102C:
            system.gpu.engine_b.WriteBGY(2, data);
            break;
        case 0x04001030:
            system.gpu.engine_b.BGPA[1] = data & 0xFFFF;
            system.gpu.engine_b.BGPB[1] = data >> 16;
            break;
        case 0x04001034:
            system.gpu.engine_b.BGPC[1] = data & 0xFFFF;
            system.gpu.engine_b.BGPD[1] = data >> 16;
            break;
        case 0x04001038:
            system.gpu.engine_b.WriteBGX(3, data);
            break;
        case 0x0400103C:
            system.gpu.engine_b.WriteBGY(3, data);
            break;
        case 0x04001040:
            system.gpu.engine_b.WINH[0] = data & 0xFFFF;
            system.gpu.engine_b.WINH[1] = data >> 16;
            break;
        case 0x04001044:
            system.gpu.engine_b.WINV[0] = data & 0xFFFF;
            system.gpu.engine_b.WINV[1] = data >> 16;
            break;
        case 0x04001048:
            system.gpu.engine_b.WININ = data & 0xFFFF;
            system.gpu.engine_b.WINOUT = data >> 16;
            break;
        case 0x0400104C:
            system.gpu.engine_b.MOSAIC = data;
            break;
        case 0x04001050:
            system.gpu.engine_b.BLDCNT = data & 0xFFFF;
            system.gpu.engine_b.BLDALPHA = data >> 16;
            break;
        case 0x04001054:
            system.gpu.engine_b.BLDY = data;
            break;
        case 0x04001058: case 0x0400105C: case 0x04001060: case 0x04001064: case 0x04001068:
            break;
        case 0x0400106C:
            system.gpu.engine_b.MASTER_BRIGHT = data & 0xFFFF;
            break;
        default:
            log_warn("[ARM9] Undefined 32-bit io write %08x = %08x", addr, data);
        }
        break;
    case 0x05:
        if ((addr & 0x7FF) < 0x400) {
            // this is the first block of oam which is 1kb and is assigned to engine a
            system.gpu.engine_a.WritePaletteRAM<u32>(addr, data);
        } else {
            // write to engine b's palette ram
            system.gpu.engine_b.WritePaletteRAM<u32>(addr, data);
        }

        break;
    case 0x06:
        system.gpu.WriteVRAM<u32>(addr, data);
        break;
    case 0x07:
        if ((addr & 0x7FF) < 0x400) {
            // this is the first block of oam which is 1kb and is assigned to engine a
            system.gpu.engine_a.WriteOAM<u32>(addr, data);
        } else {
            // write to engine b's palette ram
            system.gpu.engine_b.WriteOAM<u32>(addr, data);
        }

        break;
    case 0x08: case 0x09:
        // for now do nothing lol
        break;
    default:
        log_fatal("handle word write to %08x", addr);
    }
}