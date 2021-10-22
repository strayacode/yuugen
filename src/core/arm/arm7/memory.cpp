#include <core/arm/arm7/memory.h>
#include <core/hw/hw.h>

ARM7Memory::ARM7Memory(HW* hw) : hw(hw) {

}

void ARM7Memory::Reset() {
    UpdateMemoryMap(0, 0xFFFFFFFF);
}

void ARM7Memory::UpdateMemoryMap(u32 low_addr, u32 high_addr) {
    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;
        switch (addr >> 24) {
        case 0x00:
            read_page_table[index] = &hw->arm7_bios[addr & 0x3FFF];
            write_page_table[index] = nullptr;
            break;
        case 0x02:
            read_page_table[index] = &hw->main_memory[addr & 0x3FFFFF];
            write_page_table[index] = &hw->main_memory[addr & 0x3FFFFF];
            break;
        case 0x03:
            if (addr < 0x03800000) {
                switch (hw->WRAMCNT) {
                case 0:
                    read_page_table[index] = &hw->arm7_wram[addr & 0xFFFF];
                    write_page_table[index] = &hw->arm7_wram[addr & 0xFFFF];
                    break;
                case 1:
                    read_page_table[index] = &hw->shared_wram[addr & 0x3FFF];
                    write_page_table[index] = &hw->shared_wram[addr & 0x3FFF];
                    break;
                case 2:
                    read_page_table[index] = &hw->shared_wram[(addr & 0x3FFF) + 0x4000];
                    write_page_table[index] = &hw->shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 3:
                    read_page_table[index] = &hw->shared_wram[addr & 0x7FFF];
                    write_page_table[index] = &hw->shared_wram[addr & 0x7FFF];
                    break;
                }
            } else {
                read_page_table[index] = &hw->arm7_wram[addr & 0xFFFF];
                write_page_table[index] = &hw->arm7_wram[addr & 0xFFFF];
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

auto ARM7Memory::ReadByte(u32 addr) -> u8 {
    if (in_range(0x04000400, 0x100)) {
        return hw->spu.ReadByte(addr);
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    switch (addr >> 24) {
    case REGION_IO:
        switch (addr) {
        case 0x04000138:
            return hw->rtc.ReadRTC();
        case 0x040001C2:
            return hw->spi.ReadSPIDATA();
        case 0x04000240:
            return hw->gpu.VRAMSTAT;
        case 0x04000241:
            return hw->WRAMCNT;
        case 0x04000300:
            return hw->POSTFLG7;
        case 0x04000501:
            // read upper byte of SOUNDCNT
            return hw->spu.soundcnt >> 8;
        case 0x04000508:
            return hw->spu.SNDCAPCNT[0];
        case 0x04000509:
            return hw->spu.SNDCAPCNT[1];
        default:
            log_fatal("[ARM7] Undefined 8-bit io read %08x", addr);
        }
    case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (!(hw->EXMEMCNT & (1 << 7))) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFF;
    default:
        log_fatal("[ARM7] Undefined 8-bit read %08x", addr);
    }
}

auto ARM7Memory::ReadHalf(u32 addr) -> u16 {
    u16 return_value = 0;

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    switch (addr >> 24) {
    case REGION_IO:
        switch (addr) {
        case 0x04000004:
            return hw->gpu.DISPSTAT7;
        case 0x04000006:
            return hw->gpu.VCOUNT;
        case 0x040000BA:
            return hw->dma[0].ReadDMACNT_H(0);
        case 0x040000C6:
            return hw->dma[0].ReadDMACNT_H(1);
        case 0x040000D2:
            return hw->dma[0].ReadDMACNT_H(2);
        case 0x040000DE:
            return hw->dma[0].ReadDMACNT_H(3);
        case 0x04000100:
            return hw->timers[0].ReadTMCNT_L(0);
        case 0x04000104:
            return hw->timers[0].ReadTMCNT_L(1);
        case 0x04000108:
            return hw->timers[0].ReadTMCNT_L(2);
        case 0x0400010C:
            return hw->timers[0].ReadTMCNT_L(3);
        case 0x0400010E:
            return hw->timers[0].ReadTMCNT_H(3);
        case 0x04000130:
            return hw->input.KEYINPUT;
        case 0x04000134:
            return hw->RCNT;
        case 0x04000136:
            return hw->input.EXTKEYIN;
        case 0x04000138:
            return hw->rtc.RTC_REG;
        case 0x04000180:
            return hw->ipc.ReadIPCSYNC7();
        case 0x04000184:
            return hw->ipc.IPCFIFOCNT7;
        case 0x040001A0:
            return hw->cartridge.AUXSPICNT;
        case 0x040001A2:
            return hw->cartridge.AUXSPIDATA;
        case 0x040001C0:
            return hw->spi.SPICNT;
        case 0x040001C2:
            return hw->spi.ReadSPIDATA();
        case 0x04000204:
            return hw->EXMEMCNT;
        case 0x04000208:
            return hw->cpu_core[0]->ime & 0x1;
        case 0x04000300:
            return hw->POSTFLG7;
        case 0x04000304:
            return hw->POWCNT2;
        case 0x04000500:
            return hw->spu.soundcnt;
        case 0x04000504:
            return hw->spu.soundbias;
        case 0x04000508:
            return (hw->spu.SNDCAPCNT[0]) | (hw->spu.SNDCAPCNT[1] << 8);
        case 0x04004700:
            return 0;
        case 0x04808006:
            return hw->wifi.W_MODE_WEP;
        case 0x04808018:
            return hw->wifi.W_MACADDR_0;
        case 0x0480801A:
            return hw->wifi.W_MACADDR_1;
        case 0x0480801C:
            return hw->wifi.W_MACADDR_2;
        case 0x04808020:
            return hw->wifi.W_BSSID_0;
        case 0x04808022:
            return hw->wifi.W_BSSID_1;
        case 0x04808024:
            return hw->wifi.W_BSSID_2;
        case 0x0480802A:
            return hw->wifi.W_AID_FULL;
        case 0x04808050:
            return hw->wifi.W_RXBUF_BEGIN;
        case 0x04808052:
            return hw->wifi.W_RXBUF_END;
        case 0x04808056:
            return hw->wifi.W_RXBUF_WR_ADDR;
        default:
            log_fatal("[ARM7] Undefined 16-bit io read %08x", addr);
        }
    case REGION_VRAM:
        return_value = hw->gpu.ReadARM7<u16>(addr);
        break;
    case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (!(hw->EXMEMCNT & (1 << 7))) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFFFF;
    default:
        log_fatal("[ARM7] Undefined 16-bit io read %08x", addr);
    }

    return return_value;
}

auto ARM7Memory::ReadWord(u32 addr) -> u32 {
    if (in_range(0x04000400, 0x100)) {
        return hw->spu.ReadWord(addr);
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    u32 return_value = 0;

    switch (addr >> 24) {
    case REGION_IO:
        switch (addr) {
        case 0x04000004:
            return hw->gpu.VCOUNT;
        case 0x040000B8:
            return hw->dma[0].ReadDMACNT(0);
        case 0x040000DC:
            return hw->dma[0].ReadDMACNT(3);
        case 0x04000180:
            return hw->ipc.ReadIPCSYNC7();
        case 0x040001A4:
            return hw->cartridge.ROMCTRL;
        case 0x040001C0:
            return (hw->spi.ReadSPIDATA() << 16) | hw->spi.SPICNT;
        case 0x04000208:
            return hw->cpu_core[0]->ime & 0x1;
        case 0x04000210:
            return hw->cpu_core[0]->ie;
        case 0x04000214:
            return hw->cpu_core[0]->irf;
        case 0x04004008:
            return 0;
        case 0x04100000:
            return hw->ipc.ReadFIFORECV7();
        case 0x04100010:
            return hw->cartridge.ReadData();
        default:
            log_fatal("[ARM7] Undefined 32-bit io read %08x", addr);
        }
        break;
    case REGION_VRAM:
        return_value = hw->gpu.ReadARM7<u32>(addr);
        break;
    default:
        log_fatal("handle %08x", addr);
    }

    return return_value;
}

void ARM7Memory::WriteByte(u32 addr, u8 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        hw->spu.WriteByte(addr, data);
        return;
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return;
    }

    switch (addr >> 24) {
    case REGION_ARM7_BIOS:
        // ignore all bios writes
        break;
    case REGION_IO:
        switch (addr) {
        case 0x04000138:
            hw->rtc.WriteRTC(data);
            break;
        case 0x040001A0:
            // write to the low byte of AUXSPICNT
            hw->cartridge.AUXSPICNT = (hw->cartridge.AUXSPICNT & ~0xFF) | (data & 0xFF);
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
        case 0x040001C2:
            hw->spi.WriteSPIDATA(data);
            break;
        case 0x04000208:
            hw->cpu_core[0]->ime = data & 0x1;
            break;
        case 0x04000300:
            hw->POSTFLG7 = data;
            break;
        case 0x04000301:
            hw->WriteHALTCNT(data);
            break;
        case 0x04000500:
            // write to lower byte of SOUNDCNT
            hw->spu.soundcnt = (hw->spu.soundcnt & ~0xFF) | (data & 0xFF);
            break;
        case 0x04000501:
            // write to upper byte of SOUNDCNT
            hw->spu.soundcnt = (hw->spu.soundcnt & 0xFF) | (data << 8);
            break;
        case 0x04000508:
            hw->spu.SNDCAPCNT[0] = data;
            break;
        case 0x04000509:
            hw->spu.SNDCAPCNT[1] = data;
            break;
        default:
            log_fatal("[ARM7] Undefined 8-bit io write %08x = %08x", addr, data);
        }
        break;
    case REGION_VRAM:
        hw->gpu.WriteARM7<u8>(addr, data);
        break;
    default:
        log_fatal("[ARM7] Undefined 8-bit write %08x = %08x", addr, data);
    }
}

void ARM7Memory::WriteHalf(u32 addr, u16 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        hw->spu.WriteHalf(addr, data);
        return;
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return;
    }

    switch (addr >> 24) {
    case REGION_ARM7_BIOS:
        // ignore all bios writes
        break;
    case REGION_IO:
        switch (addr) {
        case 0x04000004:
            hw->gpu.WriteDISPSTAT7(data);
            break;
        case 0x040000BA:
            hw->dma[0].WriteDMACNT_H(0, data);
            break;
        case 0x040000C6:
            hw->dma[0].WriteDMACNT_H(1, data);
            break;
        case 0x040000D2:
            hw->dma[0].WriteDMACNT_H(2, data);
            break;
        case 0x040000DE:
            hw->dma[0].WriteDMACNT_H(3, data);
            break;
        case 0x04000100:
            hw->timers[0].WriteTMCNT_L(0, data);
            break;
        case 0x04000102:
            hw->timers[0].WriteTMCNT_H(0, data);
            break;
        case 0x04000104:
            hw->timers[0].WriteTMCNT_L(1, data);
            break;
        case 0x04000106:
            hw->timers[0].WriteTMCNT_H(1, data);
            break;
        case 0x04000108:
            hw->timers[0].WriteTMCNT_L(2, data);
            break;
        case 0x0400010A:
            hw->timers[0].WriteTMCNT_H(2, data);
            break;
        case 0x0400010C:
            hw->timers[0].WriteTMCNT_L(3, data);
            break;
        case 0x0400010E:
            hw->timers[0].WriteTMCNT_H(3, data);
            break;
        case 0x04000128:
            // debug SIOCNT
            hw->SIOCNT = data;
            break;
        case 0x04000134:
            hw->RCNT = data;
            break;
        case 0x04000138:
            hw->rtc.WriteRTC(data);
            break;
        case 0x04000180:
            hw->ipc.WriteIPCSYNC7(data);
            break;
        case 0x04000184:
            hw->ipc.WriteIPCFIFOCNT7(data);
            break;
        case 0x040001A0:
            hw->cartridge.WriteAUXSPICNT(data);
            break;
        case 0x040001A2:
            hw->cartridge.WriteAUXSPIDATA(data);
            break;
        case 0x040001B8: case 0x040001BA:
            // TODO: handle key2 encryption later
            break;
        case 0x040001C0:
            hw->spi.WriteSPICNT(data);
            break;
        case 0x040001C2:
            hw->spi.WriteSPIDATA(data);
            break;
        case 0x04000204:
            hw->EXMEMCNT = data;
            break;
        case 0x04000206:
            // TODO: implement WIFIWAITCNT
            break;
        case 0x04000208:
            hw->cpu_core[0]->ime = data & 0x1;
            break;
        case 0x04000304:
            hw->POWCNT2 = data & 0x3;
            break;
        case 0x04000500:
            hw->spu.soundcnt = data;
            break;
        case 0x04000504:
            hw->spu.soundbias = data & 0x3FF;
            break;
        case 0x04000508:
            hw->spu.SNDCAPCNT[0] = data & 0xFF;
            hw->spu.SNDCAPCNT[1] = data >> 8;
            break;
        case 0x04000514:
            hw->spu.SNDCAPLEN[0] = data;
            break;
        case 0x0400051C:
            hw->spu.SNDCAPLEN[1] = data;
            break;
        case 0x04001080:
            // this address seems to just be for debugging for the ds lite so not useful for us
            break;
        case 0x04808006:
            hw->wifi.W_MODE_WEP = data;
            break;
        case 0x04808018:
            hw->wifi.W_MACADDR_0 = data;
            break;
        case 0x0480801A:
            hw->wifi.W_MACADDR_1 = data;
            break;
        case 0x0480801C:
            hw->wifi.W_MACADDR_2 = data;
            break;
        case 0x04808020:
            hw->wifi.W_BSSID_0 = data;
            break;
        case 0x04808022:
            hw->wifi.W_BSSID_1 = data;
            break;
        case 0x04808024:
            hw->wifi.W_BSSID_2 = data;
            break;
        case 0x0480802A:
            hw->wifi.W_AID_FULL = data;
            break;
        case 0x04808050:
            hw->wifi.W_RXBUF_BEGIN = data;
            break;
        case 0x04808052:
            hw->wifi.W_RXBUF_END = data;
            break;
        case 0x04808056:
            hw->wifi.W_RXBUF_WR_ADDR = data;
            break;
        case 0x048080AE:
            hw->wifi.W_TXREQ_SET = data;
            break;
        default:
            log_fatal("[ARM7] Undefined 16-bit io write %08x = %08x", addr, data);
        }
        break;
    default:
        log_fatal("[ARM7] Undefined 16-bit write %08x = %08x", addr, data);
    }
}

void ARM7Memory::WriteWord(u32 addr, u32 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        hw->spu.WriteWord(addr, data);
        return;
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return;
    }

    switch (addr >> 24) {
    case REGION_IO:
        switch (addr) {
        case 0x040000B0:
            hw->dma[0].channel[0].source = data;
            break;
        case 0x040000B4:
            hw->dma[0].channel[0].destination = data;
            break;
        case 0x040000B8:
            hw->dma[0].WriteDMACNT(0, data);
            break;
        case 0x040000BC:
            hw->dma[0].channel[1].source = data;
            break;
        case 0x040000C0:
            hw->dma[0].channel[1].destination = data;
            break;
        case 0x040000C4:
            hw->dma[0].WriteDMACNT(1, data);
            break;
        case 0x040000C8:
            hw->dma[0].channel[2].source = data;
            break;
        case 0x040000CC:
            hw->dma[0].channel[2].destination = data;
            break;
        case 0x040000D0:
            hw->dma[0].WriteDMACNT(2, data);
            break;
        case 0x040000D4:
            hw->dma[0].channel[3].source = data;
            break;
        case 0x040000D8:
            hw->dma[0].channel[3].destination = data;
            break;
        case 0x040000DC:
            hw->dma[0].WriteDMACNT(3, data);
            break;
        case 0x04000100:
            hw->timers[0].WriteTMCNT_L(0, data);
            hw->timers[0].WriteTMCNT_H(0, data);
            break;
        case 0x04000104:
            hw->timers[0].WriteTMCNT_L(1, data);
            hw->timers[0].WriteTMCNT_H(1, data);
            break;
        case 0x04000108:
            hw->timers[0].WriteTMCNT_L(2, data);
            hw->timers[0].WriteTMCNT_H(2, data);
            break;
        case 0x04000120:
            // debug SIODATA32
            break;
        case 0x04000128:
            // debug SIOCNT but it doesn't matter
            break;
        case 0x04000180:
            hw->ipc.WriteIPCSYNC7(data);
            break;
        case 0x04000188:
            hw->ipc.WriteFIFOSEND7(data);
            break;
        case 0x040001A4:
            hw->cartridge.WriteROMCTRL(data);
            break;
        case 0x040001B0: case 0x040001B4:
            // TODO: handle key2 encryption later
            break;
        case 0x04000208:
            hw->cpu_core[0]->ime = data & 0x1;
            break;
        case 0x04000210:
            hw->cpu_core[0]->ie = data;
            break;
        case 0x04000214:
            hw->cpu_core[0]->irf &= ~data;
            break;
        case 0x04000308:
            hw->BIOSPROT = data;
            break;
        case 0x04000510:
            hw->spu.SNDCAPDAD[0] = data;
            break;
        case 0x04000518:
            hw->spu.SNDCAPDAD[1] = data;
            break;
        default:
            log_warn("[ARM7] Undefined 32-bit io write %08x = %08x", addr, data);
        }
        break;
    case REGION_VRAM:
        hw->gpu.WriteARM7<u32>(addr, data);
        break;
    case REGION_GBA_ROM_L: case REGION_GBA_ROM_H:
        // for now do nothing lol
        break;
    default:
        log_fatal("[ARM7] Undefined 32-bit write %08x = %08x", addr, data);
    }
}