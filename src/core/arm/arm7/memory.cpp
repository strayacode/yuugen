#include <core/core.h>
#include <core/arm/arm7/memory.h>


ARM7Memory::ARM7Memory(System& system) : system(system) {
    bios = LoadBios<0x4000>("../bios/bios7.bin");
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
            read_page_table[index] = &bios[addr & 0x3FFF];
            write_page_table[index] = nullptr;
            break;
        case 0x02:
            read_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
            write_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
            break;
        case 0x03:
            if (addr < 0x03800000) {
                switch (system.WRAMCNT) {
                case 0:
                    read_page_table[index] = &system.arm7_wram[addr & 0xFFFF];
                    write_page_table[index] = &system.arm7_wram[addr & 0xFFFF];
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
                read_page_table[index] = &system.arm7_wram[addr & 0xFFFF];
                write_page_table[index] = &system.arm7_wram[addr & 0xFFFF];
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
    if (in_range(0x04000400, 0x100)) {
        return system.spu.ReadByte(addr);
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x04000138:
            return system.rtc.ReadRTC();
        case 0x040001C2:
            return system.spi.ReadSPIDATA();
        case 0x04000240:
            return system.gpu.VRAMSTAT;
        case 0x04000241:
            return system.WRAMCNT;
        case 0x04000300:
            return system.POSTFLG7;
        case 0x04000501:
            // read upper byte of SOUNDCNT
            return system.spu.soundcnt >> 8;
        case 0x04000508:
            return system.spu.SNDCAPCNT[0];
        case 0x04000509:
            return system.spu.SNDCAPCNT[1];
        default:
            log_fatal("[ARM7] Undefined 8-bit io read %08x", addr);
        }
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (!(system.EXMEMCNT & (1 << 7))) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFF;
    default:
        log_fatal("[ARM7] Undefined 8-bit read %08x", addr);
    }
}

u16 ARM7Memory::ReadHalf(u32 addr) {
    u16 return_value = 0;

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x04000004:
            return system.gpu.DISPSTAT7;
        case 0x04000006:
            return system.gpu.VCOUNT;
        case 0x040000BA:
            return system.dma[0].ReadDMACNT_H(0);
        case 0x040000C6:
            return system.dma[0].ReadDMACNT_H(1);
        case 0x040000D2:
            return system.dma[0].ReadDMACNT_H(2);
        case 0x040000DE:
            return system.dma[0].ReadDMACNT_H(3);
        case 0x04000100:
            return system.timers[0].ReadTMCNT_L(0);
        case 0x04000104:
            return system.timers[0].ReadTMCNT_L(1);
        case 0x04000108:
            return system.timers[0].ReadTMCNT_L(2);
        case 0x0400010C:
            return system.timers[0].ReadTMCNT_L(3);
        case 0x0400010E:
            return system.timers[0].ReadTMCNT_H(3);
        case 0x04000130:
            return system.input.KEYINPUT;
        case 0x04000134:
            return system.RCNT;
        case 0x04000136:
            return system.input.EXTKEYIN;
        case 0x04000138:
            return system.rtc.RTC_REG;
        case 0x04000180:
            return system.ipc.ReadIPCSYNC7();
        case 0x04000184:
            return system.ipc.IPCFIFOCNT7;
        case 0x040001A0:
            return system.cartridge.AUXSPICNT;
        case 0x040001A2:
            return system.cartridge.AUXSPIDATA;
        case 0x040001C0:
            return system.spi.SPICNT;
        case 0x040001C2:
            return system.spi.ReadSPIDATA();
        case 0x04000204:
            return system.EXMEMCNT;
        case 0x04000208:
            return system.cpu_core[0].ime & 0x1;
        case 0x04000300:
            return system.POSTFLG7;
        case 0x04000304:
            return system.POWCNT2;
        case 0x04000500:
            return system.spu.soundcnt;
        case 0x04000504:
            return system.spu.soundbias;
        case 0x04000508:
            return (system.spu.SNDCAPCNT[0]) | (system.spu.SNDCAPCNT[1] << 8);
        case 0x04004700:
            return 0;
        case 0x04808006:
            return system.wifi.W_MODE_WEP;
        case 0x04808018:
            return system.wifi.W_MACADDR_0;
        case 0x0480801A:
            return system.wifi.W_MACADDR_1;
        case 0x0480801C:
            return system.wifi.W_MACADDR_2;
        case 0x04808020:
            return system.wifi.W_BSSID_0;
        case 0x04808022:
            return system.wifi.W_BSSID_1;
        case 0x04808024:
            return system.wifi.W_BSSID_2;
        case 0x0480802A:
            return system.wifi.W_AID_FULL;
        case 0x04808050:
            return system.wifi.W_RXBUF_BEGIN;
        case 0x04808052:
            return system.wifi.W_RXBUF_END;
        case 0x04808056:
            return system.wifi.W_RXBUF_WR_ADDR;
        default:
            log_fatal("[ARM7] Undefined 16-bit io read %08x", addr);
        }
    case 0x06:
        return_value = system.gpu.ReadARM7<u16>(addr);
        break;
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (!(system.EXMEMCNT & (1 << 7))) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFFFF;
    default:
        log_fatal("[ARM7] Undefined 16-bit io read %08x", addr);
    }

    return return_value;
}

u32 ARM7Memory::ReadWord(u32 addr) {
    if (in_range(0x04000400, 0x100)) {
        return system.spu.ReadWord(addr);
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    u32 return_value = 0;

    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x04000004:
            return system.gpu.VCOUNT;
        case 0x040000B8:
            return system.dma[0].ReadDMACNT(0);
        case 0x040000DC:
            return system.dma[0].ReadDMACNT(3);
        case 0x04000180:
            return system.ipc.ReadIPCSYNC7();
        case 0x040001A4:
            return system.cartridge.ROMCTRL;
        case 0x040001C0:
            return (system.spi.ReadSPIDATA() << 16) | system.spi.SPICNT;
        case 0x04000208:
            return system.cpu_core[0].ime & 0x1;
        case 0x04000210:
            return system.cpu_core[0].ie;
        case 0x04000214:
            return system.cpu_core[0].irf;
        case 0x04004008:
            return 0;
        case 0x04100000:
            return system.ipc.ReadFIFORECV7();
        case 0x04100010:
            return system.cartridge.ReadData();
        default:
            log_fatal("[ARM7] Undefined 32-bit io read %08x", addr);
        }
        break;
    case 0x06:
        return system.gpu.ReadARM7<u32>(addr);
    default:
        log_fatal("handle %08x", addr);
    }

    return return_value;
}

void ARM7Memory::WriteByte(u32 addr, u8 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        system.spu.WriteByte(addr, data);
        return;
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return;
    }

    switch (addr >> 24) {
    case 0x00:
        // ignore all bios writes
        break;
    case 0x04:
        switch (addr) {
        case 0x04000138:
            system.rtc.WriteRTC(data);
            break;
        case 0x040001A0:
            // write to the low byte of AUXSPICNT
            system.cartridge.AUXSPICNT = (system.cartridge.AUXSPICNT & ~0xFF) | (data & 0xFF);
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
        case 0x040001C2:
            system.spi.WriteSPIDATA(data);
            break;
        case 0x04000208:
            system.cpu_core[0].ime = data & 0x1;
            break;
        case 0x04000300:
            system.POSTFLG7 = data;
            break;
        case 0x04000301:
            system.WriteHALTCNT(data);
            break;
        case 0x04000500:
            // write to lower byte of SOUNDCNT
            system.spu.soundcnt = (system.spu.soundcnt & ~0xFF) | (data & 0xFF);
            break;
        case 0x04000501:
            // write to upper byte of SOUNDCNT
            system.spu.soundcnt = (system.spu.soundcnt & 0xFF) | (data << 8);
            break;
        case 0x04000508:
            system.spu.SNDCAPCNT[0] = data;
            break;
        case 0x04000509:
            system.spu.SNDCAPCNT[1] = data;
            break;
        default:
            log_fatal("[ARM7] Undefined 8-bit io write %08x = %08x", addr, data);
        }
        break;
    case 0x06:
        system.gpu.WriteARM7<u8>(addr, data);
        break;
    default:
        log_fatal("[ARM7] Undefined 8-bit write %08x = %08x", addr, data);
    }
}

void ARM7Memory::WriteHalf(u32 addr, u16 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        system.spu.WriteHalf(addr, data);
        return;
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return;
    }

    switch (addr >> 24) {
    case 0x00:
        // ignore all bios writes
        break;
    case 0x04:
        switch (addr) {
        case 0x04000004:
            system.gpu.WriteDISPSTAT7(data);
            break;
        case 0x040000BA:
            system.dma[0].WriteDMACNT_H(0, data);
            break;
        case 0x040000C6:
            system.dma[0].WriteDMACNT_H(1, data);
            break;
        case 0x040000D2:
            system.dma[0].WriteDMACNT_H(2, data);
            break;
        case 0x040000DE:
            system.dma[0].WriteDMACNT_H(3, data);
            break;
        case 0x04000100:
            system.timers[0].WriteTMCNT_L(0, data);
            break;
        case 0x04000102:
            system.timers[0].WriteTMCNT_H(0, data);
            break;
        case 0x04000104:
            system.timers[0].WriteTMCNT_L(1, data);
            break;
        case 0x04000106:
            system.timers[0].WriteTMCNT_H(1, data);
            break;
        case 0x04000108:
            system.timers[0].WriteTMCNT_L(2, data);
            break;
        case 0x0400010A:
            system.timers[0].WriteTMCNT_H(2, data);
            break;
        case 0x0400010C:
            system.timers[0].WriteTMCNT_L(3, data);
            break;
        case 0x0400010E:
            system.timers[0].WriteTMCNT_H(3, data);
            break;
        case 0x04000128:
            // debug SIOCNT
            system.SIOCNT = data;
            break;
        case 0x04000134:
            system.RCNT = data;
            break;
        case 0x04000138:
            system.rtc.WriteRTC(data);
            break;
        case 0x04000180:
            system.ipc.WriteIPCSYNC7(data);
            break;
        case 0x04000184:
            system.ipc.WriteIPCFIFOCNT7(data);
            break;
        case 0x040001A0:
            system.cartridge.WriteAUXSPICNT(data);
            break;
        case 0x040001A2:
            system.cartridge.WriteAUXSPIDATA(data);
            break;
        case 0x040001B8: case 0x040001BA:
            // TODO: handle key2 encryption later
            break;
        case 0x040001C0:
            system.spi.WriteSPICNT(data);
            break;
        case 0x040001C2:
            system.spi.WriteSPIDATA(data);
            break;
        case 0x04000204:
            system.EXMEMCNT = data;
            break;
        case 0x04000206:
            // TODO: implement WIFIWAITCNT
            break;
        case 0x04000208:
            system.cpu_core[0].ime = data & 0x1;
            break;
        case 0x04000304:
            system.POWCNT2 = data & 0x3;
            break;
        case 0x04000500:
            system.spu.soundcnt = data;
            break;
        case 0x04000504:
            system.spu.soundbias = data & 0x3FF;
            break;
        case 0x04000508:
            system.spu.SNDCAPCNT[0] = data & 0xFF;
            system.spu.SNDCAPCNT[1] = data >> 8;
            break;
        case 0x04000514:
            system.spu.SNDCAPLEN[0] = data;
            break;
        case 0x0400051C:
            system.spu.SNDCAPLEN[1] = data;
            break;
        case 0x04001080:
            // this address seems to just be for debugging for the ds lite so not useful for us
            break;
        case 0x04808006:
            system.wifi.W_MODE_WEP = data;
            break;
        case 0x04808018:
            system.wifi.W_MACADDR_0 = data;
            break;
        case 0x0480801A:
            system.wifi.W_MACADDR_1 = data;
            break;
        case 0x0480801C:
            system.wifi.W_MACADDR_2 = data;
            break;
        case 0x04808020:
            system.wifi.W_BSSID_0 = data;
            break;
        case 0x04808022:
            system.wifi.W_BSSID_1 = data;
            break;
        case 0x04808024:
            system.wifi.W_BSSID_2 = data;
            break;
        case 0x0480802A:
            system.wifi.W_AID_FULL = data;
            break;
        case 0x04808050:
            system.wifi.W_RXBUF_BEGIN = data;
            break;
        case 0x04808052:
            system.wifi.W_RXBUF_END = data;
            break;
        case 0x04808056:
            system.wifi.W_RXBUF_WR_ADDR = data;
            break;
        case 0x048080AE:
            system.wifi.W_TXREQ_SET = data;
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
        system.spu.WriteWord(addr, data);
        return;
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return;
    }

    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x040000B0:
            system.dma[0].channel[0].source = data;
            break;
        case 0x040000B4:
            system.dma[0].channel[0].destination = data;
            break;
        case 0x040000B8:
            system.dma[0].WriteDMACNT(0, data);
            break;
        case 0x040000BC:
            system.dma[0].channel[1].source = data;
            break;
        case 0x040000C0:
            system.dma[0].channel[1].destination = data;
            break;
        case 0x040000C4:
            system.dma[0].WriteDMACNT(1, data);
            break;
        case 0x040000C8:
            system.dma[0].channel[2].source = data;
            break;
        case 0x040000CC:
            system.dma[0].channel[2].destination = data;
            break;
        case 0x040000D0:
            system.dma[0].WriteDMACNT(2, data);
            break;
        case 0x040000D4:
            system.dma[0].channel[3].source = data;
            break;
        case 0x040000D8:
            system.dma[0].channel[3].destination = data;
            break;
        case 0x040000DC:
            system.dma[0].WriteDMACNT(3, data);
            break;
        case 0x04000100:
            system.timers[0].WriteTMCNT_L(0, data);
            system.timers[0].WriteTMCNT_H(0, data);
            break;
        case 0x04000104:
            system.timers[0].WriteTMCNT_L(1, data);
            system.timers[0].WriteTMCNT_H(1, data);
            break;
        case 0x04000108:
            system.timers[0].WriteTMCNT_L(2, data);
            system.timers[0].WriteTMCNT_H(2, data);
            break;
        case 0x04000120:
            // debug SIODATA32
            break;
        case 0x04000128:
            // debug SIOCNT but it doesn't matter
            break;
        case 0x04000180:
            system.ipc.WriteIPCSYNC7(data);
            break;
        case 0x04000188:
            system.ipc.WriteFIFOSEND7(data);
            break;
        case 0x040001A4:
            system.cartridge.WriteROMCTRL(data);
            break;
        case 0x040001B0: case 0x040001B4:
            // TODO: handle key2 encryption later
            break;
        case 0x04000208:
            system.cpu_core[0].ime = data & 0x1;
            break;
        case 0x04000210:
            system.cpu_core[0].ie = data;
            break;
        case 0x04000214:
            system.cpu_core[0].irf &= ~data;
            break;
        case 0x04000308:
            system.BIOSPROT = data;
            break;
        case 0x04000510:
            system.spu.SNDCAPDAD[0] = data;
            break;
        case 0x04000518:
            system.spu.SNDCAPDAD[1] = data;
            break;
        default:
            log_warn("[ARM7] Undefined 32-bit io write %08x = %08x", addr, data);
        }
        break;
    case 0x06:
        system.gpu.WriteARM7<u32>(addr, data);
        break;
    case 0x08: case 0x09:
        // for now do nothing lol
        break;
    default:
        log_fatal("[ARM7] Undefined 32-bit write %08x = %08x", addr, data);
    }
}