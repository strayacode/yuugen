#include <core/arm/memory.h>
#include <core/core.h>

auto Memory::ARM9ReadByteIO(u32 addr) -> u8 {
    switch (addr) {
    case 0x04000208:
        return core->interrupt[1].IME & 0x1;
    case 0x04004000:
        return 0;
    default:
        log_fatal("[ARM9] Undefined 8-bit io read %08x", addr);
    }
}

auto Memory::ARM9ReadHalfIO(u32 addr) -> u16 {
    switch (addr) {
    case 0x04000000:
        return core->gpu.engine_a.DISPCNT & 0xFFFF;
    case 0x04000004:
        return core->gpu.DISPSTAT9;
    case 0x04000006:
        return core->gpu.VCOUNT;
    case 0x0400000E:
        return core->gpu.engine_a.BGCNT[3];
    case 0x040000BA:
        return core->dma[1].ReadDMACNT_H(0);
    case 0x040000C6:
        return core->dma[1].ReadDMACNT_H(1);
    case 0x040000D2:
        return core->dma[1].ReadDMACNT_H(2);
    case 0x040000DE:
        return core->dma[1].ReadDMACNT_H(3);
    case 0x04000100:
        return core->timers[1].ReadTMCNT_L(0);
    case 0x04000104:
        return core->timers[1].ReadTMCNT_L(1);
    case 0x04000108:
        return core->timers[1].ReadTMCNT_L(2);
    case 0x0400010C:
        return core->timers[1].ReadTMCNT_L(3);
    case 0x04000130:
        return core->input.KEYINPUT;
    case 0x04000180:
        return core->ipc.ReadIPCSYNC9();
    case 0x04000184:
        return core->ipc.IPCFIFOCNT9;
    case 0x04000204:
        return EXMEMCNT;
    case 0x04000208:
        return core->interrupt[1].IME & 0x1;
    case 0x04000280:
        return core->maths_unit.DIVCNT;
    case 0x040002B0:
        return core->maths_unit.SQRTCNT;
    case 0x04000300:
        return POSTFLG9;
    case 0x04001008:
        return core->gpu.engine_b.BGCNT[0];
    default:
        log_fatal("[ARM9] Undefined 16-bit io read %08x", addr);
    }
}

auto Memory::ARM9ReadWordIO(u32 addr) -> u32 {
    switch (addr) {
    case 0x04000000:
        return core->gpu.engine_a.DISPCNT;
    case 0x040000B0:
        return core->dma[1].channel[0].source;
    case 0x040000B4:
        return core->dma[1].channel[0].destination;
    case 0x040000B8:
        return core->dma[1].ReadDMACNT(0);
    case 0x040000BC:
        return core->dma[1].channel[1].source;
    case 0x040000C0:
        return core->dma[1].channel[1].destination;
    case 0x040000C4:
        return core->dma[1].ReadDMACNT(1);
    case 0x040000C8:
        return core->dma[1].channel[2].source;
    case 0x040000CC:
        return core->dma[1].channel[2].destination;
    case 0x040000D0:
        return core->dma[1].ReadDMACNT(2);
    case 0x040000D4:
        return core->dma[1].channel[3].source;
    case 0x040000D8:
        return core->dma[1].channel[3].destination;
    case 0x040000DC:
        return core->dma[1].ReadDMACNT(3);
    case 0x040000E0:
        return core->dma[1].DMAFILL[0];
    case 0x040000E4:
        return core->dma[1].DMAFILL[1];
    case 0x040000E8:
        return core->dma[1].DMAFILL[2];
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
    case 0x04000290:
        return core->maths_unit.DIV_NUMER & 0xFFFFFFFF;
    case 0x04000294:
        return core->maths_unit.DIV_NUMER >> 32;
    case 0x04000298:
        return core->maths_unit.DIV_DENOM & 0xFFFFFFFF;
    case 0x0400029C:
        return core->maths_unit.DIV_DENOM >> 32;
    case 0x040002B8:
        return core->maths_unit.SQRT_PARAM & 0xFFFFFFFF;
    case 0x040002BC:
        return core->maths_unit.SQRT_PARAM >> 32;
    case 0x04001000:
        return core->gpu.engine_b.DISPCNT;
    case 0x04004000:
        return 0;
    case 0x04004008:
        return 0;
    case 0x04100000:
        return core->ipc.ReadFIFORECV9();
    default:
        log_fatal("[ARM9] Undefined 32-bit io read %08x", addr);
    }
}

void Memory::ARM9WriteByteIO(u32 addr, u8 data) {
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
    case 0x04000247:
        WRAMCNT = data;
        break;
    case 0x04000248:
        core->gpu.VRAMCNT_H = data;
        break;
    case 0x04000249:
        core->gpu.VRAMCNT_I = data;
        break;
    default:
        log_fatal("[ARM9] Undefined 8-bit io write %08x = %02x", addr, data);
    }
}

void Memory::ARM9WriteHalfIO(u32 addr, u16 data) {
    switch (addr) {
    case 0x04000004:
        core->gpu.WriteDISPSTAT9(data);
        break;
    case 0x04000008:
        core->gpu.engine_a.BGCNT[0] = data;
        break;
    case 0x0400000C:
        core->gpu.engine_a.BGCNT[2] = data;
        break;
    case 0x0400000E:
        core->gpu.engine_a.BGCNT[3] = data;
        break;
    case 0x04000018:
        core->gpu.engine_a.BGHOFS[2] = data;
        break;
    case 0x0400001A:
        core->gpu.engine_a.BGVOFS[2] = data;
        break;
    case 0x04000030:
        core->gpu.engine_a.BG3P[0] = data;
        break;
    case 0x04000032:
        core->gpu.engine_a.BG3P[1] = data;
        break;
    case 0x04000034:
        core->gpu.engine_a.BG3P[2] = data;
        break;
    case 0x04000036:
        core->gpu.engine_a.BG3P[3] = data;
        break;
    case 0x040000B8:
        core->dma[1].WriteDMACNT_L(0, data);
        break;
    case 0x040000BA:
        core->dma[1].WriteDMACNT_H(0, data);
        break;
    case 0x040000C4:
        core->dma[1].WriteDMACNT_L(1, data);
        break;
    case 0x040000C6:
        core->dma[1].WriteDMACNT_H(1, data);
        break;
    case 0x040000D0:
        core->dma[1].WriteDMACNT_L(2, data);
        break;
    case 0x040000D2:
        core->dma[1].WriteDMACNT_H(2, data);
        break;
    case 0x040000DC:
        core->dma[1].WriteDMACNT_L(3, data);
        break;
    case 0x040000DE:
        core->dma[1].WriteDMACNT_H(3, data);
        break;
    case 0x04000100:
        core->timers[1].WriteTMCNT_L(0, data);
        break;
    case 0x04000102:
        core->timers[1].WriteTMCNT_H(0, data);
        break;
    case 0x04000104:
        core->timers[1].WriteTMCNT_L(1, data);
        break;
    case 0x04000106:
        core->timers[1].WriteTMCNT_H(1, data);
        break;
    case 0x04000108:
        core->timers[1].WriteTMCNT_L(2, data);
        break;
    case 0x0400010A:
        core->timers[1].WriteTMCNT_H(2, data);
        break;
    case 0x0400010C:
        core->timers[1].WriteTMCNT_L(3, data);
        break;
    case 0x0400010E:
        core->timers[1].WriteTMCNT_H(3, data);
        break;
    case 0x04000130:
        core->input.KEYINPUT = data;
        break;
    case 0x04000180:
        core->ipc.WriteIPCSYNC9(data);
        break;
    case 0x04000184:
        core->ipc.WriteIPCFIFOCNT9(data);
        break;
    case 0x04000204:
        EXMEMCNT = data;
        break;
    case 0x04000208:
        core->interrupt[1].IME = data & 0x1;
        break;
    case 0x04000280:
        core->maths_unit.DIVCNT = data;
        core->maths_unit.StartDivision();
        break;
    case 0x040002B0:
        core->maths_unit.SQRTCNT = data;
        core->maths_unit.StartSquareRoot();
        break;
    case 0x04000304:
        core->gpu.POWCNT1 = data;
        break;
    case 0x04001008:
        core->gpu.engine_b.BGCNT[0] = data;
        break;
    case 0x0400100E:
        core->gpu.engine_b.BGCNT[3] = data;
        break;
    case 0x04001010:
        core->gpu.engine_b.BGHOFS[0] = data;
        break;
    case 0x04001012:
        core->gpu.engine_b.BGVOFS[0] = data;
        break;
    case 0x04001030:
        core->gpu.engine_b.BG3P[0] = data;
        break;
    case 0x04001032:
        core->gpu.engine_b.BG3P[1] = data;
        break;
    case 0x04001034:
        core->gpu.engine_b.BG3P[2] = data;
        break;
    case 0x04001036:
        core->gpu.engine_b.BG3P[3] = data;
        break;
    case 0x0400106C:
        // TODO: handle brightness properly later
        core->gpu.engine_b.MASTER_BRIGHT = data;
        break;
    default:
        log_fatal("[ARM9] Undefined 16-bit io write %08x = %04x", addr, data);
    }
}

void Memory::ARM9WriteWordIO(u32 addr, u32 data) {
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
    case 0x040000E0:
        core->dma[1].DMAFILL[0] = data;
        break;
    case 0x040000E4:
        core->dma[1].DMAFILL[1] = data;
        break;
    case 0x040000E8:
        core->dma[1].DMAFILL[2] = data;
        break;
    case 0x040000EC:
        core->dma[1].DMAFILL[3] = data;
        break;
    case 0x04000180:
        core->ipc.WriteIPCSYNC9(data);
        break;
    case 0x04000188:
        core->ipc.WriteFIFOSEND9(data);
        break;
    case 0x04000208:
        core->interrupt[1].IME = data & 0x1;
        break;
    case 0x04000210:
        core->interrupt[1].IE = data;
        break;
    case 0x04000214:
        core->interrupt[1].IF &= ~data;
        break;
    case 0x04000240:
        core->gpu.VRAMCNT_A = data & 0xFF;
        core->gpu.VRAMCNT_B = (data >> 8) & 0xFF;
        core->gpu.VRAMCNT_C = (data >> 16) & 0xFF;
        core->gpu.VRAMCNT_D = (data >> 24) & 0xFF;
        break;
    case 0x04000280:
        core->maths_unit.DIVCNT = data;
        core->maths_unit.StartDivision();
    case 0x04000290:
        // write to lower 32 bits of DIV_NUMER, starting a division
        core->maths_unit.DIV_NUMER = (core->maths_unit.DIV_NUMER & ~0xFFFFFFFF) | data;
        core->maths_unit.StartDivision();
        break;
    case 0x04000294:
        // write to upper 32 bits of DIV_NUMER, starting a division
        core->maths_unit.DIV_NUMER = (core->maths_unit.DIV_NUMER & 0xFFFFFFFF) | ((u64)data << 32);
        core->maths_unit.StartDivision();
        break;
    case 0x04000298:
        // write to lower 32 bits of DIV_DENOM, starting a division
        core->maths_unit.DIV_DENOM = (core->maths_unit.DIV_DENOM & ~0xFFFFFFFF) | data;
        core->maths_unit.StartDivision();
        break;
    case 0x0400029C:
        // write to upper 32 bits of DIV_DENOM, starting a division
        core->maths_unit.DIV_DENOM = (core->maths_unit.DIV_DENOM & 0xFFFFFFFF) | ((u64)data << 32);
        core->maths_unit.StartDivision();
        break;
    case 0x040002B8:
        // write to lower 32 bits of SQRT_PARAM
        core->maths_unit.SQRT_PARAM = (core->maths_unit.SQRT_PARAM & ~0xFFFFFFFF) | data;
        core->maths_unit.StartSquareRoot();
        break;
    case 0x040002BC:
        // write to upper 32 bits of SQRT_PARAM
        core->maths_unit.SQRT_PARAM = (core->maths_unit.SQRT_PARAM & 0xFFFFFFFF) | ((u64)data << 32);
        core->maths_unit.StartSquareRoot();
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
        log_fatal("[ARM9] Undefined 32-bit io write %08x = %08x", addr, data);
    }
}