#include <core/core.h>
#include <core/gpu.h>

GPU::GPU(Core* core) : core(core), engine_a(this, 1), engine_b(this, 0) {

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

    engine_a.Reset();
    engine_b.Reset();
}

void GPU::WriteDISPSTAT7(u16 data) {
    // preserve non writeable bits too 
    DISPSTAT7 = (DISPSTAT7 & ~0xFFB8) | (data & 0xFFB8);
}



void GPU::WriteDISPSTAT9(u16 data) {
    // preserve non writeable bits too
    DISPSTAT9 = (DISPSTAT9 & ~0xFFB8) | (data & 0xFFB8);
}

const u32* GPU::GetFramebuffer(Screen screen) {
    switch (screen) {
    case Screen::TOP_SCREEN:
        return ((POWCNT1 & (1 << 15)) ? engine_a.GetFramebuffer() : engine_b.GetFramebuffer());
    default:
        // default is bottom screen but we just have this to avoid a warning of no return value
        return ((POWCNT1 & (1 << 15)) ? engine_b.GetFramebuffer() : engine_a.GetFramebuffer());
    }
}

void GPU::RenderScanlineStart() {
    // since hblank has started we can render a scanline to the framebuffer
    if (VCOUNT < 192) {
        engine_a.RenderScanline(VCOUNT);
        engine_b.RenderScanline(VCOUNT);

        // trigger an arm9 dma transfer on hblank (only for visible scanlines)
        core->dma[1].Trigger(2);
    }

    // set the hblank flag as hblank has started
    DISPSTAT7 |= (1 << 1);

    if (DISPSTAT7 & (1 << 4)) {
        core->arm7.SendInterrupt(1);
    }

    DISPSTAT9 |= (1 << 1);

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
}

void GPU::RenderScanlineFinish() {
    switch (++VCOUNT) {
    case 192:
        // start of vblank as all 192 visible scanlines have been rendered
        DISPSTAT7 |= 1;

        if (DISPSTAT7 & (1 << 3)) {
            core->arm7.SendInterrupt(0);
        }

        // trigger an arm7 vblank dma transfer
        core->dma[0].Trigger(1);

        DISPSTAT9 |= 1;

        if (DISPSTAT9 & (1 << 3)) {
            core->arm9.SendInterrupt(0);
        }

        // trigger an arm9 vblank dma transfer
        core->dma[1].Trigger(1);
        break;
    case 262:
        // last scanline so we clear vblank flag because end of vblank
        DISPSTAT7 &= ~1;
        DISPSTAT9 &= ~1;

        break;
    case 263:
        // end of frame so reset VCOUNT
        VCOUNT = 0;
        break;
    }

    if (((DISPSTAT7 >> 8) | ((DISPSTAT7 & (1 << 7)) << 1)) == VCOUNT) {
        // set the v counter flag
        DISPSTAT7 |= (1 << 2);

        // also request a v counter irq if enabled
        if (DISPSTAT7 & (1 << 5)) {
            core->arm7.SendInterrupt(2);
        }
    } else if (DISPSTAT7 & (1 << 2)) {
        // reset v counter flag on next line as VCOUNT will not be the same as the LYC setting and thus v counter flag must be reset
        DISPSTAT7 &= ~(1 << 2);
    }

    if (((DISPSTAT9 >> 8) | ((DISPSTAT9 & (1 << 7)) << 1)) == VCOUNT) {
        // set the v counter flag
        DISPSTAT9 |= (1 << 2);

        // also request a v counter irq if enabled
        if (DISPSTAT9 & (1 << 5)) {
            core->arm9.SendInterrupt(2);
        }
    } else if (DISPSTAT9 & (1 << 2)) {
        // reset v counter flag on next line as VCOUNT will not be the same as the LYC setting and thus v counter flag must be reset
        DISPSTAT9 &= ~(1 << 2);
    }

    // clear the hblank flag as we are now at the end of the scanline
    DISPSTAT7 &= ~(1 << 1);
    DISPSTAT9 &= ~(1 << 1);
}

int GPU::GetVRAMCNTMST(u8 vramcnt) {
    return vramcnt & 0x7;
}

int GPU::GetVRAMCNTOffset(u8 vramcnt) {
    return (vramcnt >> 3) & 0x3;
}

bool GPU::GetVRAMCNTEnabled(u8 vramcnt) {
    return (vramcnt & (1 << 7));
}

void GPU::WriteLCDC(u32 addr, u16 data) {
    // small optimisation by checking enabled first instead of in a range as it should be faster?
    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06800000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_A) == 0)) {
            memcpy(&VRAM_A[addr & 0x1FFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06820000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_B) == 0)) {
            memcpy(&VRAM_B[addr & 0x1FFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06840000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_C) == 0)) {
            memcpy(&VRAM_C[addr & 0x1FFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06860000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_D) == 0)) {
            memcpy(&VRAM_D[addr & 0x1FFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06880000, 0x10000, addr) && (GetVRAMCNTMST(VRAMCNT_E) == 0)) {
            memcpy(&VRAM_E[addr & 0xFFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06890000, 0x4000, addr) && (GetVRAMCNTMST(VRAMCNT_F) == 0)) {
            memcpy(&VRAM_F[addr & 0x3FFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06894000, 0x4000, addr) && (GetVRAMCNTMST(VRAMCNT_G) == 0)) {
            memcpy(&VRAM_G[addr & 0x3FFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        if (in_range(0x06898000, 0x8000, addr) && (GetVRAMCNTMST(VRAMCNT_H) == 0)) {
            memcpy(&VRAM_H[addr & 0x7FFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x068A0000, 0x4000, addr) && (GetVRAMCNTMST(VRAMCNT_I) == 0)) {
            memcpy(&VRAM_I[addr & 0x3FFF], &data, 2);
        }
    }
}

u16 GPU::ReadLCDC(u32 addr) {
    u16 return_value = 0;

    // small optimisation by checking enabled first instead of in a range as it should be faster?
    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06800000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_A) == 0)) {
            memcpy(&return_value, &VRAM_A[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06820000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_B) == 0)) {
            memcpy(&return_value, &VRAM_B[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06840000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_C) == 0)) {
            memcpy(&return_value, &VRAM_C[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06860000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_D) == 0)) {
            memcpy(&return_value, &VRAM_D[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06880000, 0x10000, addr) && (GetVRAMCNTMST(VRAMCNT_E) == 0)) {
            memcpy(&return_value, &VRAM_E[addr & 0xFFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06890000, 0x4000, addr) && (GetVRAMCNTMST(VRAMCNT_F) == 0)) {
            memcpy(&return_value, &VRAM_F[addr & 0x3FFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06894000, 0x4000, addr) && (GetVRAMCNTMST(VRAMCNT_G) == 0)) {
            memcpy(&return_value, &VRAM_G[addr & 0x3FFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        if (in_range(0x06898000, 0x8000, addr) && (GetVRAMCNTMST(VRAMCNT_H) == 0)) {
            memcpy(&return_value, &VRAM_H[addr & 0x7FFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x068A0000, 0x4000, addr) && (GetVRAMCNTMST(VRAMCNT_I) == 0)) {
            memcpy(&return_value, &VRAM_I[addr & 0x3FFF], 2);
        }
    }

    return return_value;
}

void GPU::WriteBGA(u32 addr, u16 data) {
    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_A)), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_A) == 1)) {
            memcpy(&VRAM_A[addr & 0x1FFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_B)), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_B) == 1)) {
            memcpy(&VRAM_B[addr & 0x1FFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_C)), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_C) == 1)) {
            memcpy(&VRAM_C[addr & 0x1FFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_D)), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_D) == 1)) {
            memcpy(&VRAM_D[addr & 0x1FFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06000000, 0x10000, addr) && (GetVRAMCNTMST(VRAMCNT_E) == 1)) {
            memcpy(&VRAM_E[addr & 0xFFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06000000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x2)), 0x3FFF, addr) && (GetVRAMCNTMST(VRAMCNT_F) == 1)) {
            memcpy(&VRAM_F[addr & 0x3FFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06000000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x2)), 0x3FFF, addr) && (GetVRAMCNTMST(VRAMCNT_G) == 1)) {
            memcpy(&VRAM_G[addr & 0x3FFF], &data, 2);
        }
    }
}

u16 GPU::ReadBGA(u32 addr) {
    u16 return_value = 0;

    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_A)), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_A) == 1)) {
            memcpy(&return_value, &VRAM_A[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_B)), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_B) == 1)) {
            memcpy(&return_value, &VRAM_B[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_C)), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_C) == 1)) {
            memcpy(&return_value, &VRAM_C[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06000000 + (0x20000 * GetVRAMCNTOffset(VRAMCNT_D)), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_D) == 1)) {
            memcpy(&return_value, &VRAM_D[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        if (in_range(0x06000000, 0x10000, addr) && (GetVRAMCNTMST(VRAMCNT_E) == 1)) {
            memcpy(&return_value, &VRAM_E[addr & 0xFFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0x06000000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_F) & 0x2)), 0x3FFF, addr) && (GetVRAMCNTMST(VRAMCNT_F) == 1)) {
            memcpy(&return_value, &VRAM_F[addr & 0x3FFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        if (in_range(0x06000000 + (0x4000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x1)) + (0x10000 * (GetVRAMCNTOffset(VRAMCNT_G) & 0x2)), 0x3FFF, addr) && (GetVRAMCNTMST(VRAMCNT_G) == 1)) {
            memcpy(&return_value, &VRAM_G[addr & 0x3FFF], 2);
        }
    }

    return return_value;
}

void GPU::WriteBGB(u32 addr, u16 data) {
    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06200000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_C) == 4)) {
            memcpy(&VRAM_C[addr & 0x1FFFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        if (in_range(0x06200000, 0x8000, addr) && (GetVRAMCNTMST(VRAMCNT_H) == 1)) {
            memcpy(&VRAM_H[addr & 0x7FFF], &data, 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x06208000, 0x4000, addr) && (GetVRAMCNTMST(VRAMCNT_I) == 1)) {
            memcpy(&VRAM_I[addr & 0x3FFF], &data, 2);
        }
    }
}

u16 GPU::ReadBGB(u32 addr) {
    u16 return_value = 0;

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06200000, 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_C) == 4)) {
            memcpy(&return_value, &VRAM_C[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        if (in_range(0x06200000, 0x8000, addr) && (GetVRAMCNTMST(VRAMCNT_H) == 1)) {
            memcpy(&return_value, &VRAM_H[addr & 0x7FFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0x06208000, 0x4000, addr) && (GetVRAMCNTMST(VRAMCNT_I) == 1)) {
            memcpy(&return_value, &VRAM_I[addr & 0x3FFF], 2);
        }
    }

    return return_value;
}

u16 GPU::ReadARM7(u32 addr) {
    u16 return_value = 0;
    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(VRAMCNT_C) * 0x20000), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_C) == 2)) {
            memcpy(&return_value, &VRAM_C[addr & 0x1FFFF], 2);
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(VRAMCNT_D) * 0x20000), 0x20000, addr) && (GetVRAMCNTMST(VRAMCNT_D) == 2)) {
            memcpy(&return_value, &VRAM_D[addr & 0x1FFFF], 2);
        }
    }

    return return_value;
}