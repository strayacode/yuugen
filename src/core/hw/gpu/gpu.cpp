#include <core/hw/gpu/gpu.h>
#include <core/core.h>

GPU::GPU(Core* core) : core(core), engine_a(this, 1), engine_b(this , 0), render_engine(this), geometry_engine(this) {

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
    geometry_engine.Reset();
    render_engine.Reset();

    // schedule the first RenderScanlineStart event
    // we would expect hblank to start at 1536 (256 * 6) cycles
    // but actually the hblank flag stays 0 until 1606 cycles have passed
    core->scheduler.Add(1606, RenderScanlineStartTask);

    MapVRAM();
}

void GPU::RenderScanlineStart() {
    if (VCOUNT < 192) {
        engine_a.RenderScanline(VCOUNT);
        engine_b.RenderScanline(VCOUNT);

        // trigger an arm9 dma transfer on hblank (only for visible scanlines)
        core->dma[1].Trigger(2);
    }

    // TODO: try to only do this when 3d is enabled
    if (VCOUNT == 215) {
        // 3d render engine renders 48 scanlines early in vblank period
        render_engine.Render();
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

        // swap buffers
        core->gpu.geometry_engine.DoSwapBuffers();

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

void GPU::MapVRAM() {
    // we will map vram blocks in increments of 4kb
    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        u8 ofs = GetVRAMCNTOffset(VRAMCNT_I);
        switch (GetVRAMCNTMST(VRAMCNT_I)) {
        case 0:
            for (int i = 0; i < 4; i++) {
                lcdc[i + 0xA0] = &VRAM_I[i * 0x1000];
            }
            break;
        case 3:
            // obj extended palette
            // handle later
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(VRAMCNT_I));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_H)) {
        u8 ofs = GetVRAMCNTOffset(VRAMCNT_H);
        switch (GetVRAMCNTMST(VRAMCNT_H)) {
        case 0:
            for (int i = 0; i < 8; i++) {
                lcdc[i + 0x98] = &VRAM_H[i * 0x1000];
            }
            break;
        case 1:
            for (int i = 0; i < 8; i++) {
                bgb[i] = &VRAM_H[i * 0x1000];
            }
            break;
        case 2:
            // handle extpal later
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(VRAMCNT_H));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        u8 ofs = GetVRAMCNTOffset(VRAMCNT_G);
        switch (GetVRAMCNTMST(VRAMCNT_G)) {
        case 0:
            for (int i = 0; i < 4; i++) {
                lcdc[i + 0x94] = &VRAM_G[i * 0x1000];
            }
            break;
        case 1:
            for (int i = 0; i < 4; i++) {
                bga[(ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10 + i] = &VRAM_G[i * 0x1000];
            }
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(VRAMCNT_G));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        u8 ofs = GetVRAMCNTOffset(VRAMCNT_F);
        switch (GetVRAMCNTMST(VRAMCNT_F)) {
        case 0:
            for (int i = 0; i < 4; i++) {
                lcdc[i + 0x90] = &VRAM_F[i * 0x1000];
            }
            break;
        case 1:
            for (int i = 0; i < 4; i++) {
                bga[(ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10 + i] = &VRAM_F[i * 0x1000];
            }
            break;
        case 5:
            // obj ext palette handle later
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(VRAMCNT_F));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_E)) {
        u8 ofs = GetVRAMCNTOffset(VRAMCNT_E);
        switch (GetVRAMCNTMST(VRAMCNT_E)) {
        case 0:
            for (int i = 0; i < 16; i++) {
                lcdc[i + 0x80] = &VRAM_E[i * 0x1000];
            }
            break;
        case 1:
            for (int i = 0; i < 16; i++) {
                bga[i] = &VRAM_E[i * 0x1000];
            }
            break;
        case 4:
            // handle extpal later
            // for (int i = 0; i < 16; i++) {
            //     lcdc[i + 0x80] = &VRAM_E[i * 0x1000];
            // }
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(VRAMCNT_E));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_D)) {
        u8 ofs = GetVRAMCNTOffset(VRAMCNT_D);
        switch (GetVRAMCNTMST(VRAMCNT_D)) {
        case 0:
            for (int i = 0; i < 32; i++) {
                lcdc[i + 0x60] = &VRAM_D[i * 0x1000];
            }
            break;
        case 1:
            for (int i = 0; i < 32; i++) {
                bga[(ofs * 0x20) + i] = &VRAM_D[i * 0x1000];
            }
            break;
        case 4:
            for (int i = 0; i < 32; i++) {
                objb[i] = &VRAM_D[i * 0x1000];
            }
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(VRAMCNT_D));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_C)) {
        u8 ofs = GetVRAMCNTOffset(VRAMCNT_C);
        switch (GetVRAMCNTMST(VRAMCNT_C)) {
        case 0:
            for (int i = 0; i < 32; i++) {
                lcdc[i + 0x40] = &VRAM_C[i * 0x1000];
            }
            break;
        case 1:
            for (int i = 0; i < 32; i++) {
                bga[(ofs * 0x20) + i] = &VRAM_C[i * 0x1000];
            }
            break;
        case 4:
            for (int i = 0; i < 32; i++) {
                bgb[i] = &VRAM_C[i * 0x1000];
            }
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(VRAMCNT_C));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_B)) {
        u8 ofs = GetVRAMCNTOffset(VRAMCNT_B);
        switch (GetVRAMCNTMST(VRAMCNT_B)) {
        case 0:
            for (int i = 0; i < 32; i++) {
                lcdc[i + 0x20] = &VRAM_B[i * 0x1000];
            }
            break;
        case 1:
            for (int i = 0; i < 32; i++) {
                bga[(ofs * 0x20) + i] = &VRAM_B[i * 0x1000];
            }
            break;
        case 2:
            for (int i = 0; i < 32; i++) {
                obja[((ofs & 0x1) * 0x20) + i] = &VRAM_B[i * 0x1000];
            }
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(VRAMCNT_B));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_A)) {
        u8 ofs = GetVRAMCNTOffset(VRAMCNT_A);
        switch (GetVRAMCNTMST(VRAMCNT_A)) {
        case 0:
            for (int i = 0; i < 32; i++) {
                lcdc[i] = &VRAM_A[i * 0x1000];
            }
            break;
        case 1:
            for (int i = 0; i < 32; i++) {
                bga[(ofs * 0x20) + i] = &VRAM_A[i * 0x1000];
            }
            break;
        case 2:
            for (int i = 0; i < 32; i++) {
                obja[((ofs & 0x1) * 0x20) + i] = &VRAM_A[i * 0x1000];
            }
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(VRAMCNT_A));
        }
    }
}

template auto GPU::ReadVRAM(u32 addr) -> u8;
template auto GPU::ReadVRAM(u32 addr) -> u16;
template auto GPU::ReadVRAM(u32 addr) -> u32;
template <typename T>
auto GPU::ReadVRAM(u32 addr) -> T {
    T return_value = 0;

    u8 region = (addr >> 20) & 0xF;

    u32 offset = addr - (region * 0x100000) - 0x06000000;

    int page = offset >> 12;
    int index = offset & 0xFFF;

    switch (region) {
    case 0x0: case 0x1:
        memcpy(&return_value, &bga[page][index], sizeof(T));
        break;
    case 0x2: case 0x3:
        memcpy(&return_value, &bgb[page][index], sizeof(T));
        break;
    case 0x4: case 0x5:
        memcpy(&return_value, &obja[page][index], sizeof(T));
        break;
    case 0x6: case 0x7:
        memcpy(&return_value, &objb[page][index], sizeof(T));
        break;
    default:
        memcpy(&return_value, &lcdc[page][index], sizeof(T));
        break;
    }

    return return_value;
}

template void GPU::WriteVRAM(u32 addr, u8 data);
template void GPU::WriteVRAM(u32 addr, u16 data);
template void GPU::WriteVRAM(u32 addr, u32 data);
template <typename T>
void GPU::WriteVRAM(u32 addr, T data) {
    u8 region = (addr >> 20) & 0xF;

    u32 offset = addr - (region * 0x100000) - 0x06000000;

    int page_index = offset >> 12;
    int index = offset & 0xFFF;

    u8* page = nullptr;

    switch (region) {
    case 0x0: case 0x1:
        page = bga[page_index];
        break;
    case 0x2: case 0x3:
        page = bgb[page_index];
        break;
    case 0x4: case 0x5:
        page = obja[page_index];
        break;
    case 0x6: case 0x7:
        page = objb[page_index];
        break;
    default:
        page = lcdc[page_index];
        break;
    }

    // make sure the page has been mapped
    if (page != nullptr) {
        memcpy(&page[index], &data, sizeof(T));
    }
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

template auto GPU::ReadExtPaletteBGB(u32 addr) -> u8;
template auto GPU::ReadExtPaletteBGB(u32 addr) -> u16;
template auto GPU::ReadExtPaletteBGB(u32 addr) -> u32;
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

template auto GPU::ReadExtPaletteOBJA(u32 addr) -> u8;
template auto GPU::ReadExtPaletteOBJA(u32 addr) -> u16;
template auto GPU::ReadExtPaletteOBJA(u32 addr) -> u32;
template <typename T>
auto GPU::ReadExtPaletteOBJA(u32 addr) -> T {
    T return_value = 0;

    // only the lower 8kb of a vram bank is used, since for objs only one 8kb slot is used
    if (GetVRAMCNTEnabled(VRAMCNT_F)) {
        if (in_range(0, 0x2000) && (GetVRAMCNTMST(VRAMCNT_F) == 5)) {
            memcpy(&return_value, &VRAM_F[addr & 0x1FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(VRAMCNT_G)) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        if (in_range(0, 0x2000) && (GetVRAMCNTMST(VRAMCNT_G) == 5)) {
            memcpy(&return_value, &VRAM_G[addr & 0x1FFF], sizeof(T));
        }
    }

    return return_value;
}

template auto GPU::ReadExtPaletteOBJB(u32 addr) -> u8;
template auto GPU::ReadExtPaletteOBJB(u32 addr) -> u16;
template auto GPU::ReadExtPaletteOBJB(u32 addr) -> u32;
template <typename T>
auto GPU::ReadExtPaletteOBJB(u32 addr) -> T {
    T return_value = 0;

    // only the lower 8kb of a vram bank is used, since for objs only one 8kb slot is used
    if (GetVRAMCNTEnabled(VRAMCNT_I)) {
        if (in_range(0, 0x2000) && (GetVRAMCNTMST(VRAMCNT_I) == 3)) {
            memcpy(&return_value, &VRAM_I[addr & 0x1FFF], sizeof(T));
        }
    }

    return return_value;
}