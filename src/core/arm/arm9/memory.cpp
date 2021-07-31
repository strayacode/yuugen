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
    switch (addr > 24) {
    default:
        log_fatal("handle byte read from %08x", addr);
    }
}

auto ARM9Memory::ReadHalf(u32 addr) -> u16 {
    switch (addr >> 24) {
    case REGION_IO:
        switch (addr) {
        case 0x04000000:
            return hw->gpu.engine_a.DISPCNT & 0xFFFF;
        case 0x04000004:
            return hw->gpu.DISPSTAT9;
        case 0x04000006:
            return hw->gpu.VCOUNT;
        case 0x04000008:
            return hw->gpu.engine_a.BGCNT[0];
        case 0x0400000A:
            return hw->gpu.engine_a.BGCNT[1];
        case 0x0400000C:
            return hw->gpu.engine_a.BGCNT[2];
        case 0x0400000E:
            return hw->gpu.engine_a.BGCNT[3];
        case 0x04000048:
            return hw->gpu.engine_a.WININ;
        case 0x0400004A:
            return hw->gpu.engine_a.WINOUT;
        case 0x04000050:
            return hw->gpu.engine_a.BLDCNT;
        case 0x04000060:
            return hw->gpu.render_engine.DISP3DCNT;
        case 0x0400006C:
            return hw->gpu.engine_a.MASTER_BRIGHT;
        case 0x040000BA:
            return hw->dma[1].ReadDMACNT_H(0);
        case 0x040000C6:
            return hw->dma[1].ReadDMACNT_H(1);
        case 0x040000D2:
            return hw->dma[1].ReadDMACNT_H(2);
        case 0x040000DE:
            return hw->dma[1].ReadDMACNT_H(3);
        case 0x040000EC:
            return hw->dma[1].DMAFILL[3] & 0xFFFF;
        case 0x04000100:
            return hw->timers[1].ReadTMCNT_L(0);
        case 0x04000104:
            return hw->timers[1].ReadTMCNT_L(1);
        case 0x04000108:
            return hw->timers[1].ReadTMCNT_L(2);
        case 0x0400010C:
            return hw->timers[1].ReadTMCNT_L(3);
        case 0x04000130:
            return hw->input.KEYINPUT;
        case 0x04000180:
            return hw->ipc.ReadIPCSYNC9();
        case 0x04000184:
            return hw->ipc.IPCFIFOCNT9;
        case 0x040001A0:
            return hw->cartridge.AUXSPICNT;
        case 0x04000204:
            return hw->EXMEMCNT;
        default:
            log_fatal("handle half read from %08x", addr);
        }
    default:
        log_fatal("handle half read from %08x", addr);
    }
}

auto ARM9Memory::ReadWord(u32 addr) -> u32 {
    switch (addr > 24) {
    default:
        log_fatal("handle word read from %08x", addr);
    }
}

void ARM9Memory::WriteByte(u32 addr, u8 data) {
    switch (addr >> 24) {
    case REGION_IO:
        switch (addr) {
        case 0x04000040:
            hw->gpu.engine_a.WINH[0] = (hw->gpu.engine_a.WINH[0] & ~0xFF) | data;
            break;
        case 0x04000041:
            hw->gpu.engine_a.WINH[0] = (hw->gpu.engine_a.WINH[0] & 0xFF) | (data << 8);
            break;
        case 0x04000044:
            hw->gpu.engine_a.WINV[0] = (hw->gpu.engine_a.WINV[0] & ~0xFF) | data;
            break;
        case 0x04000045:
            hw->gpu.engine_a.WINV[0] = (hw->gpu.engine_a.WINV[0] & 0xFF) | (data << 8);
            break;
        case 0x0400004C:
            hw->gpu.engine_a.MOSAIC = (hw->gpu.engine_a.MOSAIC & ~0xFF) | data;
            break;
        case 0x0400004D:
            hw->gpu.engine_a.MOSAIC = (hw->gpu.engine_a.MOSAIC & ~0xFF00) | (data << 8);
            break;
        case 0x0400004E:
            hw->gpu.engine_a.MOSAIC = (hw->gpu.engine_a.MOSAIC & ~0xFF0000) | (data << 16);
            break;
        case 0x0400004F:
            hw->gpu.engine_a.MOSAIC = (hw->gpu.engine_a.MOSAIC & ~0xFF000000) | (data << 24);
            break;
        case 0x040001A1:
            // write to the high byte of AUXSPICNT
            hw->cartridge.AUXSPICNT = (hw->cartridge.AUXSPICNT & 0xFF) | (data << 8);
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
            hw->cartridge.ReceiveCommand(data, addr - 0x040001A8);
            break;
        case 0x04000240:
            hw->gpu.VRAMCNT_A = data;
            hw->gpu.MapVRAM();
            break;
        case 0x04000241:
            hw->gpu.VRAMCNT_B = data;
            hw->gpu.MapVRAM();
            break;
        default:
            log_fatal("handle byte write from %08x", addr);
        }
        break;
    default:
        log_fatal("handle byte write from %08x", addr);
    }
}

void ARM9Memory::WriteHalf(u32 addr, u16 data) {
    switch (addr >> 24) {
    case REGION_IO:
        switch (addr) {
        case 0x04000000:
            hw->gpu.engine_a.DISPCNT = (hw->gpu.engine_a.DISPCNT & ~0xFFFF) | data;
            break;
        case 0x04000004:
            hw->gpu.WriteDISPSTAT9(data);
            break;
        case 0x04000008:
            hw->gpu.engine_a.BGCNT[0] = data;
            break;
        case 0x0400000A:
            hw->gpu.engine_a.BGCNT[1] = data;
            break;
        case 0x0400000C:
            hw->gpu.engine_a.BGCNT[2] = data;
            break;
        case 0x0400000E:
            hw->gpu.engine_a.BGCNT[3] = data;
            break;
        case 0x04000010:
            hw->gpu.engine_a.BGHOFS[0] = data;
            break;
        case 0x04000012:
            hw->gpu.engine_a.BGVOFS[0] = data;
            break;
        case 0x04000014:
            hw->gpu.engine_a.BGHOFS[1] = data;
            break;
        case 0x04000016:
            hw->gpu.engine_a.BGVOFS[1] = data;
            break;
        case 0x04000018:
            hw->gpu.engine_a.BGHOFS[2] = data;
            break;
        case 0x0400001A:
            hw->gpu.engine_a.BGVOFS[2] = data;
            break;
        case 0x0400001C:
            hw->gpu.engine_a.BGHOFS[3] = data;
            break;
        case 0x0400001E:
            hw->gpu.engine_a.BGVOFS[3] = data;
            break;
        case 0x04000020:
            hw->gpu.engine_a.BGPA[0] = data;
            break;
        case 0x04000022:
            hw->gpu.engine_a.BGPB[0] = data;
            break;
        case 0x04000024:
            hw->gpu.engine_a.BGPC[0] = data;
            break;
        case 0x04000026:
            hw->gpu.engine_a.BGPD[0] = data;
            break;
        case 0x04000030:
            hw->gpu.engine_a.BGPA[1] = data;
            break;
        case 0x04000032:
            hw->gpu.engine_a.BGPB[1] = data;
            break;
        case 0x04000034:
            hw->gpu.engine_a.BGPC[1] = data;
            break;
        case 0x04000036:
            hw->gpu.engine_a.BGPD[1] = data;
            break;
        case 0x04000040:
            hw->gpu.engine_a.WINH[0] = data;
            break;
        case 0x04000042:
            hw->gpu.engine_a.WINH[1] = data;
            break;
        case 0x04000044:
            hw->gpu.engine_a.WINV[0] = data;
            break;
        case 0x04000046:
            hw->gpu.engine_a.WINV[1] = data;
            break;
        case 0x04000048:
            hw->gpu.engine_a.WININ = data;
            break;
        case 0x0400004A:
            hw->gpu.engine_a.WINOUT = data;
            break;
        case 0x0400004C:
            hw->gpu.engine_a.MOSAIC = data;
            break;
        case 0x04000050:
            hw->gpu.engine_a.BLDCNT = data;
            break;
        case 0x04000052:
            hw->gpu.engine_a.BLDALPHA = data;
            break;
        case 0x04000054:
            hw->gpu.engine_a.BLDY = data;
            break;
        case 0x04000060:
            hw->gpu.render_engine.DISP3DCNT = data;
            break;
        case 0x04000068:
            // DISP_MMEM_FIFO
            // handle later
            break;
        case 0x0400006C:
            // TODO: handle brightness properly later
            hw->gpu.engine_a.MASTER_BRIGHT = data;
            break;
        case 0x040000B8:
            hw->dma[1].WriteDMACNT_L(0, data);
            break;
        case 0x040000BA:
            hw->dma[1].WriteDMACNT_H(0, data);
            break;
        case 0x040000C4:
            hw->dma[1].WriteDMACNT_L(1, data);
            break;
        case 0x040000C6:
            hw->dma[1].WriteDMACNT_H(1, data);
            break;
        case 0x040000D0:
            hw->dma[1].WriteDMACNT_L(2, data);
            break;
        case 0x040000D2:
            hw->dma[1].WriteDMACNT_H(2, data);
            break;
        case 0x040000DC:
            hw->dma[1].WriteDMACNT_L(3, data);
            break;
        case 0x040000DE:
            hw->dma[1].WriteDMACNT_H(3, data);
            break;
        case 0x04000100:
            hw->timers[1].WriteTMCNT_L(0, data);
            break;
        case 0x04000102:
            hw->timers[1].WriteTMCNT_H(0, data);
            break;
        case 0x04000104:
            hw->timers[1].WriteTMCNT_L(1, data);
            break;
        case 0x04000106:
            hw->timers[1].WriteTMCNT_H(1, data);
            break;
        case 0x04000108:
            hw->timers[1].WriteTMCNT_L(2, data);
            break;
        case 0x0400010A:
            hw->timers[1].WriteTMCNT_H(2, data);
            break;
        case 0x0400010C:
            hw->timers[1].WriteTMCNT_L(3, data);
            break;
        case 0x0400010E:
            hw->timers[1].WriteTMCNT_H(3, data);
            break;
        case 0x04000130:
            hw->input.KEYINPUT = data;
            break;
        case 0x04000180:
            hw->ipc.WriteIPCSYNC9(data);
            break;
        case 0x04000184:
            hw->ipc.WriteIPCFIFOCNT9(data);
            break;
        case 0x040001A0:
            hw->cartridge.WriteAUXSPICNT(data);
            break;
        case 0x04000204:
            hw->EXMEMCNT = data;
            break;
        default:
            log_fatal("unimplemented half io write %08x", addr);
        }
        break;
    case REGION_VRAM:
        hw->gpu.WriteVRAM<u16>(addr, data);
        break;
    default:
        log_fatal("handle half write to %08x", addr);
    }
}

void ARM9Memory::WriteWord(u32 addr, u32 data) {
    switch (addr) {
    case 0x04000000:
        hw->gpu.engine_a.DISPCNT = data;
        break;
    case 0x04000240:
        hw->gpu.VRAMCNT_A = data & 0xFF;
        hw->gpu.VRAMCNT_B = (data >> 8) & 0xFF;
        hw->gpu.VRAMCNT_C = (data >> 16) & 0xFF;
        hw->gpu.VRAMCNT_D = (data >> 24) & 0xFF;

        hw->gpu.MapVRAM();
        break;
    case 0x04000304:
        hw->gpu.POWCNT1 = data;
        break;
    default:
        log_fatal("handle word write to %08x", addr);
    }
}