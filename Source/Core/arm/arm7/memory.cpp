#include "Common/Memory.h"
#include "Core/Core.h"
#include "Core/arm/arm7/memory.h"

ARM7Memory::ARM7Memory(System& system) : system(system) {
    bios = LoadBios<0x4000>("../bios/bios7.bin");
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
        switch (addr) {
        case 0x04000138:
            return system.rtc.ReadRTC();
        case 0x040001C2:
            return system.spi.ReadSPIDATA();
        case 0x04000240:
            return system.video_unit.vram.vramstat;
        case 0x04000241:
            return system.wramcnt;
        case 0x04000300:
            return system.POSTFLG7;
        case 0x04000501:
            // read upper byte of SOUNDCNT
            return system.spu.soundcnt >> 8;
        case 0x04000508:
            return system.spu.sndcapcnt[0];
        case 0x04000509:
            return system.spu.sndcapcnt[1];
        }

        break;
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (!(system.EXMEMCNT & (1 << 7))) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFF;
    }

    if (Common::in_range(0x04000400, 0x04000500, addr)) {
        return system.spu.read_byte(addr);
    }

    log_warn("ARM7: handle byte read %08x", addr);

    return 0;
}

u16 ARM7Memory::ReadHalf(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x04000004:
            return system.video_unit.dispstat[0];
        case 0x04000006:
            return system.video_unit.vcount;
        case 0x040000BA:
            return system.dma[0].ReadDMACNT_H(0);
        case 0x040000C6:
            return system.dma[0].ReadDMACNT_H(1);
        case 0x040000D2:
            return system.dma[0].ReadDMACNT_H(2);
        case 0x040000DE:
            return system.dma[0].ReadDMACNT_H(3);
        case 0x04000100:
            return system.timers[0].read_counter(0);
        case 0x04000104:
            return system.timers[0].read_counter(1);
        case 0x04000108:
            return system.timers[0].read_counter(2);
        case 0x0400010C:
            return system.timers[0].read_counter(3);
        case 0x0400010E:
            return system.timers[0].read_control(3);
        case 0x04000130:
            return system.input.KEYINPUT;
        case 0x04000134:
            return system.RCNT;
        case 0x04000136:
            return system.input.EXTKEYIN;
        case 0x04000138:
            return system.rtc.RTC_REG;
        case 0x04000180:
            return system.ipc.ipcsync[0].data;
        case 0x04000184:
            return system.ipc.ipcfifocnt[0].data;
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
            return (system.spu.sndcapcnt[0]) | (system.spu.sndcapcnt[1] << 8);
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
        }

        break;
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

    log_warn("ARM7: handle half read %08x", addr);

    return 0;
}

u32 ARM7Memory::ReadWord(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x040000B8:
            return system.dma[0].ReadDMACNT(0);
        case 0x040000DC:
            return system.dma[0].ReadDMACNT(3);
        case 0x04000180:
            return system.ipc.ipcsync[0].data;
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
            return system.ipc.read_ipcfiforecv(0);
        case 0x04100010:
            return system.cartridge.ReadData();
        }

        break;
    case 0x06:
        return system.video_unit.vram.read_arm7<u32>(addr);
    }

    if (Common::in_range(0x04000400, 0x04000500, addr)) {
        return system.spu.read_word(addr);
    }

    log_warn("ARM7: handle word read %08x", addr);

    return 0;
}

void ARM7Memory::WriteByte(u32 addr, u8 data) {
    switch (addr >> 24) {
    case 0x00:
        // ignore all bios writes
        return;
    case 0x04:
        switch (addr) {
        case 0x04000138:
            system.rtc.WriteRTC(data);
            return;
        case 0x040001A0:
            // write to the low byte of AUXSPICNT
            system.cartridge.AUXSPICNT = (system.cartridge.AUXSPICNT & ~0xFF) | (data & 0xFF);
            return;
        case 0x040001A1:
            // write to the high byte of AUXSPICNT
            system.cartridge.AUXSPICNT = (system.cartridge.AUXSPICNT & 0xFF) | (data << 8);
            return;
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
            return;
        case 0x040001C2:
            system.spi.WriteSPIDATA(data);
            return;
        case 0x04000208:
            system.cpu_core[0].ime = data & 0x1;
            return;
        case 0x04000300:
            system.POSTFLG7 = data;
            return;
        case 0x04000301:
            system.WriteHALTCNT(data);
            return;
        case 0x04000500:
            // write to lower byte of SOUNDCNT
            system.spu.soundcnt = (system.spu.soundcnt & ~0xFF) | (data & 0xFF);
            return;
        case 0x04000501:
            // write to upper byte of SOUNDCNT
            system.spu.soundcnt = (system.spu.soundcnt & 0xFF) | (data << 8);
            return;
        case 0x04000508:
            system.spu.sndcapcnt[0] = data;
            return;
        case 0x04000509:
            system.spu.sndcapcnt[1] = data;
            return;
        }

        break;
    case 0x06:
        system.video_unit.vram.write_arm7<u8>(addr, data);
        return;
    }

    if (Common::in_range(0x04000400, 0x04000500, addr)) {
        system.spu.write_byte(addr, data);
        return;
    }

    log_warn("ARM7: handle byte write %08x = %02x", addr, data);
}

void ARM7Memory::WriteHalf(u32 addr, u16 data) {
    switch (addr >> 24) {
    case 0x00:
        // ignore all bios writes
        break;
    case 0x04:
        switch (addr) {
        case 0x04000004:
            system.video_unit.dispstat[0] = data;
            return;
        case 0x040000BA:
            system.dma[0].WriteDMACNT_H(0, data);
            return;
        case 0x040000C6:
            system.dma[0].WriteDMACNT_H(1, data);
            return;
        case 0x040000D2:
            system.dma[0].WriteDMACNT_H(2, data);
            return;
        case 0x040000DE:
            system.dma[0].WriteDMACNT_H(3, data);
            return;
        case 0x04000100:
            system.timers[0].write_counter(0, data);
            return;
        case 0x04000102:
            system.timers[0].write_control(0, data);
            return;
        case 0x04000104:
            system.timers[0].write_counter(1, data);
            return;
        case 0x04000106:
            system.timers[0].write_control(1, data);
            return;
        case 0x04000108:
            system.timers[0].write_counter(2, data);
            return;
        case 0x0400010A:
            system.timers[0].write_control(2, data);
            return;
        case 0x0400010C:
            system.timers[0].write_counter(3, data);
            return;
        case 0x0400010E:
            system.timers[0].write_control(3, data);
            return;
        case 0x04000128:
            // debug SIOCNT
            system.SIOCNT = data;
            return;
        case 0x04000134:
            system.RCNT = data;
            return;
        case 0x04000138:
            system.rtc.WriteRTC(data);
            return;
        case 0x04000180:
            system.ipc.write_ipcsync(0, data);
            return;
        case 0x04000184:
            system.ipc.write_ipcfifocnt(0, data);
            return;
        case 0x040001A0:
            system.cartridge.WriteAUXSPICNT(data);
            return;
        case 0x040001A2:
            system.cartridge.WriteAUXSPIDATA(data);
            return;
        case 0x040001B8: case 0x040001BA:
            // TODO: handle key2 encryption later
            return;
        case 0x040001C0:
            system.spi.WriteSPICNT(data);
            return;
        case 0x040001C2:
            system.spi.WriteSPIDATA(data);
            return;
        case 0x04000204:
            system.EXMEMCNT = data;
            return;
        case 0x04000206:
            // TODO: implement WIFIWAITCNT
            return;
        case 0x04000208:
            system.cpu_core[0].ime = data & 0x1;
            return;
        case 0x04000304:
            system.POWCNT2 = data & 0x3;
            return;
        case 0x04000500:
            system.spu.soundcnt = data;
            return;
        case 0x04000504:
            system.spu.soundbias = data & 0x3FF;
            return;
        case 0x04000508:
            system.spu.sndcapcnt[0] = data & 0xFF;
            system.spu.sndcapcnt[1] = data >> 8;
            return;
        case 0x04000514:
            system.spu.sndcaplen[0] = data;
            return;
        case 0x0400051C:
            system.spu.sndcaplen[1] = data;
            return;
        case 0x04001080:
            // this address seems to just be for debugging for the ds lite so not useful for us
            return;
        }
        break;
    }

    if (Common::in_range(0x04000400, 0x04000500, addr)) {
        system.spu.write_half(addr, data);
        return;
    }

    if (Common::in_range(0x04800000, 0x04900000, addr)) {
        // TODO: implement wifi regs correctly
        return;
    }

    log_warn("ARM7: handle half write %08x = %04x", addr, data);
}

void ARM7Memory::WriteWord(u32 addr, u32 data) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x040000B0:
            system.dma[0].channel[0].source = data;
            return;
        case 0x040000B4:
            system.dma[0].channel[0].destination = data;
            return;
        case 0x040000B8:
            system.dma[0].WriteDMACNT(0, data);
            return;
        case 0x040000BC:
            system.dma[0].channel[1].source = data;
            return;
        case 0x040000C0:
            system.dma[0].channel[1].destination = data;
            return;
        case 0x040000C4:
            system.dma[0].WriteDMACNT(1, data);
            return;
        case 0x040000C8:
            system.dma[0].channel[2].source = data;
            return;
        case 0x040000CC:
            system.dma[0].channel[2].destination = data;
            return;
        case 0x040000D0:
            system.dma[0].WriteDMACNT(2, data);
            return;
        case 0x040000D4:
            system.dma[0].channel[3].source = data;
            return;
        case 0x040000D8:
            system.dma[0].channel[3].destination = data;
            return;
        case 0x040000DC:
            system.dma[0].WriteDMACNT(3, data);
            return;
        case 0x04000100:
            system.timers[0].write_counter(0, data);
            system.timers[0].write_control(0, data);
            return;
        case 0x04000104:
            system.timers[0].write_counter(1, data);
            system.timers[0].write_control(1, data);
            return;
        case 0x04000108:
            system.timers[0].write_counter(2, data);
            system.timers[0].write_control(2, data);
            return;
        case 0x04000120:
            // debug SIODATA32
            return;
        case 0x04000128:
            // debug SIOCNT but it doesn't matter
            return;
        case 0x04000180:
            system.ipc.write_ipcsync(0, data);
            return;
        case 0x04000188:
            system.ipc.write_send_fifo(0, data);
            return;
        case 0x040001A4:
            system.cartridge.WriteROMCTRL(data);
            return;
        case 0x040001B0: case 0x040001B4:
            // TODO: handle key2 encryption later
            return;
        case 0x04000208:
            system.cpu_core[0].ime = data & 0x1;
            return;
        case 0x04000210:
            system.cpu_core[0].ie = data;
            return;
        case 0x04000214:
            system.cpu_core[0].irf &= ~data;
            return;
        case 0x04000308:
            system.BIOSPROT = data;
            return;
        case 0x04000510:
            system.spu.sndcapdad[0] = data;
            return;
        case 0x04000518:
            system.spu.sndcapdad[1] = data;
            return;
        }

        break;
    case 0x06:
        system.video_unit.vram.write_arm7<u32>(addr, data);
        return;
    case 0x08: case 0x09:
        // for now do nothing lol
        return;
    }

    if (Common::in_range(0x04000400, 0x04000500, addr)) {
        system.spu.write_word(addr, data);
        return;
    }

    log_warn("ARM7: handle word write %08x = %08x", addr, data);
}