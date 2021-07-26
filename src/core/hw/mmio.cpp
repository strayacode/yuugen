#include <core/hw/hw.h>

auto HW::ARM7ReadByteIO(u32 addr) -> u8 {
    if (in_range(0x04000400, 0x100)) {
        return spu.ReadByte(addr);
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    switch (addr) {
    case 0x04000138:
        return rtc.ReadRTC();
    case 0x040001C2:
        return spi.ReadSPIDATA();
    case 0x04000240:
        return gpu.VRAMSTAT;
    case 0x04000241:
        return WRAMCNT;
    case 0x04000300:
        return POSTFLG7;
    case 0x04000501:
        // read upper byte of SOUNDCNT
        return spu.SOUNDCNT >> 8;
    case 0x04000508:
        return spu.SNDCAPCNT[0];
    case 0x04000509:
        return spu.SNDCAPCNT[1];
    default:
        log_fatal("[ARM7] Undefined 8-bit io read %08x", addr);
    }
}

auto HW::ARM7ReadHalfIO(u32 addr) -> u16 {
    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    switch (addr) {
    case 0x04000004:
        return gpu.DISPSTAT7;
    case 0x04000006:
        return gpu.VCOUNT;
    case 0x040000BA:
        return dma[0].ReadDMACNT_H(0);
    case 0x040000C6:
        return dma[0].ReadDMACNT_H(1);
    case 0x040000D2:
        return dma[0].ReadDMACNT_H(2);
    case 0x040000DE:
        return dma[0].ReadDMACNT_H(3);
    case 0x04000100:
        return timers[0].ReadTMCNT_L(0);
    case 0x04000104:
        return timers[0].ReadTMCNT_L(1);
    case 0x04000108:
        return timers[0].ReadTMCNT_L(2);
    case 0x0400010C:
        return timers[0].ReadTMCNT_L(3);
    case 0x0400010E:
        return timers[0].ReadTMCNT_H(3);
    case 0x04000130:
        return input.KEYINPUT;
    case 0x04000134:
        return RCNT;
    case 0x04000136:
        return input.EXTKEYIN;
    case 0x04000138:
        return rtc.RTC_REG;
    case 0x04000180:
        return ipc.ReadIPCSYNC7();
    case 0x04000184:
        return ipc.IPCFIFOCNT7;
    case 0x040001A0:
        return cartridge.AUXSPICNT;
    case 0x040001A2:
        return cartridge.AUXSPIDATA;
    case 0x040001C0:
        return spi.SPICNT;
    case 0x040001C2:
        return spi.ReadSPIDATA();
    case 0x04000204:
        return EXMEMCNT;
    case 0x04000208:
        return interrupt[0].IME & 0x1;
    case 0x04000300:
        return POSTFLG7;
    case 0x04000304:
        return POWCNT2;
    case 0x04000500:
        return spu.SOUNDCNT;
    case 0x04000504:
        return spu.SOUNDBIAS;
    case 0x04000508:
        return (spu.SNDCAPCNT[0]) | (spu.SNDCAPCNT[1] << 8);
    case 0x04004700:
        return 0;
    case 0x04808006:
        return wifi.W_MODE_WEP;
    case 0x04808018:
        return wifi.W_MACADDR_0;
    case 0x0480801A:
        return wifi.W_MACADDR_1;
    case 0x0480801C:
        return wifi.W_MACADDR_2;
    case 0x04808020:
        return wifi.W_BSSID_0;
    case 0x04808022:
        return wifi.W_BSSID_1;
    case 0x04808024:
        return wifi.W_BSSID_2;
    case 0x0480802A:
        return wifi.W_AID_FULL;
    case 0x04808050:
        return wifi.W_RXBUF_BEGIN;
    case 0x04808052:
        return wifi.W_RXBUF_END;
    case 0x04808056:
        return wifi.W_RXBUF_WR_ADDR;
    default:
        log_fatal("[ARM7] Undefined 16-bit io read %08x", addr);
    }
}

auto HW::ARM7ReadWordIO(u32 addr) -> u32 {
    if (in_range(0x04000400, 0x100)) {
        return spu.ReadWord(addr);
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return 0;
    }

    switch (addr) {
    case 0x04000004:
        return gpu.VCOUNT;
    case 0x040000B8:
        return dma[0].ReadDMACNT(0);
    case 0x040000DC:
        return dma[0].ReadDMACNT(3);
    case 0x04000180:
        return ipc.ReadIPCSYNC7();
    case 0x040001A4:
        return cartridge.ROMCTRL;
    case 0x040001C0:
        return (spi.ReadSPIDATA() << 16) | spi.SPICNT;
    case 0x04000208:
        return interrupt[0].IME & 0x1;
    case 0x04000210:
        return interrupt[0].IE;
    case 0x04000214:
        return interrupt[0].IF;
    case 0x04004008:
        return 0;
    case 0x04100000:
        return ipc.ReadFIFORECV7();
    case 0x04100010:
        return cartridge.ReadData();
    default:
        log_fatal("[ARM7] Undefined 32-bit io read %08x", addr);
    }
}

void HW::ARM7WriteByteIO(u32 addr, u8 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        spu.WriteByte(addr, data);
        return;
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return;
    }

    switch (addr) {
    case 0x04000138:
        rtc.WriteRTC(data);
        break;
    case 0x040001A1:
        // write to the high byte of AUXSPICNT
        cartridge.AUXSPICNT = (cartridge.AUXSPICNT & 0xFF) | (data << 8);
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
        cartridge.ReceiveCommand(data, addr - 0x040001A8);
        break;
    case 0x040001C2:
        spi.WriteSPIDATA(data);
        break;
    case 0x04000208:
        interrupt[0].IME = data & 0x1;
        break;
    case 0x04000300:
        POSTFLG7 = data;
        break;
    case 0x04000301:
        WriteHALTCNT(data);
        break;
    case 0x04000500:
        // write to lower byte of SOUNDCNT
        spu.SOUNDCNT = (spu.SOUNDCNT & ~0xFF) | (data & 0xFF);
        break;
    case 0x04000501:
        // write to upper byte of SOUNDCNT
        spu.SOUNDCNT = (spu.SOUNDCNT & 0xFF) | (data << 8);
        break;
    case 0x04000508:
        spu.SNDCAPCNT[0] = data;
        break;
    case 0x04000509:
        spu.SNDCAPCNT[1] = data;
        break;
    default:
        log_fatal("[ARM7] Undefined 8-bit io write %08x = %08x", addr, data);
    }
}

void HW::ARM7WriteHalfIO(u32 addr, u16 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        spu.WriteHalf(addr, data);
        return;
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return;
    }

    switch (addr) {
    case 0x04000004:
        gpu.WriteDISPSTAT7(data);
        break;
    case 0x040000BA:
        dma[0].WriteDMACNT_H(0, data);
        break;
    case 0x040000C6:
        dma[0].WriteDMACNT_H(1, data);
        break;
    case 0x040000D2:
        dma[0].WriteDMACNT_H(2, data);
        break;
    case 0x040000DE:
        dma[0].WriteDMACNT_H(3, data);
        break;
    case 0x04000100:
        timers[0].WriteTMCNT_L(0, data);
        break;
    case 0x04000102:
        timers[0].WriteTMCNT_H(0, data);
        break;
    case 0x04000104:
        timers[0].WriteTMCNT_L(1, data);
        break;
    case 0x04000106:
        timers[0].WriteTMCNT_H(1, data);
        break;
    case 0x04000108:
        timers[0].WriteTMCNT_L(2, data);
        break;
    case 0x0400010A:
        timers[0].WriteTMCNT_H(2, data);
        break;
    case 0x0400010C:
        timers[0].WriteTMCNT_L(3, data);
        break;
    case 0x0400010E:
        timers[0].WriteTMCNT_H(3, data);
        break;
    case 0x04000128:
        // debug SIOCNT
        SIOCNT = data;
        break;
    case 0x04000134:
        RCNT = data;
        break;
    case 0x04000138:
        rtc.WriteRTC(data);
        break;
    case 0x04000180:
        ipc.WriteIPCSYNC7(data);
        break;
    case 0x04000184:
        ipc.WriteIPCFIFOCNT7(data);
        break;
    case 0x040001A0:
        cartridge.WriteAUXSPICNT(data);
        break;
    case 0x040001A2:
        cartridge.WriteAUXSPIDATA(data);
        break;
    case 0x040001B8: case 0x040001BA:
        // TODO: handle key2 encryption later
        break;
    case 0x040001C0:
        spi.WriteSPICNT(data);
        break;
    case 0x040001C2:
        spi.WriteSPIDATA(data);
        break;
    case 0x04000204:
        EXMEMCNT = data;
        break;
    case 0x04000206:
        // TODO: implement WIFIWAITCNT
        break;
    case 0x04000208:
        interrupt[0].IME = data & 0x1;
        break;
    case 0x04000304:
        POWCNT2 = data & 0x3;
        break;
    case 0x04000500:
        spu.SOUNDCNT = data;
        break;
    case 0x04000504:
        spu.SOUNDBIAS = data;
        break;
    case 0x04000508:
        spu.SNDCAPCNT[0] = data & 0xFF;
        spu.SNDCAPCNT[1] = data >> 8;
        break;
    case 0x04000514:
        spu.SNDCAPLEN[0] = data;
        break;
    case 0x0400051C:
        spu.SNDCAPLEN[1] = data;
        break;
    case 0x04001080:
        // this address seems to just be for debugging for the ds lite so not useful for us
        break;
    case 0x04808006:
        wifi.W_MODE_WEP = data;
        break;
    case 0x04808018:
        wifi.W_MACADDR_0 = data;
        break;
    case 0x0480801A:
        wifi.W_MACADDR_1 = data;
        break;
    case 0x0480801C:
        wifi.W_MACADDR_2 = data;
        break;
    case 0x04808020:
        wifi.W_BSSID_0 = data;
        break;
    case 0x04808022:
        wifi.W_BSSID_1 = data;
        break;
    case 0x04808024:
        wifi.W_BSSID_2 = data;
        break;
    case 0x0480802A:
        wifi.W_AID_FULL = data;
        break;
    case 0x04808050:
        wifi.W_RXBUF_BEGIN = data;
        break;
    case 0x04808052:
        wifi.W_RXBUF_END = data;
        break;
    case 0x04808056:
        wifi.W_RXBUF_WR_ADDR = data;
        break;
    case 0x048080AE:
        wifi.W_TXREQ_SET = data;
        break;
    default:
        log_fatal("[ARM7] Undefined 16-bit io write %08x = %08x", addr, data);
    }
}

void HW::ARM7WriteWordIO(u32 addr, u32 data) {
    if (in_range(0x04000400, 0x100)) {
        // write to an spu channel
        spu.WriteWord(addr, data);
        return;
    }

    if (in_range(0x04800000, 0x100000)) {
        // TODO: implement wifi regs correctly
        return;
    }

    switch (addr) {
    case 0x040000B0:
        dma[0].channel[0].source = data;
        break;
    case 0x040000B4:
        dma[0].channel[0].destination = data;
        break;
    case 0x040000B8:
        dma[0].WriteDMACNT(0, data);
        break;
    case 0x040000BC:
        dma[0].channel[1].source = data;
        break;
    case 0x040000C0:
        dma[0].channel[1].destination = data;
        break;
    case 0x040000C4:
        dma[0].WriteDMACNT(1, data);
        break;
    case 0x040000C8:
        dma[0].channel[2].source = data;
        break;
    case 0x040000CC:
        dma[0].channel[2].destination = data;
        break;
    case 0x040000D0:
        dma[0].WriteDMACNT(2, data);
        break;
    case 0x040000D4:
        dma[0].channel[3].source = data;
        break;
    case 0x040000D8:
        dma[0].channel[3].destination = data;
        break;
    case 0x040000DC:
        dma[0].WriteDMACNT(3, data);
        break;
    case 0x04000100:
        timers[0].WriteTMCNT_L(0, data);
        timers[0].WriteTMCNT_H(0, data);
        break;
    case 0x04000104:
        timers[0].WriteTMCNT_L(1, data);
        timers[0].WriteTMCNT_H(1, data);
        break;
    case 0x04000108:
        timers[0].WriteTMCNT_L(2, data);
        timers[0].WriteTMCNT_H(2, data);
        break;
    case 0x04000120:
        // debug SIODATA32
        break;
    case 0x04000128:
        // debug SIOCNT but it doesn't matter
        break;
    case 0x04000180:
        ipc.WriteIPCSYNC7(data);
        break;
    case 0x04000188:
        ipc.WriteFIFOSEND7(data);
        break;
    case 0x040001A4:
        cartridge.WriteROMCTRL(data);
        break;
    case 0x040001B0: case 0x040001B4:
        // TODO: handle key2 encryption later
        break;
    case 0x04000208:
        interrupt[0].IME = data & 0x1;
        break;
    case 0x04000210:
        interrupt[0].IE = data;
        break;
    case 0x04000214:
        interrupt[0].IF &= ~data;
        break;
    case 0x04000308:
        BIOSPROT = data;
        break;
    case 0x04000510:
        spu.SNDCAPDAD[0] = data;
        break;
    case 0x04000518:
        spu.SNDCAPDAD[1] = data;
        break;
    default:
        log_fatal("[ARM7] Undefined 32-bit io write %08x = %08x", addr, data);
    }
}

auto HW::ARM9ReadByteIO(u32 addr) -> u8 {
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
        return cartridge.ReadCommand(addr - 0x040001A8);
    case 0x04000208:
        return interrupt[1].IME & 0x1;
    case 0x04000300:
        return POSTFLG9;
    case 0x04004000:
        return 0;
    default:
        log_fatal("[ARM9] Undefined 8-bit io read %08x", addr);
    }
}

auto HW::ARM9ReadHalfIO(u32 addr) -> u16 {
    switch (addr) {
    case 0x04000000:
        return gpu.engine_a.DISPCNT & 0xFFFF;
    case 0x04000004:
        return gpu.DISPSTAT9;
    case 0x04000006:
        return gpu.VCOUNT;
    case 0x04000008:
        return gpu.engine_a.BGCNT[0];
    case 0x0400000A:
        return gpu.engine_a.BGCNT[1];
    case 0x0400000C:
        return gpu.engine_a.BGCNT[2];
    case 0x0400000E:
        return gpu.engine_a.BGCNT[3];
    case 0x04000048:
        return gpu.engine_a.WININ;
    case 0x0400004A:
        return gpu.engine_a.WINOUT;
    case 0x04000050:
        return gpu.engine_a.BLDCNT;
    case 0x04000060:
        return gpu.render_engine.DISP3DCNT;
    case 0x0400006C:
        return gpu.engine_a.MASTER_BRIGHT;
    case 0x040000BA:
        return dma[1].ReadDMACNT_H(0);
    case 0x040000C6:
        return dma[1].ReadDMACNT_H(1);
    case 0x040000D2:
        return dma[1].ReadDMACNT_H(2);
    case 0x040000DE:
        return dma[1].ReadDMACNT_H(3);
    case 0x040000EC:
        return dma[1].DMAFILL[3] & 0xFFFF;
    case 0x04000100:
        return timers[1].ReadTMCNT_L(0);
    case 0x04000104:
        return timers[1].ReadTMCNT_L(1);
    case 0x04000108:
        return timers[1].ReadTMCNT_L(2);
    case 0x0400010C:
        return timers[1].ReadTMCNT_L(3);
    case 0x04000130:
        return input.KEYINPUT;
    case 0x04000180:
        return ipc.ReadIPCSYNC9();
    case 0x04000184:
        return ipc.IPCFIFOCNT9;
    case 0x040001A0:
        return cartridge.AUXSPICNT;
    case 0x04000204:
        return EXMEMCNT;
    case 0x04000208:
        return interrupt[1].IME & 0x1;
    case 0x04000280:
        return maths_unit.DIVCNT;
    case 0x040002B0:
        return maths_unit.SQRTCNT;
    case 0x04000300:
        return POSTFLG9;
    case 0x04000304:
        return gpu.POWCNT1;
    case 0x04001000:
        return gpu.engine_b.DISPCNT & 0xFFFF;
    case 0x04001008:
        return gpu.engine_b.BGCNT[0];
    case 0x0400100A:
        return gpu.engine_b.BGCNT[1];
    case 0x0400100C:
        return gpu.engine_b.BGCNT[2];
    case 0x0400100E:
        return gpu.engine_b.BGCNT[3];
    case 0x04001048:
        return gpu.engine_b.WININ;
    case 0x0400104A:
        return gpu.engine_b.WINOUT;
    case 0x0400106C:
        return gpu.engine_b.MASTER_BRIGHT;
    case 0x04004004:
        return 0;
    case 0x04004010:
        return 0;
    default:
        log_fatal("[ARM9] Undefined 16-bit io read %08x", addr);
    }
}

auto HW::ARM9ReadWordIO(u32 addr) -> u32 {
    if (in_range(0x04000640, 0x40)) {
        return gpu.geometry_engine.clip_current.field[(addr - 0x04000640) / 4][(addr - 0x04000640) % 4];
    }

    if (in_range(0x04000680, 0x24)) {
        return gpu.geometry_engine.directional_current.field[(addr - 0x04000680) / 3][(addr - 0x04000680) % 3];
    }

    switch (addr) {
    case 0x04000000:
        return gpu.engine_a.DISPCNT;
    case 0x04000004:
        return (gpu.VCOUNT << 16) | (gpu.DISPSTAT9);
    case 0x04000008:
        return (gpu.engine_a.BGCNT[1] << 16) | (gpu.engine_a.BGCNT[0]);
    case 0x0400000C:
        return (gpu.engine_a.BGCNT[3] << 16) | (gpu.engine_a.BGCNT[2]);
    case 0x04000048:
        return (gpu.engine_a.WINOUT << 16) | gpu.engine_a.WININ;
    case 0x0400004C:
        return 0;
    case 0x04000050:
        return (gpu.engine_a.BLDALPHA << 16) | (gpu.engine_a.BLDCNT);
    case 0x04000054:
        return gpu.engine_a.BLDY;
    case 0x04000058:
        return 0;
    case 0x0400005C:
        return 0;
    case 0x04000060:
        return gpu.render_engine.DISP3DCNT;
    case 0x04000064:
        return 0;
    case 0x04000068:
        return 0;
    case 0x0400006C:
        return 0;
    case 0x040000B0:
        return dma[1].channel[0].source;
    case 0x040000B4:
        return dma[1].channel[0].destination;
    case 0x040000B8:
        return dma[1].ReadDMACNT(0);
    case 0x040000BC:
        return dma[1].channel[1].source;
    case 0x040000C0:
        return dma[1].channel[1].destination;
    case 0x040000C4:
        return dma[1].ReadDMACNT(1);
    case 0x040000C8:
        return dma[1].channel[2].source;
    case 0x040000CC:
        return dma[1].channel[2].destination;
    case 0x040000D0:
        return dma[1].ReadDMACNT(2);
    case 0x040000D4:
        return dma[1].channel[3].source;
    case 0x040000D8:
        return dma[1].channel[3].destination;
    case 0x040000DC:
        return dma[1].ReadDMACNT(3);
    case 0x040000E0:
        return dma[1].DMAFILL[0];
    case 0x040000E4:
        return dma[1].DMAFILL[1];
    case 0x040000E8:
        return dma[1].DMAFILL[2];
    case 0x040000EC:
        return dma[1].DMAFILL[3];
    case 0x04000100:
        return timers[1].ReadTMCNT(0);
    case 0x04000180:
        return ipc.ReadIPCSYNC9();
    case 0x040001A4:
        return cartridge.ROMCTRL;
    case 0x04000208:
        return interrupt[1].IME & 0x1;
    case 0x04000210:
        return interrupt[1].IE;
    case 0x04000214:
        return interrupt[1].IF;
    case 0x04000240:
        return ((gpu.VRAMCNT_D << 24) | (gpu.VRAMCNT_C << 16) | (gpu.VRAMCNT_B << 8) | (gpu.VRAMCNT_A));
    case 0x04000280:
        return maths_unit.DIVCNT;
    case 0x04000290:
        return maths_unit.DIV_NUMER & 0xFFFFFFFF;
    case 0x04000294:
        return maths_unit.DIV_NUMER >> 32;
    case 0x04000298:
        return maths_unit.DIV_DENOM & 0xFFFFFFFF;
    case 0x0400029C:
        return maths_unit.DIV_DENOM >> 32;
    case 0x040002A0:
        return maths_unit.DIV_RESULT & 0xFFFFFFFF;
    case 0x040002A4:
        return maths_unit.DIV_RESULT >> 32;
    case 0x040002A8:
        return maths_unit.DIVREM_RESULT & 0xFFFFFFFF;
    case 0x040002AC:
        return maths_unit.DIVREM_RESULT >> 32;
    case 0x040002B4:
        return maths_unit.SQRT_RESULT;
    case 0x040002B8:
        return maths_unit.SQRT_PARAM & 0xFFFFFFFF;
    case 0x040002BC:
        return maths_unit.SQRT_PARAM >> 32;
    case 0x04000600:
        return gpu.geometry_engine.ReadGXSTAT();
    case 0x04001000:
        return gpu.engine_b.DISPCNT;
    case 0x04004000:
        return 0;
    case 0x04004008:
        return 0;
    case 0x04100000:
        return ipc.ReadFIFORECV9();
    case 0x04100010:
        return cartridge.ReadData();
    default:
        log_fatal("[ARM9] Undefined 32-bit io read %08x", addr);
    }
}

void HW::ARM9WriteByteIO(u32 addr, u8 data) {
    switch (addr) {
    case 0x04000040:
        gpu.engine_a.WINH[0] = (gpu.engine_a.WINH[0] & ~0xFF) | data;
        break;
    case 0x04000041:
        gpu.engine_a.WINH[0] = (gpu.engine_a.WINH[0] & 0xFF) | (data << 8);
        break;
    case 0x04000044:
        gpu.engine_a.WINV[0] = (gpu.engine_a.WINV[0] & ~0xFF) | data;
        break;
    case 0x04000045:
        gpu.engine_a.WINV[0] = (gpu.engine_a.WINV[0] & 0xFF) | (data << 8);
        break;
    case 0x0400004C:
        gpu.engine_a.MOSAIC = (gpu.engine_a.MOSAIC & ~0xFF) | data;
        break;
    case 0x0400004D:
        gpu.engine_a.MOSAIC = (gpu.engine_a.MOSAIC & ~0xFF00) | (data << 8);
        break;
    case 0x0400004E:
        gpu.engine_a.MOSAIC = (gpu.engine_a.MOSAIC & ~0xFF0000) | (data << 16);
        break;
    case 0x0400004F:
        gpu.engine_a.MOSAIC = (gpu.engine_a.MOSAIC & ~0xFF000000) | (data << 24);
        break;
    case 0x040001A1:
        // write to the high byte of AUXSPICNT
        cartridge.AUXSPICNT = (cartridge.AUXSPICNT & 0xFF) | (data << 8);
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
        cartridge.ReceiveCommand(data, addr - 0x040001A8);
        break;
    case 0x04000208:
        interrupt[1].IME = data & 0x1;
        break;
    case 0x04000240:
        gpu.VRAMCNT_A = data;
        gpu.MapVRAM();
        break;
    case 0x04000241:
        gpu.VRAMCNT_B = data;
        gpu.MapVRAM();
        break;
    case 0x04000242:
        gpu.VRAMCNT_C = data;
        // if vramcnt_c has an mst of 2 and is now enabled,
        // then set bit 0 of VRAMSTAT (vram bank c allocated to the arm7)
        if ((data & (1 << 7)) && ((data & 0x7) == 2)) {
            // then set bit 0
            gpu.VRAMSTAT |= 1;
        } else {
            // reset bit 0
            gpu.VRAMSTAT &= ~1;
        }

        gpu.MapVRAM();
        break;
    case 0x04000243:
        gpu.VRAMCNT_D = data;
        // if vramcnt_d has an mst of 2 and is now enabled,
        // then set bit 0 of VRAMSTAT (vram bank d allocated to the arm7)
        if ((data & (1 << 7)) && ((data & 0x7) == 2)) {
            // then set bit 0
            gpu.VRAMSTAT |= (1 << 1);
        } else {
            // reset bit 0
            gpu.VRAMSTAT &= ~(1 << 1);
        }

        gpu.MapVRAM();
        break;
    case 0x04000244:
        gpu.VRAMCNT_E = data;
        gpu.MapVRAM();
        break;
    case 0x04000245:
        gpu.VRAMCNT_F = data;
        gpu.MapVRAM();
        break;
    case 0x04000246:
        gpu.VRAMCNT_G = data;
        gpu.MapVRAM();
        break;
    case 0x04000247:
        WriteWRAMCNT(data);
        break;
    case 0x04000248:
        gpu.VRAMCNT_H = data;
        gpu.MapVRAM();
        break;
    case 0x04000249:
        gpu.VRAMCNT_I = data;
        gpu.MapVRAM();
        break;
    case 0x04000300:
        POSTFLG9 = data;
        break;
    case 0x0400104C:
        gpu.engine_b.MOSAIC = (gpu.engine_b.MOSAIC & ~0xFF) | data;
        break;
    case 0x0400104D:
        gpu.engine_b.MOSAIC = (gpu.engine_b.MOSAIC & ~0xFF00) | (data << 8);
        break;
    case 0x0400104E:
        gpu.engine_b.MOSAIC = (gpu.engine_b.MOSAIC & ~0xFF0000) | (data << 16);
        break;
    case 0x0400104F:
        gpu.engine_b.MOSAIC = (gpu.engine_b.MOSAIC & ~0xFF000000) | (data << 24);
        break;
    default:
        log_fatal("[ARM9] Undefined 8-bit io write %08x = %02x", addr, data);
    }
}

void HW::ARM9WriteHalfIO(u32 addr, u16 data) {
    if (in_range(0x04000330, 0x10)) {
        return;
    }

    if (in_range(0x04000380, 0x40)) {
        return;
    }

    switch (addr) {
    case 0x04000000:
        gpu.engine_a.DISPCNT = (gpu.engine_a.DISPCNT & ~0xFFFF) | data;
        break;
    case 0x04000004:
        gpu.WriteDISPSTAT9(data);
        break;
    case 0x04000008:
        gpu.engine_a.BGCNT[0] = data;
        break;
    case 0x0400000A:
        gpu.engine_a.BGCNT[1] = data;
        break;
    case 0x0400000C:
        gpu.engine_a.BGCNT[2] = data;
        break;
    case 0x0400000E:
        gpu.engine_a.BGCNT[3] = data;
        break;
    case 0x04000010:
        gpu.engine_a.BGHOFS[0] = data;
        break;
    case 0x04000012:
        gpu.engine_a.BGVOFS[0] = data;
        break;
    case 0x04000014:
        gpu.engine_a.BGHOFS[1] = data;
        break;
    case 0x04000016:
        gpu.engine_a.BGVOFS[1] = data;
        break;
    case 0x04000018:
        gpu.engine_a.BGHOFS[2] = data;
        break;
    case 0x0400001A:
        gpu.engine_a.BGVOFS[2] = data;
        break;
    case 0x0400001C:
        gpu.engine_a.BGHOFS[3] = data;
        break;
    case 0x0400001E:
        gpu.engine_a.BGVOFS[3] = data;
        break;
    case 0x04000020:
        gpu.engine_a.BGPA[0] = data;
        break;
    case 0x04000022:
        gpu.engine_a.BGPB[0] = data;
        break;
    case 0x04000024:
        gpu.engine_a.BGPC[0] = data;
        break;
    case 0x04000026:
        gpu.engine_a.BGPD[0] = data;
        break;
    case 0x04000030:
        gpu.engine_a.BGPA[1] = data;
        break;
    case 0x04000032:
        gpu.engine_a.BGPB[1] = data;
        break;
    case 0x04000034:
        gpu.engine_a.BGPC[1] = data;
        break;
    case 0x04000036:
        gpu.engine_a.BGPD[1] = data;
        break;
    case 0x04000040:
        gpu.engine_a.WINH[0] = data;
        break;
    case 0x04000042:
        gpu.engine_a.WINH[1] = data;
        break;
    case 0x04000044:
        gpu.engine_a.WINV[0] = data;
        break;
    case 0x04000046:
        gpu.engine_a.WINV[1] = data;
        break;
    case 0x04000048:
        gpu.engine_a.WININ = data;
        break;
    case 0x0400004A:
        gpu.engine_a.WINOUT = data;
        break;
    case 0x0400004C:
        gpu.engine_a.MOSAIC = data;
        break;
    case 0x04000050:
        gpu.engine_a.BLDCNT = data;
        break;
    case 0x04000052:
        gpu.engine_a.BLDALPHA = data;
        break;
    case 0x04000054:
        gpu.engine_a.BLDY = data;
        break;
    case 0x04000060:
        gpu.render_engine.DISP3DCNT = data;
        break;
    case 0x04000068:
        // DISP_MMEM_FIFO
        // handle later
        break;
    case 0x0400006C:
        // TODO: handle brightness properly later
        gpu.engine_a.MASTER_BRIGHT = data;
        break;
    case 0x040000B8:
        dma[1].WriteDMACNT_L(0, data);
        break;
    case 0x040000BA:
        dma[1].WriteDMACNT_H(0, data);
        break;
    case 0x040000C4:
        dma[1].WriteDMACNT_L(1, data);
        break;
    case 0x040000C6:
        dma[1].WriteDMACNT_H(1, data);
        break;
    case 0x040000D0:
        dma[1].WriteDMACNT_L(2, data);
        break;
    case 0x040000D2:
        dma[1].WriteDMACNT_H(2, data);
        break;
    case 0x040000DC:
        dma[1].WriteDMACNT_L(3, data);
        break;
    case 0x040000DE:
        dma[1].WriteDMACNT_H(3, data);
        break;
    case 0x04000100:
        timers[1].WriteTMCNT_L(0, data);
        break;
    case 0x04000102:
        timers[1].WriteTMCNT_H(0, data);
        break;
    case 0x04000104:
        timers[1].WriteTMCNT_L(1, data);
        break;
    case 0x04000106:
        timers[1].WriteTMCNT_H(1, data);
        break;
    case 0x04000108:
        timers[1].WriteTMCNT_L(2, data);
        break;
    case 0x0400010A:
        timers[1].WriteTMCNT_H(2, data);
        break;
    case 0x0400010C:
        timers[1].WriteTMCNT_L(3, data);
        break;
    case 0x0400010E:
        timers[1].WriteTMCNT_H(3, data);
        break;
    case 0x04000130:
        input.KEYINPUT = data;
        break;
    case 0x04000180:
        ipc.WriteIPCSYNC9(data);
        break;
    case 0x04000184:
        ipc.WriteIPCFIFOCNT9(data);
        break;
    case 0x040001A0:
        cartridge.WriteAUXSPICNT(data);
        break;
    case 0x04000204:
        EXMEMCNT = data;
        break;
    case 0x04000208:
        interrupt[1].IME = data & 0x1;
        break;
    case 0x04000248:
        gpu.VRAMCNT_H = data & 0xFF;
        gpu.VRAMCNT_I = data >> 8;

        gpu.MapVRAM();
        break;
    case 0x04000280:
        maths_unit.DIVCNT = data;
        maths_unit.StartDivision();
        break;
    case 0x040002B0:
        maths_unit.SQRTCNT = data;
        maths_unit.StartSquareRoot();
        break;
    case 0x04000300:
        POSTFLG9 = data;
        break;
    case 0x04000304:
        gpu.POWCNT1 = data;
        break;
    case 0x04000340:
        gpu.render_engine.ALPHA_TEST_REF = data;
        break;
    case 0x04000354:
        gpu.render_engine.CLEAR_DEPTH = data;
        break;
    case 0x04000356:
        gpu.render_engine.CLRIMAGE_OFFSET = data;
        break;
    case 0x0400035C:
        gpu.render_engine.FOG_OFFSET = data;
        break;
    case 0x04001000:
        gpu.engine_b.DISPCNT = (gpu.engine_b.DISPCNT & ~0xFFFF) | data;
        break;
    case 0x04001008:
        gpu.engine_b.BGCNT[0] = data;
        break;
    case 0x0400100A:
        gpu.engine_b.BGCNT[1] = data;
        break;
    case 0x0400100C:
        gpu.engine_b.BGCNT[2] = data;
        break;
    case 0x0400100E:
        gpu.engine_b.BGCNT[3] = data;
        break;
    case 0x04001010:
        gpu.engine_b.BGHOFS[0] = data;
        break;
    case 0x04001012:
        gpu.engine_b.BGVOFS[0] = data;
        break;
    case 0x04001014:
        gpu.engine_b.BGHOFS[1] = data;
        break;
    case 0x04001016:
        gpu.engine_b.BGVOFS[1] = data;
        break;
    case 0x04001018:
        gpu.engine_b.BGHOFS[2] = data;
        break;
    case 0x0400101A:
        gpu.engine_b.BGVOFS[2] = data;
        break;
    case 0x0400101C:
        gpu.engine_b.BGHOFS[3] = data;
        break;
    case 0x0400101E:
        gpu.engine_b.BGVOFS[3] = data;
        break;
    case 0x04001020:
        gpu.engine_b.BGPA[0] = data;
        break;
    case 0x04001022:
        gpu.engine_b.BGPB[0] = data;
        break;
    case 0x04001024:
        gpu.engine_b.BGPC[0] = data;
        break;
    case 0x04001026:
        gpu.engine_b.BGPD[0] = data;
        break;
    case 0x04001030:
        gpu.engine_b.BGPA[1] = data;
        break;
    case 0x04001032:
        gpu.engine_b.BGPB[1] = data;
        break;
    case 0x04001034:
        gpu.engine_b.BGPC[1] = data;
        break;
    case 0x04001036:
        gpu.engine_b.BGPD[1] = data;
        break;
    case 0x04001040:
        gpu.engine_b.WINH[0] = data;
        break;
    case 0x04001042:
        gpu.engine_b.WINH[1] = data;
        break;
    case 0x04001044:
        gpu.engine_b.WINV[0] = data;
        break;
    case 0x04001046:
        gpu.engine_b.WINV[1] = data;
        break;
    case 0x04001048:
        gpu.engine_b.WININ = data;
        break;
    case 0x0400104A:
        gpu.engine_b.WINOUT = data;
        break;
    case 0x0400104C:
        gpu.engine_b.MOSAIC = data;
        break;
    case 0x04001050:
        gpu.engine_b.BLDCNT = data;
        break;
    case 0x04001052:
        gpu.engine_b.BLDALPHA = data;
        break;
    case 0x04001054:
        gpu.engine_b.BLDY = data;
        break;
    case 0x0400106C:
        // TODO: handle brightness properly later
        gpu.engine_b.MASTER_BRIGHT = data;
        break;
    default:
        log_fatal("[ARM9] Undefined 16-bit io write %08x = %04x", addr, data);
    }
}

void HW::ARM9WriteWordIO(u32 addr, u32 data) {
    if (addr >= 0x04000400 && addr < 0x04000440) {
        gpu.geometry_engine.WriteGXFIFO(data);
        return;
    }
    
    if (addr >= 0x04000440 && addr < 0x040005CC) {
        gpu.geometry_engine.QueueCommand(addr - 0x04000400, data);
        return;
    }

    switch (addr) {
    case 0x04000000:
        gpu.engine_a.DISPCNT = data;
        break;
    case 0x04000004:
        gpu.WriteDISPSTAT9(data & 0xFFFF);
        break;
    case 0x04000008:
        gpu.engine_a.BGCNT[0] = data & 0xFFFF;
        gpu.engine_a.BGCNT[1] = data >> 16;
        break;
    case 0x0400000C:
        gpu.engine_a.BGCNT[2] = data & 0xFFFF;
        gpu.engine_a.BGCNT[3] = data >> 16;
        break;
    case 0x04000010:
        gpu.engine_a.BGHOFS[0] = data & 0xFFFF;
        gpu.engine_a.BGVOFS[0] = data >> 16;
        break;
    case 0x04000014:
        gpu.engine_a.BGHOFS[1] = data & 0xFFFF;
        gpu.engine_a.BGVOFS[1] = data >> 16;
        break;
    case 0x04000018:
        gpu.engine_a.BGHOFS[2] = data & 0xFFFF;
        gpu.engine_a.BGVOFS[2] = data >> 16;
        break;
    case 0x0400001C:
        gpu.engine_a.BGHOFS[3] = data & 0xFFFF;
        gpu.engine_a.BGVOFS[3] = data >> 16;
        break;
    case 0x04000020:
        gpu.engine_a.BGPA[0] = data & 0xFFFF;
        gpu.engine_a.BGPB[0] = data >> 16;
        break;
    case 0x04000024:
        gpu.engine_a.BGPC[0] = data & 0xFFFF;
        gpu.engine_a.BGPD[0] = data >> 16;
        break;
    case 0x04000028:
        gpu.engine_a.WriteBGX(2, data);
        break;
    case 0x0400002C:
        gpu.engine_a.WriteBGY(2, data);
        break;
    case 0x04000030:
        gpu.engine_a.BGPA[1] = data & 0xFFFF;
        gpu.engine_a.BGPB[1] = data >> 16;
        break;
    case 0x04000034:
        gpu.engine_a.BGPC[1] = data & 0xFFFF;
        gpu.engine_a.BGPD[1] = data >> 16;
        break;
    case 0x04000038:
        gpu.engine_a.WriteBGX(3, data);
        break;
    case 0x0400003C:
        gpu.engine_a.WriteBGY(3, data);
        break;
    case 0x04000040:
        gpu.engine_a.WINH[0] = data & 0xFFFF;
        gpu.engine_a.WINH[1] = data >> 16;
        break;
    case 0x04000044:
        gpu.engine_a.WINV[0] = data & 0xFFFF;
        gpu.engine_a.WINV[1] = data >> 16;
        break;
    case 0x04000048:
        gpu.engine_a.WININ = data & 0xFFFF;
        gpu.engine_a.WINOUT = data >> 16;
        break;
    case 0x0400004C:
        gpu.engine_a.MOSAIC = data;
        break;
    case 0x04000050:
        gpu.engine_a.BLDCNT = data & 0xFFFF;
        gpu.engine_a.BLDALPHA = data >> 16;
        break;
    case 0x04000054:
        gpu.engine_a.BLDY = data;
        break;
    case 0x04000058: case 0x0400005C: case 0x04000060:
        break;
    case 0x04000064:
        gpu.DISPCAPCNT = data;
        break;
    case 0x040000B0:
        dma[1].channel[0].source = data;
        break;
    case 0x040000B4:
        dma[1].channel[0].destination = data;
        break;
    case 0x040000B8:
        dma[1].WriteDMACNT(0, data);
        break;
    case 0x040000BC:
        dma[1].channel[1].source = data;
        break;
    case 0x040000C0:
        dma[1].channel[1].destination = data;
        break;
    case 0x040000C4:
        dma[1].WriteDMACNT(1, data);
        break;
    case 0x040000C8:
        dma[1].channel[2].source = data;
        break;
    case 0x040000CC:
        dma[1].channel[2].destination = data;
        break;
    case 0x040000D0:
        dma[1].WriteDMACNT(2, data);
        break;
    case 0x040000D4:
        dma[1].channel[3].source = data;
        break;
    case 0x040000D8:
        dma[1].channel[3].destination = data;
        break;
    case 0x040000DC:
        dma[1].WriteDMACNT(3, data);
        break;
    case 0x040000E0:
        dma[1].DMAFILL[0] = data;
        break;
    case 0x040000E4:
        dma[1].DMAFILL[1] = data;
        break;
    case 0x040000E8:
        dma[1].DMAFILL[2] = data;
        break;
    case 0x040000EC:
        dma[1].DMAFILL[3] = data;
        break;
    case 0x040001A0:
        cartridge.WriteAUXSPICNT(data & 0xFFFF);
        cartridge.WriteAUXSPIDATA(data >> 16);
        break;
    case 0x040001A4:
        cartridge.WriteROMCTRL(data);
        break;
    case 0x040001A8:
        cartridge.ReceiveCommand(data & 0xFF, 0);
        cartridge.ReceiveCommand((data >> 8) & 0xFF, 1);
        cartridge.ReceiveCommand((data >> 16) & 0xFF, 2);
        cartridge.ReceiveCommand((data >> 24) & 0xFF, 3);
        break;
    case 0x040001AC:
        cartridge.ReceiveCommand(data & 0xFF, 4);
        cartridge.ReceiveCommand((data >> 8) & 0xFF, 5);
        cartridge.ReceiveCommand((data >> 16) & 0xFF, 6);
        cartridge.ReceiveCommand((data >> 24) & 0xFF, 7);
        break;
    case 0x04000180:
        ipc.WriteIPCSYNC9(data);
        break;
    case 0x04000188:
        ipc.WriteFIFOSEND9(data);
        break;
    case 0x04000208:
        interrupt[1].IME = data & 0x1;
        break;
    case 0x04000210:
        interrupt[1].IE = data;
        break;
    case 0x04000214:
        interrupt[1].IF &= ~data;
        break;
    case 0x04000240:
        gpu.VRAMCNT_A = data & 0xFF;
        gpu.VRAMCNT_B = (data >> 8) & 0xFF;
        gpu.VRAMCNT_C = (data >> 16) & 0xFF;
        gpu.VRAMCNT_D = (data >> 24) & 0xFF;

        gpu.MapVRAM();
        break;
    case 0x04000244:
        // sets vramcnt_e, vramcnt_f, vramcnt_g and wramcnt
        gpu.VRAMCNT_E = data & 0xFF;
        gpu.VRAMCNT_F = (data >> 8) & 0xFF;
        gpu.VRAMCNT_G = (data >> 16) & 0xFF;
        WriteWRAMCNT((data >> 24) & 0xFF);

        gpu.MapVRAM();
        break;
    case 0x04000280:
        maths_unit.DIVCNT = data;
        maths_unit.StartDivision();
    case 0x04000290:
        // write to lower 32 bits of DIV_NUMER, starting a division
        maths_unit.DIV_NUMER = (maths_unit.DIV_NUMER & ~0xFFFFFFFF) | data;
        maths_unit.StartDivision();
        break;
    case 0x04000294:
        // write to upper 32 bits of DIV_NUMER, starting a division
        maths_unit.DIV_NUMER = (maths_unit.DIV_NUMER & 0xFFFFFFFF) | ((u64)data << 32);
        maths_unit.StartDivision();
        break;
    case 0x04000298:
        // write to lower 32 bits of DIV_DENOM, starting a division
        maths_unit.DIV_DENOM = (maths_unit.DIV_DENOM & ~0xFFFFFFFF) | data;
        maths_unit.StartDivision();
        break;
    case 0x0400029C:
        // write to upper 32 bits of DIV_DENOM, starting a division
        maths_unit.DIV_DENOM = (maths_unit.DIV_DENOM & 0xFFFFFFFF) | ((u64)data << 32);
        maths_unit.StartDivision();
        break;
    case 0x040002B8:
        // write to lower 32 bits of SQRT_PARAM
        maths_unit.SQRT_PARAM = (maths_unit.SQRT_PARAM & ~0xFFFFFFFF) | data;
        maths_unit.StartSquareRoot();
        break;
    case 0x040002BC:
        // write to upper 32 bits of SQRT_PARAM
        maths_unit.SQRT_PARAM = (maths_unit.SQRT_PARAM & 0xFFFFFFFF) | ((u64)data << 32);
        maths_unit.StartSquareRoot();
        break;
    case 0x04000304:
        gpu.POWCNT1 = data;
        break;
    case 0x04000330: case 0x04000331: case 0x04000332: case 0x04000333:
    case 0x04000334: case 0x04000335: case 0x04000336: case 0x04000337:
    case 0x04000338: case 0x04000339: case 0x0400033A: case 0x0400033B:
    case 0x0400033C: case 0x0400033D: case 0x0400033E: case 0x0400033F:
        gpu.render_engine.EDGE_COLOR[addr - 0x04000330] = data;
        break;
    case 0x04000350:
        gpu.render_engine.CLEAR_COLOR = data;
        break;
    case 0x04000358:
        gpu.render_engine.FOG_COLOR = data;
        break;
    case 0x04000360: case 0x04000361: case 0x04000362: case 0x04000363:
    case 0x04000364: case 0x04000365: case 0x04000366: case 0x04000367:
    case 0x04000368: case 0x04000369: case 0x0400036A: case 0x0400036B:
    case 0x0400036C: case 0x0400036D: case 0x0400036E: case 0x0400036F:
    case 0x04000370: case 0x04000371: case 0x04000372: case 0x04000373:
    case 0x04000374: case 0x04000375: case 0x04000376: case 0x04000377:
    case 0x04000378: case 0x04000379: case 0x0400037A: case 0x0400037B:
    case 0x0400037C: case 0x0400037D: case 0x0400037E: case 0x0400037F:
        gpu.render_engine.FOG_TABLE[addr - 0x04000360] = data;
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
        gpu.render_engine.TOON_TABLE[addr - 0x04000380] = data;
        break;
    case 0x04000600:
        gpu.geometry_engine.GXSTAT = data;
        break;
    case 0x04001000:
        gpu.engine_b.DISPCNT = data;
        break;
    case 0x04001004:
        break;
    case 0x04001008:
        gpu.engine_b.BGCNT[0] = data & 0xFFFF;
        gpu.engine_b.BGCNT[1] = data >> 16;
        break;
    case 0x0400100C:
        gpu.engine_b.BGCNT[2] = data & 0xFFFF;
        gpu.engine_b.BGCNT[3] = data >> 16;
        break;
    case 0x04001010:
        gpu.engine_b.BGHOFS[0] = data & 0xFFFF;
        gpu.engine_b.BGVOFS[0] = data >> 16;
        break;
    case 0x04001014:
        gpu.engine_b.BGHOFS[1] = data & 0xFFFF;
        gpu.engine_b.BGVOFS[1] = data >> 16;
        break;
    case 0x04001018:
        gpu.engine_b.BGHOFS[2] = data & 0xFFFF;
        gpu.engine_b.BGVOFS[2] = data >> 16;
        break;
    case 0x0400101C:
        gpu.engine_b.BGHOFS[3] = data & 0xFFFF;
        gpu.engine_b.BGVOFS[3] = data >> 16;
        break;
    case 0x04001020:
        gpu.engine_b.BGPA[0] = data & 0xFFFF;
        gpu.engine_b.BGPB[0] = data >> 16;
        break;
    case 0x04001024:
        gpu.engine_b.BGPC[0] = data & 0xFFFF;
        gpu.engine_b.BGPD[0] = data >> 16;
        break;
    case 0x04001028:
        gpu.engine_b.WriteBGX(2, data);
        break;
    case 0x0400102C:
        gpu.engine_b.WriteBGY(2, data);
        break;
    case 0x04001030:
        gpu.engine_b.BGPA[1] = data & 0xFFFF;
        gpu.engine_b.BGPB[1] = data >> 16;
        break;
    case 0x04001034:
        gpu.engine_b.BGPC[1] = data & 0xFFFF;
        gpu.engine_b.BGPD[1] = data >> 16;
        break;
    case 0x04001038:
        gpu.engine_b.WriteBGX(3, data);
        break;
    case 0x0400103C:
        gpu.engine_b.WriteBGY(3, data);
        break;
    case 0x04001040:
        gpu.engine_b.WINH[0] = data & 0xFFFF;
        gpu.engine_b.WINH[1] = data >> 16;
        break;
    case 0x04001044:
        gpu.engine_b.WINV[0] = data & 0xFFFF;
        gpu.engine_b.WINV[1] = data >> 16;
        break;
    case 0x04001048:
        gpu.engine_b.WININ = data & 0xFFFF;
        gpu.engine_b.WINOUT = data >> 16;
        break;
    case 0x0400104C:
        gpu.engine_b.MOSAIC = data;
        break;
    case 0x04001050:
        gpu.engine_b.BLDCNT = data & 0xFFFF;
        gpu.engine_b.BLDALPHA = data >> 16;
        break;
    case 0x04001054:
        gpu.engine_b.BLDY = data;
        break;
    case 0x04001058: case 0x0400105C: case 0x04001060: case 0x04001064: case 0x04001068:
        break;
    case 0x0400106C:
        gpu.engine_b.MASTER_BRIGHT = data & 0xFFFF;
        break;
    default:
        log_fatal("[ARM9] Undefined 32-bit io write %08x = %08x", addr, data);
    }
}