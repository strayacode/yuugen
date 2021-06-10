#include <core/arm/memory.h>
#include <core/core.h>

auto Memory::ARM7ReadByteIO(u32 addr) -> u8 {
    if (in_range(0x04000400, 0x100)) {
        return core->spu.ReadByte(addr);
    }

    switch (addr) {
    case 0x04000138:
        return core->rtc.ReadRTC();
    case 0x040001C2:
        return core->spi.ReadSPIDATA();
    case 0x04000240:
        return core->gpu.VRAMSTAT;
    case 0x04000241:
        return WRAMCNT;
    case 0x04000300:
        return POSTFLG7;
    case 0x04000501:
        // read upper byte of SOUNDCNT
        return core->spu.SOUNDCNT >> 8;
    case 0x04000508:
        return core->spu.SNDCAPCNT[0];
    case 0x04000509:
        return core->spu.SNDCAPCNT[1];
    default:
        log_fatal("[ARM7] Undefined 8-bit io read %08x", addr);
    }
}

auto Memory::ARM7ReadHalfIO(u32 addr) -> u16 {
    switch (addr) {
    case 0x04000004:
        return core->gpu.DISPSTAT7;
    case 0x04000006:
        return core->gpu.VCOUNT;
    case 0x040000BA:
        return core->dma[0].ReadDMACNT_H(0);
    case 0x040000C6:
        return core->dma[0].ReadDMACNT_H(1);
    case 0x040000D2:
        return core->dma[0].ReadDMACNT_H(2);
    case 0x040000DE:
        return core->dma[0].ReadDMACNT_H(3);
    case 0x04000100:
        return core->timers[0].ReadTMCNT_L(0);
    case 0x04000104:
        return core->timers[0].ReadTMCNT_L(1);
    case 0x04000108:
        return core->timers[0].ReadTMCNT_L(2);
    case 0x0400010C:
        return core->timers[0].ReadTMCNT_L(3);
    case 0x0400010E:
        return core->timers[0].ReadTMCNT_H(3);
    case 0x04000130:
        return core->input.KEYINPUT;
    case 0x04000134:
        return RCNT;
    case 0x04000136:
        return core->input.EXTKEYIN;
    case 0x04000138:
        return core->rtc.RTC_REG;
    case 0x04000180:
        return core->ipc.ReadIPCSYNC7();
    case 0x04000184:
        return core->ipc.IPCFIFOCNT7;
    case 0x040001A0:
        return core->cartridge.AUXSPICNT;
    case 0x040001A2:
        return core->cartridge.AUXSPIDATA;
    case 0x040001C0:
        return core->spi.SPICNT;
    case 0x040001C2:
        return core->spi.ReadSPIDATA();
    case 0x04000204:
        return EXMEMCNT;
    case 0x04000208:
        return core->interrupt[0].IME & 0x1;
    case 0x04000300:
        return POSTFLG7;
    case 0x04000304:
        return POWCNT2;
    case 0x04000500:
        return core->spu.SOUNDCNT;
    case 0x04000504:
        return core->spu.SOUNDBIAS;
    case 0x04000508:
        return (core->spu.SNDCAPCNT[0]) | (core->spu.SNDCAPCNT[1] << 8);
    case 0x04004700:
        return 0;
    case 0x04808006:
        return core->wifi.W_MODE_WEP;
    case 0x04808018:
        return core->wifi.W_MACADDR_0;
    case 0x0480801A:
        return core->wifi.W_MACADDR_1;
    case 0x0480801C:
        return core->wifi.W_MACADDR_2;
    case 0x04808020:
        return core->wifi.W_BSSID_0;
    case 0x04808022:
        return core->wifi.W_BSSID_1;
    case 0x04808024:
        return core->wifi.W_BSSID_2;
    case 0x0480802A:
        return core->wifi.W_AID_FULL;
    case 0x04808050:
        return core->wifi.W_RXBUF_BEGIN;
    case 0x04808052:
        return core->wifi.W_RXBUF_END;
    case 0x04808056:
        return core->wifi.W_RXBUF_WR_ADDR;
    default:
        log_fatal("[ARM7] Undefined 16-bit io read %08x", addr);
    }
}

auto Memory::ARM7ReadWordIO(u32 addr) -> u32 {
    if (in_range(0x04000400, 0x100)) {
        return core->spu.ReadWord(addr);
    }

    switch (addr) {
    case 0x04000004:
        return core->gpu.VCOUNT;
    case 0x040000B8:
        return core->dma[0].ReadDMACNT(0);
    case 0x040000DC:
        return core->dma[0].ReadDMACNT(3);
    case 0x04000180:
        return core->ipc.ReadIPCSYNC7();
    case 0x040001A4:
        return core->cartridge.ROMCTRL;
    case 0x040001C0:
        return (core->spi.ReadSPIDATA() << 16) | core->spi.SPICNT;
    case 0x04000208:
        return core->interrupt[0].IME & 0x1;
    case 0x04000210:
        return core->interrupt[0].IE;
    case 0x04000214:
        return core->interrupt[0].IF;
    case 0x04004008:
        return 0;
    case 0x04100000:
        return core->ipc.ReadFIFORECV7();
    case 0x04100010:
        return core->cartridge.ReadData();
    default:
        log_fatal("[ARM7] Undefined 32-bit io read %08x", addr);
    }
}

void Memory::ARM7WriteByteIO(u32 addr, u8 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        core->spu.WriteByte(addr, data);
        return;
    }

    switch (addr) {
    case 0x04000138:
        core->rtc.WriteRTC(data);
        break;
    case 0x040001A1:
        // write to the high byte of AUXSPICNT
        core->cartridge.AUXSPICNT = (core->cartridge.AUXSPICNT & 0xFF) | (data << 8);
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
        core->cartridge.ReceiveCommand(data, addr - 0x040001A8);
        break;
    case 0x040001C2:
        core->spi.WriteSPIDATA(data);
        break;
    case 0x04000208:
        core->interrupt[0].IME = data & 0x1;
        break;
    case 0x04000300:
        POSTFLG7 = data;
        break;
    case 0x04000301:
        WriteHALTCNT(data);
        break;
    case 0x04000500:
        // write to lower byte of SOUNDCNT
        core->spu.SOUNDCNT = (core->spu.SOUNDCNT & ~0xFF) | (data & 0xFF);
        break;
    case 0x04000501:
        // write to upper byte of SOUNDCNT
        core->spu.SOUNDCNT = (core->spu.SOUNDCNT & 0xFF) | (data << 8);
        break;
    case 0x04000508:
        core->spu.SNDCAPCNT[0] = data;
        break;
    case 0x04000509:
        core->spu.SNDCAPCNT[1] = data;
        break;
    default:
        log_fatal("[ARM7] Undefined 8-bit io write %08x = %08x", addr, data);
    }
}

void Memory::ARM7WriteHalfIO(u32 addr, u16 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        core->spu.WriteHalf(addr, data);
        return;
    }

    switch (addr) {
    case 0x04000004:
        core->gpu.WriteDISPSTAT7(data);
        break;
    case 0x040000BA:
        core->dma[0].WriteDMACNT_H(0, data);
        break;
    case 0x040000C6:
        core->dma[0].WriteDMACNT_H(1, data);
        break;
    case 0x040000D2:
        core->dma[0].WriteDMACNT_H(2, data);
        break;
    case 0x040000DE:
        core->dma[0].WriteDMACNT_H(3, data);
        break;
    case 0x04000100:
        core->timers[0].WriteTMCNT_L(0, data);
        break;
    case 0x04000102:
        core->timers[0].WriteTMCNT_H(0, data);
        break;
    case 0x04000104:
        core->timers[0].WriteTMCNT_L(1, data);
        break;
    case 0x04000106:
        core->timers[0].WriteTMCNT_H(1, data);
        break;
    case 0x04000108:
        core->timers[0].WriteTMCNT_L(2, data);
        break;
    case 0x0400010A:
        core->timers[0].WriteTMCNT_H(2, data);
        break;
    case 0x0400010C:
        core->timers[0].WriteTMCNT_L(3, data);
        break;
    case 0x0400010E:
        core->timers[0].WriteTMCNT_H(3, data);
        break;
    case 0x04000128:
        // debug SIOCNT
        SIOCNT = data;
        break;
    case 0x04000134:
        RCNT = data;
        break;
    case 0x04000138:
        core->rtc.WriteRTC(data);
        break;
    case 0x04000180:
        core->ipc.WriteIPCSYNC7(data);
        break;
    case 0x04000184:
        core->ipc.WriteIPCFIFOCNT7(data);
        break;
    case 0x040001A0:
        core->cartridge.WriteAUXSPICNT(data);
        break;
    case 0x040001A2:
        core->cartridge.WriteAUXSPIDATA(data);
        break;
    case 0x040001B8: case 0x040001BA:
        // TODO: handle key2 encryption later
        break;
    case 0x040001C0:
        core->spi.WriteSPICNT(data);
        break;
    case 0x040001C2:
        core->spi.WriteSPIDATA(data);
        break;
    case 0x04000204:
        EXMEMCNT = data;
        break;
    case 0x04000206:
        // TODO: implement WIFIWAITCNT
        break;
    case 0x04000208:
        core->interrupt[0].IME = data & 0x1;
        break;
    case 0x04000304:
        POWCNT2 = data & 0x3;
        break;
    case 0x04000500:
        core->spu.SOUNDCNT = data;
        break;
    case 0x04000504:
        core->spu.SOUNDBIAS = data;
        break;
    case 0x04000508:
        core->spu.SNDCAPCNT[0] = data & 0xFF;
        core->spu.SNDCAPCNT[1] = data >> 8;
        break;
    case 0x04000514:
        core->spu.SNDCAPLEN[0] = data;
        break;
    case 0x0400051C:
        core->spu.SNDCAPLEN[1] = data;
        break;
    case 0x04001080:
        // this address seems to just be for debugging for the ds lite so not useful for us
        break;
    case 0x04808006:
        core->wifi.W_MODE_WEP = data;
        break;
    case 0x04808018:
        core->wifi.W_MACADDR_0 = data;
        break;
    case 0x0480801A:
        core->wifi.W_MACADDR_1 = data;
        break;
    case 0x0480801C:
        core->wifi.W_MACADDR_2 = data;
        break;
    case 0x04808020:
        core->wifi.W_BSSID_0 = data;
        break;
    case 0x04808022:
        core->wifi.W_BSSID_1 = data;
        break;
    case 0x04808024:
        core->wifi.W_BSSID_2 = data;
        break;
    case 0x0480802A:
        core->wifi.W_AID_FULL = data;
        break;
    case 0x04808050:
        core->wifi.W_RXBUF_BEGIN = data;
        break;
    case 0x04808052:
        core->wifi.W_RXBUF_END = data;
        break;
    case 0x04808056:
        core->wifi.W_RXBUF_WR_ADDR = data;
        break;
    case 0x048080AE:
        core->wifi.W_TXREQ_SET = data;
        break;
    default:
        log_fatal("[ARM7] Undefined 16-bit io write %08x = %08x", addr, data);
    }
}

void Memory::ARM7WriteWordIO(u32 addr, u32 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        core->spu.WriteWord(addr, data);
        return;
    }

    switch (addr) {
    case 0x040000B0:
        core->dma[0].channel[0].source = data;
        break;
    case 0x040000B4:
        core->dma[0].channel[0].destination = data;
        break;
    case 0x040000B8:
        core->dma[0].WriteDMACNT(0, data);
        break;
    case 0x040000BC:
        core->dma[0].channel[1].source = data;
        break;
    case 0x040000C0:
        core->dma[0].channel[1].destination = data;
        break;
    case 0x040000C4:
        core->dma[0].WriteDMACNT(1, data);
        break;
    case 0x040000C8:
        core->dma[0].channel[2].source = data;
        break;
    case 0x040000CC:
        core->dma[0].channel[2].destination = data;
        break;
    case 0x040000D0:
        core->dma[0].WriteDMACNT(2, data);
        break;
    case 0x040000D4:
        core->dma[0].channel[3].source = data;
        break;
    case 0x040000D8:
        core->dma[0].channel[3].destination = data;
        break;
    case 0x040000DC:
        core->dma[0].WriteDMACNT(3, data);
        break;
    case 0x04000100:
        core->timers[0].WriteTMCNT_L(0, data);
        core->timers[0].WriteTMCNT_H(0, data);
        break;
    case 0x04000104:
        core->timers[0].WriteTMCNT_L(1, data);
        core->timers[0].WriteTMCNT_H(1, data);
        break;
    case 0x04000108:
        core->timers[0].WriteTMCNT_L(2, data);
        core->timers[0].WriteTMCNT_H(2, data);
        break;
    case 0x04000120:
        // debug SIODATA32
        break;
    case 0x04000128:
        // debug SIOCNT but it doesn't matter
        break;
    case 0x04000180:
        core->ipc.WriteIPCSYNC7(data);
        break;
    case 0x04000188:
        core->ipc.WriteFIFOSEND7(data);
        break;
    case 0x040001A4:
        core->cartridge.WriteROMCTRL(data);
        break;
    case 0x040001B0: case 0x040001B4:
        // TODO: handle key2 encryption later
        break;
    case 0x04000208:
        core->interrupt[0].IME = data & 0x1;
        break;
    case 0x04000210:
        core->interrupt[0].IE = data;
        break;
    case 0x04000214:
        core->interrupt[0].IF &= ~data;
        break;
    case 0x04000308:
        BIOSPROT = data;
        break;
    case 0x04000510:
        core->spu.SNDCAPDAD[0] = data;
        break;
    case 0x04000518:
        core->spu.SNDCAPDAD[1] = data;
        break;
    default:
        log_fatal("[ARM7] Undefined 32-bit io write %08x = %08x", addr, data);
    }
}