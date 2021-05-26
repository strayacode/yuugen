#include <core/hw/gpu/gpu.h>
#include <core/core.h>

GPU::GPU(Core* core) : core(core), engine_a(this, 1), engine_b(this , 0) {

}

void GPU::Reset() {
    memset(VRAM_A, 0, 0x20000);
    memset(VRAM_B, 0, 0x20000);
    memset(VRAM_C, 0, 0x20000);
    memset(VRAM_D, 0, 0x20000);
    memset(VRAM_E, 0, 0x10000);
    memset(VRAM_F, 0, 0x4000);
    memset(VRAM_G, 0, 0x4000);
    memset(VRAM_H, 0, 0x8000);
    memset(VRAM_I, 0, 0x4000);

    POWCNT1 = 0;
    VCOUNT = 0;
    VRAMCNT_A = 0;
    VRAMCNT_B = 0;
    VRAMCNT_C = 0;
    VRAMCNT_D = 0;
    VRAMCNT_E = 0;
    VRAMCNT_F = 0;
    VRAMCNT_G = 0;
    VRAMCNT_H = 0;
    VRAMCNT_I = 0;

    VRAMSTAT = 0;

    DISPSTAT7 = 0;
    DISPSTAT9 = 0;

    DISPCAPCNT = 0;

    engine_a.Reset();
    engine_b.Reset();

    // schedule the first RenderScanlineStart event
    // we would expect hblank to start at 1536 (256 * 6) cycles
    // but actually the hblank flag stays 0 until 1606 cycles have passed
    core->scheduler.Add(1606, RenderScanlineStartTask);
}

void GPU::RenderScanlineStart() {
    if (VCOUNT < 192) {
        engine_a.RenderScanline(VCOUNT);
        engine_b.RenderScanline(VCOUNT);

        // trigger an arm9 dma transfer on hblank (only for visible scanlines)
        core->dma[1].Trigger(2);
    }

    DISPSTAT7 |= (1 << 1);
    DISPSTAT9 |= (1 << 1);
    
    if (DISPSTAT7 & (1 << 4)) {
        core->arm7.SendInterrupt(1);
    }

    if (DISPSTAT9 & (1 << 4)) {
        core->arm9.SendInterrupt(1);
    }

    // ARM9 DMA exclusive
    // check if scanline is between 2 and 193 inclusive
    // if so trigger a start of display dma transfer
    // TODO: on scanline 194 automatically clear the enable bit in DMA
    if ((VCOUNT > 1) && (VCOUNT < 194)) {
        core->dma[1].Trigger(3);
    }

    core->scheduler.Add(524, RenderScanlineFinishTask);
}

void GPU::RenderScanlineFinish() {
    DISPSTAT7 &= ~(1 << 1);
    DISPSTAT9 &= ~(1 << 1);

    switch (++VCOUNT) {
    case 192:
        // start of vblank
        DISPSTAT7 |= 1;
        DISPSTAT9 |= 1;

        if (DISPSTAT7 & (1 << 3)) {
            core->arm7.SendInterrupt(0);
        }

        if (DISPSTAT9 & (1 << 3)) {
            core->arm9.SendInterrupt(0);
        }

        core->dma[0].Trigger(1);
        core->dma[1].Trigger(1);
        break;
    case 262:
        // end of vblank
        DISPSTAT7 &= ~1;
        DISPSTAT9 &= ~1;

        break;
    case 263:
        // end of frame
        VCOUNT = 0;
        break;
    }

    if (((DISPSTAT7 >> 8) | ((DISPSTAT7 & (1 << 7)) << 1)) == VCOUNT) {
        DISPSTAT7 |= (1 << 2);

        if (DISPSTAT7 & (1 << 5)) {
            core->arm7.SendInterrupt(2);
        }

    } else if (DISPSTAT7 & (1 << 2)) {
        DISPSTAT7 &= ~(1 << 2);
    }

    if (((DISPSTAT9 >> 8) | ((DISPSTAT9 & (1 << 7)) << 1)) == VCOUNT) {
        DISPSTAT9 |= (1 << 2);

        if (DISPSTAT9 & (1 << 5)) {
            core->arm9.SendInterrupt(2);
        }

    } else if (DISPSTAT9 & (1 << 2)) {
        DISPSTAT9 &= ~(1 << 2);
    }

    core->scheduler.Add(1606, RenderScanlineStartTask);
}

void GPU::WriteDISPSTAT7(u16 data) {
    // preserve non writeable bits too 
    DISPSTAT7 = (DISPSTAT7 & ~0xFFB8) | (data & 0xFFB8);
}

void GPU::WriteDISPSTAT9(u16 data) {
    // preserve non writeable bits too
    DISPSTAT9 = (DISPSTAT9 & ~0xFFB8) | (data & 0xFFB8);
}

auto GPU::GetFramebuffer(Screen screen) -> const u32* {
    switch (screen) {
    case TOP_SCREEN:
        return ((POWCNT1 & (1 << 15)) ? engine_a.GetFramebuffer() : engine_b.GetFramebuffer());
    default:
        // default is bottom screen but we just have this to avoid a warning of no return value
        return ((POWCNT1 & (1 << 15)) ? engine_b.GetFramebuffer() : engine_a.GetFramebuffer());
    }
}

auto GPU::GetVRAMCNTMST(u8 vramcnt) -> int {
    return vramcnt & 0x7;
}

auto GPU::GetVRAMCNTOffset(u8 vramcnt) -> int {
    return (vramcnt >> 3) & 0x3;
}

auto GPU::GetVRAMCNTEnabled(u8 vramcnt) -> bool {
    return (vramcnt & (1 << 7));
}

template void GPU::WriteLCDC(u32 addr, u8 data);
template void GPU::WriteLCDC(u32 addr, u16 data);
template void GPU::WriteLCDC(u32 addr, u32 data);
template <typename T>
void GPU::WriteLCDC(u32 addr, T data) {
    // small optimisation by checking enabled first instead of in a range as it should be faster?
    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06800000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_A) == 0)) {
            memcpy(&VRAM_A[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06820000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_B) == 0)) {
            memcpy(&VRAM_B[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06840000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_C) == 0)) {
            memcpy(&VRAM_C[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06860000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_D) == 0)) {
            memcpy(&VRAM_D[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06880000, 0x10000) && (GetVRAMCNTMST(VRAMCNT_E) == 0)) {
            memcpy(&VRAM_E[addr & 0xFFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06890000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_F) == 0)) {
            memcpy(&VRAM_F[addr & 0x3FFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06894000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_G) == 0)) {
            memcpy(&VRAM_G[addr & 0x3FFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        if (in_range(0x06898000, 0x8000) && (GetVRAMCNTMST(VRAMCNT_H) == 0)) {
            memcpy(&VRAM_H[addr & 0x7FFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x068A0000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_I) == 0)) {
            memcpy(&VRAM_I[addr & 0x3FFF], &data, sizeof(T));
        }
    }
}

template auto GPU::ReadLCDC(u32 addr) -> u8;
template auto GPU::ReadLCDC(u32 addr) -> u16;
template auto GPU::ReadLCDC(u32 addr) -> u32;
template <typename T>
auto GPU::ReadLCDC(u32 addr) -> T {
    T return_value = 0;

    // small optimisation by checking enabled first instead of in a range as it should be faster?
    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06800000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_A) == 0)) {
            memcpy(&return_value, &VRAM_A[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06820000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_B) == 0)) {
            memcpy(&return_value, &VRAM_B[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06840000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_C) == 0)) {
            memcpy(&return_value, &VRAM_C[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06860000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_D) == 0)) {
            memcpy(&return_value, &VRAM_D[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06880000, 0x10000) && (GetVRAMCNTMST(VRAMCNT_E) == 0)) {
            memcpy(&return_value, &VRAM_E[addr & 0xFFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06890000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_F) == 0)) {
            memcpy(&return_value, &VRAM_F[addr & 0x3FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06894000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_G) == 0)) {
            memcpy(&return_value, &VRAM_G[addr & 0x3FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        if (in_range(0x06898000, 0x8000) && (GetVRAMCNTMST(VRAMCNT_H) == 0)) {
            memcpy(&return_value, &VRAM_H[addr & 0x7FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x068A0000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_I) == 0)) {
            memcpy(&return_value, &VRAM_I[addr & 0x3FFF], sizeof(T));
        }
    }

    return return_value;
}

template void GPU::WriteBGA(u32 addr, u8 data);
template void GPU::WriteBGA(u32 addr, u16 data);
template void GPU::WriteBGA(u32 addr, u32 data);
template <typename T>
void GPU::WriteBGA(u32 addr, T data) {
    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_A)), 0x20000) && (GetVRAMCNTMST(VRAMCNT_A) == 1)) {
            memcpy(&VRAM_A[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_B)), 0x20000) && (GetVRAMCNTMST(VRAMCNT_B) == 1)) {
            memcpy(&VRAM_B[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_C)), 0x20000) && (GetVRAMCNTMST(VRAMCNT_C) == 1)) {
            memcpy(&VRAM_C[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_D)), 0x20000) && (GetVRAMCNTMST(VRAMCNT_D) == 1)) {
            memcpy(&VRAM_D[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06000000, 0x10000) && (GetVRAMCNTMST(VRAMCNT_E) == 1)) {
            memcpy(&VRAM_E[addr & 0xFFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06000000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x2)), 0x3FFF) && (GetVRAMCNTMST(VRAMCNT_F) == 1)) {
            memcpy(&VRAM_F[addr & 0x3FFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06000000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x2)), 0x3FFF) && (GetVRAMCNTMST(VRAMCNT_G) == 1)) {
            memcpy(&VRAM_G[addr & 0x3FFF], &data, sizeof(T));
        }
    }
}

template auto GPU::ReadBGA(u32 addr) -> u8;
template auto GPU::ReadBGA(u32 addr) -> u16;
template auto GPU::ReadBGA(u32 addr) -> u32;
template <typename T>
auto GPU::ReadBGA(u32 addr) -> T {
    T return_value = 0;

    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_A)), 0x20000) && (GetVRAMCNTMST(VRAMCNT_A) == 1)) {
            memcpy(&return_value, &VRAM_A[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_B)), 0x20000) && (GetVRAMCNTMST(VRAMCNT_B) == 1)) {
            memcpy(&return_value, &VRAM_B[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_C)), 0x20000) && (GetVRAMCNTMST(VRAMCNT_C) == 1)) {
            memcpy(&return_value, &VRAM_C[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_D)), 0x20000) && (GetVRAMCNTMST(VRAMCNT_D) == 1)) {
            memcpy(&return_value, &VRAM_D[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06000000, 0x10000) && (GetVRAMCNTMST(VRAMCNT_E) == 1)) {
            memcpy(&return_value, &VRAM_E[addr & 0xFFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06000000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x2)), 0x3FFF) && (GetVRAMCNTMST(VRAMCNT_F) == 1)) {
            memcpy(&return_value, &VRAM_F[addr & 0x3FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06000000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x2)), 0x3FFF) && (GetVRAMCNTMST(VRAMCNT_G) == 1)) {
            memcpy(&return_value, &VRAM_G[addr & 0x3FFF], sizeof(T));
        }
    }

    return return_value;
}

template void GPU::WriteBGB(u32 addr, u8 data);
template void GPU::WriteBGB(u32 addr, u16 data);
template void GPU::WriteBGB(u32 addr, u32 data);
template <typename T>
void GPU::WriteBGB(u32 addr, T data) {
    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06200000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_C) == 4)) {
            memcpy(&VRAM_C[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        if (in_range(0x06200000, 0x8000) && (GetVRAMCNTMST(VRAMCNT_H) == 1)) {
            memcpy(&VRAM_H[addr & 0x7FFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x06208000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_I) == 1)) {
            memcpy(&VRAM_I[addr & 0x3FFF], &data, sizeof(T));
        }
    }
}

template auto GPU::ReadBGB(u32 addr) -> u8;
template auto GPU::ReadBGB(u32 addr) -> u16;
template auto GPU::ReadBGB(u32 addr) -> u32;
template <typename T>
auto GPU::ReadBGB(u32 addr) -> T {
    T return_value = 0;

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06200000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_C) == 4)) {
            memcpy(&return_value, &VRAM_C[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        if (in_range(0x06200000, 0x8000) && (GetVRAMCNTMST(VRAMCNT_H) == 1)) {
            memcpy(&return_value, &VRAM_H[addr & 0x7FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x06208000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_I) == 1)) {
            memcpy(&return_value, &VRAM_I[addr & 0x3FFF], sizeof(T));
        }
    }

    return return_value;
}

template auto GPU::ReadARM7(u32 addr) -> u8;
template auto GPU::ReadARM7(u32 addr) -> u16;
template auto GPU::ReadARM7(u32 addr) -> u32;
template <typename T>
auto GPU::ReadARM7(u32 addr) -> T {
    T return_value = 0;
    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(VRAMCNT_C) * 0x20000), 0x20000) && (GetVRAMCNTMST(VRAMCNT_C) == 2)) {
            memcpy(&return_value, &VRAM_C[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(VRAMCNT_D) * 0x20000), 0x20000) && (GetVRAMCNTMST(VRAMCNT_D) == 2)) {
            memcpy(&return_value, &VRAM_D[addr & 0x1FFFF], sizeof(T));
        }
    }

    return return_value;
}

template void GPU::WriteARM7(u32 addr, u8 data);
template void GPU::WriteARM7(u32 addr, u16 data);
template void GPU::WriteARM7(u32 addr, u32 data);
template <typename T>
void GPU::WriteARM7(u32 addr, T data) {
    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(VRAMCNT_C) * 0x20000), 0x20000) && (GetVRAMCNTMST(VRAMCNT_C) == 2)) {
            memcpy(&VRAM_C[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(VRAMCNT_D) * 0x20000), 0x20000) && (GetVRAMCNTMST(VRAMCNT_D) == 2)) {
            memcpy(&VRAM_D[addr & 0x1FFFF], &data, sizeof(T));
        }
    }
}

template void GPU::WriteOBJA(u32 addr, u8 data);
template void GPU::WriteOBJA(u32 addr, u16 data);
template void GPU::WriteOBJA(u32 addr, u32 data);
template <typename T>
void GPU::WriteOBJA(u32 addr, T data) {
    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06400000 + ((GetVRAMCNTOffset(VRAMCNT_A) & 0x1) * 0x20000), 0x20000) && (GetVRAMCNTMST(VRAMCNT_A) == 2)) {
            memcpy(&VRAM_A[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06400000 + ((GetVRAMCNTOffset(VRAMCNT_B) & 0x1) * 0x20000), 0x20000) && (GetVRAMCNTMST(VRAMCNT_B) == 2)) {
            memcpy(&VRAM_B[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06400000, 0x10000) && (GetVRAMCNTMST(VRAMCNT_E) == 2)) {
            memcpy(&VRAM_E[addr & 0xFFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06400000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x2)), 0x3FFF) && (GetVRAMCNTMST(VRAMCNT_F) == 2)) {
            memcpy(&VRAM_F[addr & 0x3FFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06400000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x2)), 0x3FFF) && (GetVRAMCNTMST(VRAMCNT_G) == 2)) {
            memcpy(&VRAM_G[addr & 0x3FFF], &data, sizeof(T));
        }
    }
}

template void GPU::WriteOBJB(u32 addr, u8 data);
template void GPU::WriteOBJB(u32 addr, u16 data);
template void GPU::WriteOBJB(u32 addr, u32 data);
template <typename T>
void GPU::WriteOBJB(u32 addr, T data) {
    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06600000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_D) == 4)) {
            memcpy(&VRAM_D[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x06600000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_D) == 2)) {
            memcpy(&VRAM_I[addr & 0x3FFF], &data, sizeof(T));
        }
    }
}

template auto GPU::ReadOBJB(u32 addr) -> u8;
template auto GPU::ReadOBJB(u32 addr) -> u16;
template auto GPU::ReadOBJB(u32 addr) -> u32;
template <typename T>
auto GPU::ReadOBJB(u32 addr) -> T {
    T return_value = 0;

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06600000, 0x20000) && (GetVRAMCNTMST(VRAMCNT_D) == 4)) {
            memcpy(&return_value, &VRAM_D[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x06600000, 0x4000) && (GetVRAMCNTMST(VRAMCNT_D) == 2)) {
            memcpy(&return_value, &VRAM_I[addr & 0x3FFF], sizeof(T));
        }
    }

    return return_value;
}

template auto GPU::ReadOBJA(u32 addr) -> u8;
template auto GPU::ReadOBJA(u32 addr) -> u16;
template auto GPU::ReadOBJA(u32 addr) -> u32;
template <typename T>
auto GPU::ReadOBJA(u32 addr) -> T {
    T return_value = 0;

    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06400000 + ((GetVRAMCNTOffset(VRAMCNT_A) & 0x1) * 0x20000), 0x20000) && (GetVRAMCNTMST(VRAMCNT_A) == 2)) {
            memcpy(&return_value, &VRAM_A[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06400000 + ((GetVRAMCNTOffset(VRAMCNT_B) & 0x1) * 0x20000), 0x20000) && (GetVRAMCNTMST(VRAMCNT_B) == 2)) {
            memcpy(&return_value, &VRAM_B[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06400000, 0x10000) && (GetVRAMCNTMST(VRAMCNT_E) == 2)) {
            memcpy(&return_value, &VRAM_E[addr & 0xFFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06400000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x2)), 0x3FFF) && (GetVRAMCNTMST(VRAMCNT_F) == 2)) {
            memcpy(&return_value, &VRAM_F[addr & 0x3FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06400000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x2)), 0x3FFF) && (GetVRAMCNTMST(VRAMCNT_G) == 2)) {
            memcpy(&return_value, &VRAM_G[addr & 0x3FFF], sizeof(T));
        }
    }

    return return_value;
}

template auto GPU::ReadExtPaletteBGA(u32 addr) -> u8;
template auto GPU::ReadExtPaletteBGA(u32 addr) -> u16;
template auto GPU::ReadExtPaletteBGA(u32 addr) -> u32;
template <typename T>
auto GPU::ReadExtPaletteBGA(u32 addr) -> T {
    T return_value = 0;

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        // only lower 32kb are used
        // vram bank e can then hold all 4 8kb slots
        if (in_range(0, 0x8000) && (GetVRAMCNTMST(VRAMCNT_E) == 4)) {
            memcpy(&return_value, &VRAM_E[addr & 0xFFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        u32 offset = GetVRAMCNTOffset(VRAMCNT_F) & 0x1 ? 0x4000 : 0;
        if (in_range(offset, 0x4000) && (GetVRAMCNTMST(VRAMCNT_F) == 4)) {
            memcpy(&return_value, &VRAM_F[addr & 0x3FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        u32 offset = GetVRAMCNTOffset(VRAMCNT_G) & 0x1 ? 0x4000 : 0;
        if (in_range(offset, 0x4000) && (GetVRAMCNTMST(VRAMCNT_G) == 4)) {
            memcpy(&return_value, &VRAM_G[addr & 0x3FFF], sizeof(T));
        }
    }

    return return_value;
}

template <typename T>
auto GPU::ReadExtPaletteBGB(u32 addr) -> T {
    T return_value = 0;

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        // vram bank h can cover all slots 0-3
        if (in_range(0, 0x8000) && (GetVRAMCNTMST(VRAMCNT_H) == 2)) {
            memcpy(&return_value, &VRAM_H[addr & 0x7FFF], sizeof(T));
        }
    }

    return return_value;
}