#include <core/hw/gpu/gpu.h>
#include <core/hw/hw.h>

GPU::GPU(HW* hw) : hw(hw), engine_a(this, 1), engine_b(this , 0), render_engine(this), geometry_engine(this) {

}

void GPU::Reset() {
    memset(bank_a, 0, 0x20000);
    memset(bank_b, 0, 0x20000);
    memset(bank_c, 0, 0x20000);
    memset(bank_d, 0, 0x20000);
    memset(bank_e, 0, 0x10000);
    memset(bank_f, 0, 0x4000);
    memset(bank_g, 0, 0x4000);
    memset(bank_h, 0, 0x8000);
    memset(bank_i, 0, 0x4000);

    POWCNT1 = 0;
    VCOUNT = 0;

    for (int i = 0; i < 9; i++) {
        vramcnt[i] = 0;
    }

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
    hw->scheduler.Add(1606, RenderScanlineStartTask);

    MapVRAM();
}

void GPU::RenderScanlineStart() {
    if (VCOUNT < 192) {
        engine_a.RenderScanline(VCOUNT);
        engine_b.RenderScanline(VCOUNT);

        // trigger an arm9 dma transfer on hblank (only for visible scanlines)
        hw->dma[1].Trigger(2);
    }

    DISPSTAT7 |= (1 << 1);
    DISPSTAT9 |= (1 << 1);
    
    if (DISPSTAT7 & (1 << 4)) {
        hw->cpu_core[0]->SendInterrupt(1);
    }

    if (DISPSTAT9 & (1 << 4)) {
        hw->cpu_core[1]->SendInterrupt(1);
    }

    if (VCOUNT == 215) {
        render_engine.Render();
    }

    // ARM9 DMA exclusive
    // check if scanline is between 2 and 193 inclusive
    // if so trigger a start of display dma transfer
    // TODO: on scanline 194 automatically clear the enable bit in DMA
    if ((VCOUNT > 1) && (VCOUNT < 194)) {
        hw->dma[1].Trigger(3);
    }

    hw->scheduler.Add(524, RenderScanlineFinishTask);
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
            hw->cpu_core[0]->SendInterrupt(0);
        }

        if (DISPSTAT9 & (1 << 3)) {
            hw->cpu_core[1]->SendInterrupt(0);
        }

        hw->dma[0].Trigger(1);
        hw->dma[1].Trigger(1);
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
            hw->cpu_core[0]->SendInterrupt(2);
        }

    } else if (DISPSTAT7 & (1 << 2)) {
        DISPSTAT7 &= ~(1 << 2);
    }

    if (((DISPSTAT9 >> 8) | ((DISPSTAT9 & (1 << 7)) << 1)) == VCOUNT) {
        DISPSTAT9 |= (1 << 2);

        if (DISPSTAT9 & (1 << 5)) {
            hw->cpu_core[1]->SendInterrupt(2);
        }

    } else if (DISPSTAT9 & (1 << 2)) {
        DISPSTAT9 &= ~(1 << 2);
    }

    hw->scheduler.Add(1606, RenderScanlineStartTask);
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
    case Screen::Top:
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
    // reset all the vram pages
    VRAMMappingReset();

    // we will map vram blocks in increments of 4kb
    if (GetVRAMCNTEnabled(vramcnt[0])) {
        u8 ofs = GetVRAMCNTOffset(vramcnt[0]);
        switch (GetVRAMCNTMST(vramcnt[0])) {
        case 0:
            for (int i = 0; i < 32; i++) {
                lcdc[i].AddBank(&bank_a[i * 0x1000]);
            }
            break;
        case 1:
            for (int i = 0; i < 32; i++) {
                bga[(ofs * 0x20) + i].AddBank(&bank_a[i * 0x1000]);
            }
            break;
        case 2:
            for (int i = 0; i < 32; i++) {
                obja[((ofs & 0x1) * 0x20) + i].AddBank(&bank_a[i * 0x1000]);
            }
            break;
        case 3:
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(vramcnt[0]));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[1])) {
        u8 ofs = GetVRAMCNTOffset(vramcnt[1]);
        switch (GetVRAMCNTMST(vramcnt[1])) {
        case 0:
            for (int i = 0; i < 32; i++) {
                lcdc[i + 0x20].AddBank(&bank_b[i * 0x1000]);
            }
            break;
        case 1:
            for (int i = 0; i < 32; i++) {
                bga[(ofs * 0x20) + i].AddBank(&bank_b[i * 0x1000]);
            }
            break;
        case 2:
            for (int i = 0; i < 32; i++) {
                obja[((ofs & 0x1) * 0x20) + i].AddBank(&bank_b[i * 0x1000]);
            }
            break;
        case 3:
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(vramcnt[1]));
        }
    }
    
    if (GetVRAMCNTEnabled(vramcnt[2])) {
        u8 ofs = GetVRAMCNTOffset(vramcnt[2]);
        switch (GetVRAMCNTMST(vramcnt[2])) {
        case 0:
            for (int i = 0; i < 32; i++) {
                lcdc[i + 0x40].AddBank(&bank_c[i * 0x1000]);
            }
            break;
        case 1:
            for (int i = 0; i < 32; i++) {
                bga[(ofs * 0x20) + i].AddBank(&bank_c[i * 0x1000]);
            }
            break;
        case 2:
        //     for (int i = 0; i < 32; i++) {
        //         arm7_vram[(ofs & 0x1) * 0x20 + i].AddBank(&bank_c[i * 0x1000]);
        //     }
            break;
        case 3:
            // handle texture later
            break;
        case 4:
            for (int i = 0; i < 32; i++) {
                bgb[i].AddBank(&bank_c[i * 0x1000]);
            }
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(vramcnt[2]));
        }
    }
    
    if (GetVRAMCNTEnabled(vramcnt[3])) {
        u8 ofs = GetVRAMCNTOffset(vramcnt[3]);
        switch (GetVRAMCNTMST(vramcnt[3])) {
        case 0:
            for (int i = 0; i < 32; i++) {
                lcdc[i + 0x60].AddBank(&bank_d[i * 0x1000]);
            }
            break;
        case 1:
            for (int i = 0; i < 32; i++) {
                bga[(ofs * 0x20) + i].AddBank(&bank_d[i * 0x1000]);
            }
            break;
        case 2:
            // for (int i = 0; i < 32; i++) {
            //     arm7_vram[(ofs & 0x1) * 0x20 + i].AddBank(&bank_d[i * 0x1000]);
            // }
            break;
        case 3:
            // handle texture later
            break;
        case 4:
            for (int i = 0; i < 32; i++) {
                objb[i].AddBank(&bank_d[i * 0x1000]);
            }
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(vramcnt[3]));
        }
    }
    
    if (GetVRAMCNTEnabled(vramcnt[4])) {
        u8 ofs = GetVRAMCNTOffset(vramcnt[4]);
        switch (GetVRAMCNTMST(vramcnt[4])) {
        case 0:
            for (int i = 0; i < 16; i++) {
                lcdc[i + 0x80].AddBank(&bank_e[i * 0x1000]);
            }
            break;
        case 1:
            for (int i = 0; i < 16; i++) {
                bga[i].AddBank(&bank_e[i * 0x1000]);
            }
            break;
        case 2:
            for (int i = 0; i < 16; i++) {
                obja[i].AddBank(&bank_e[i * 1000]);
            }
            break;
        case 3:
            // handle texture later
            break;
        case 4:
            // handle extpal later
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(vramcnt[4]));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[5])) {
        u8 ofs = GetVRAMCNTOffset(vramcnt[5]);
        switch (GetVRAMCNTMST(vramcnt[5])) {
        case 0:
            for (int i = 0; i < 4; i++) {
                lcdc[i + 0x90].AddBank(&bank_f[i * 0x1000]);
            }
            break;
        case 1:
            for (int i = 0; i < 4; i++) {
                bga[(ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10 + i].AddBank(&bank_f[i * 0x1000]);
            }
            break;
        case 2:
            for (int i = 0; i < 4; i++) {
                obja[(ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10 + i].AddBank(&bank_f[i * 0x1000]);
            }
            break;
        case 3:
            break;
        case 4:
            // handle ext palette later
            break;
        case 5:
            // obj ext palette handle later
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(vramcnt[5]));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[6])) {
        u8 ofs = GetVRAMCNTOffset(vramcnt[6]);
        switch (GetVRAMCNTMST(vramcnt[6])) {
        case 0:
            for (int i = 0; i < 4; i++) {
                lcdc[i + 0x94].AddBank(&bank_g[i * 0x1000]);
            }
            break;
        case 1:
            for (int i = 0; i < 4; i++) {
                bga[(ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10 + i].AddBank(&bank_g[i * 0x1000]);
            }
            break;
        case 2:
            for (int i = 0; i < 4; i++) {
                obja[(ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10 + i].AddBank(&bank_g[i * 0x1000]);
            }
            break;
        case 3:
            break;
        case 4:
            // handle extended palette later
            break;
        case 5:
            // handle extended palette later
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(vramcnt[6]));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[7])) {
        u8 ofs = GetVRAMCNTOffset(vramcnt[7]);
        switch (GetVRAMCNTMST(vramcnt[7])) {
        case 0:
            for (int i = 0; i < 8; i++) {
                lcdc[i + 0x98].AddBank(&bank_h[i * 0x1000]);
            }
            break;
        case 1:
            for (int i = 0; i < 8; i++) {
                bgb[i].AddBank(&bank_h[i * 0x1000]);
            }
            break;
        case 2:
            // handle extpal later
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(vramcnt[7]));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[8])) {
        u8 ofs = GetVRAMCNTOffset(vramcnt[8]);
        switch (GetVRAMCNTMST(vramcnt[8])) {
        case 0:
            for (int i = 0; i < 4; i++) {
                lcdc[i + 0xA0].AddBank(&bank_i[i * 0x1000]);
            }
            break;
        case 1:
            for (int i = 0; i < 4; i++) {
                bgb[i + 0x8].AddBank(&bank_i[i * 0x1000]);
            }
            break;
        case 2:
            for (int i = 0; i < 4; i++) {
                objb[i].AddBank(&bank_i[i * 0x1000]);
            }
            break;
        case 3:
            // obj extended palette
            // handle later
            break;
        default:
            log_fatal("handle mst %d", GetVRAMCNTMST(vramcnt[8]));
        }
    }
}

void GPU::VRAMMappingReset() {
    for (int i = 0; i < 0xA4; i++) {
        lcdc[i].Reset();
    }

    for (int i = 0; i < 0x80; i++) {
        bga[i].Reset();
    }

    for (int i = 0; i < 0x40; i++) {
        obja[i].Reset();
    }

    for (int i = 0; i < 0x20; i++) {
        bgb[i].Reset();
    }

    for (int i = 0; i < 0x20; i++) {
        objb[i].Reset();
    }

    for (int i = 0; i < 0x20; i++) {
        arm7_vram[i].Reset();
    }
}

template auto GPU::ReadVRAM(u32 addr) -> u8;
template auto GPU::ReadVRAM(u32 addr) -> u16;
template auto GPU::ReadVRAM(u32 addr) -> u32;
template <typename T>
auto GPU::ReadVRAM(u32 addr) -> T {
    u8 region = (addr >> 20) & 0xF;

    u32 offset = addr - (region * 0x100000) - 0x06000000;

    int page_index = offset >> 12;

    VRAMPage* page = nullptr;

    switch (region) {
    case 0x0: case 0x1:
        page = &bga[page_index];
        break;
    case 0x2: case 0x3:
        page = &bgb[page_index];
        break;
    case 0x4: case 0x5:
        page = &obja[page_index];
        break;
    case 0x6: case 0x7:
        page = &objb[page_index];
        break;
    default:
        page = &lcdc[page_index];
        break;
    }

    if (page == nullptr) {
        log_fatal("handle");
    }

    return page->Read<T>(addr);
}

template void GPU::WriteVRAM(u32 addr, u8 data);
template void GPU::WriteVRAM(u32 addr, u16 data);
template void GPU::WriteVRAM(u32 addr, u32 data);
template <typename T>
void GPU::WriteVRAM(u32 addr, T data) {
    u8 region = (addr >> 20) & 0xF;

    u32 offset = addr - (region * 0x100000) - 0x06000000;

    int page_index = offset >> 12;

    VRAMPage* page = nullptr;

    switch (region) {
    case 0x0: case 0x1:
        page = &bga[page_index];
        break;
    case 0x2: case 0x3:
        page = &bgb[page_index];
        break;
    case 0x4: case 0x5:
        page = &obja[page_index];
        break;
    case 0x6: case 0x7:
        page = &objb[page_index];
        break;
    default:
        page = &lcdc[page_index];
        break;
    }

    if (page == nullptr) {
        log_fatal("handle");
    }

    page->Write<T>(addr, data);
}

template auto GPU::ReadExtPaletteBGA(u32 addr) -> u8;
template auto GPU::ReadExtPaletteBGA(u32 addr) -> u16;
template auto GPU::ReadExtPaletteBGA(u32 addr) -> u32;
template <typename T>
auto GPU::ReadExtPaletteBGA(u32 addr) -> T {
    T return_value = 0;

    if (GetVRAMCNTEnabled(vramcnt[4])) {
        // only lower 32kb are used
        // vram bank e can then hold all 4 8kb slots
        if (in_range(0, 0x8000) && (GetVRAMCNTMST(vramcnt[4]) == 4)) {
            memcpy(&return_value, &bank_e[addr & 0xFFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[5])) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        u32 offset = GetVRAMCNTOffset(vramcnt[5]) & 0x1 ? 0x4000 : 0;
        if (in_range(offset, 0x4000) && (GetVRAMCNTMST(vramcnt[5]) == 4)) {
            memcpy(&return_value, &bank_f[addr & 0x3FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[6])) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        u32 offset = GetVRAMCNTOffset(vramcnt[6]) & 0x1 ? 0x4000 : 0;
        if (in_range(offset, 0x4000) && (GetVRAMCNTMST(vramcnt[6]) == 4)) {
            memcpy(&return_value, &bank_g[addr & 0x3FFF], sizeof(T));
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

    if (GetVRAMCNTEnabled(vramcnt[7])) {
        // vram bank h can cover all slots 0-3
        if (in_range(0, 0x8000) && (GetVRAMCNTMST(vramcnt[7]) == 2)) {
            memcpy(&return_value, &bank_h[addr & 0x7FFF], sizeof(T));
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
    if (GetVRAMCNTEnabled(vramcnt[5])) {
        if (in_range(0, 0x2000) && (GetVRAMCNTMST(vramcnt[5]) == 5)) {
            memcpy(&return_value, &bank_f[addr & 0x1FFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[6])) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        if (in_range(0, 0x2000) && (GetVRAMCNTMST(vramcnt[6]) == 5)) {
            memcpy(&return_value, &bank_g[addr & 0x1FFF], sizeof(T));
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
    if (GetVRAMCNTEnabled(vramcnt[8])) {
        if (in_range(0, 0x2000) && (GetVRAMCNTMST(vramcnt[8]) == 3)) {
            memcpy(&return_value, &bank_i[addr & 0x1FFF], sizeof(T));
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
    if (GetVRAMCNTEnabled(vramcnt[2])) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(vramcnt[2]) * 0x20000), 0x20000) && (GetVRAMCNTMST(vramcnt[2]) == 2)) {
            memcpy(&return_value, &bank_c[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[3])) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(vramcnt[3]) * 0x20000), 0x20000) && (GetVRAMCNTMST(vramcnt[3]) == 2)) {
            memcpy(&return_value, &bank_d[addr & 0x1FFFF], sizeof(T));
        }
    }

    return return_value;
}

template void GPU::WriteARM7(u32 addr, u8 data);
template void GPU::WriteARM7(u32 addr, u16 data);
template void GPU::WriteARM7(u32 addr, u32 data);
template <typename T>
void GPU::WriteARM7(u32 addr, T data) {
    if (GetVRAMCNTEnabled(vramcnt[2])) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(vramcnt[2]) * 0x20000), 0x20000) && (GetVRAMCNTMST(vramcnt[2]) == 2)) {
            memcpy(&bank_c[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (GetVRAMCNTEnabled(vramcnt[3])) {
        if (in_range(0x06000000 + (GetVRAMCNTOffset(vramcnt[3]) * 0x20000), 0x20000) && (GetVRAMCNTMST(vramcnt[3]) == 2)) {
            memcpy(&bank_d[addr & 0x1FFFF], &data, sizeof(T));
        }
    }
}

void GPU::RegisterMMIO(MMIO* mmio, MMIOType type) {
    if (type == MMIOType::ARMv5) {
        mmio->Register<u16>(0x04000004,
            [this](u32) {
                return DISPSTAT9 & 0xFFFF;
            },
            [this](u32, u16 data) {
                DISPSTAT9 = (DISPSTAT9 & ~0xFFB8) | (data & 0xFFB8);
            }
        );

        int masks[9] = {0x9B, 0x9B, 0x9F, 0x9F, 0x87, 0x9F, 0x9F, 0x83, 0x83};

        for (int i = 0; i < 7; i++) {
            mmio->Register<u8>(0x04000240 + i,
                [&, this](u32) {
                    return vramcnt[i];
                },
                [&, this](u32, u8 data) {
                    vramcnt[i] = data & masks[i];

                    MapVRAM();
                }
            );
        }

        mmio->Register<u8>(0x04000248,
            [this](u32) {
                return vramcnt[7];
            },
            [this](u32, u8 data) {
                vramcnt[7] = data & 0x83;

                MapVRAM();
            }
        );

        mmio->Register<u8>(0x04000249,
            [this](u32) {
                return vramcnt[8];
            },
            [this](u32, u8 data) {
                vramcnt[8] = data & 0x83;

                MapVRAM();
            }
        );

        mmio->Register<u32>(0x04000240,
            [this](u32) {
                return ((vramcnt[3] << 24) | (vramcnt[2] << 16) | (vramcnt[1] << 8) | (vramcnt[0]));
            },
            [this](u32, u32 data) {
                vramcnt[0] = data & 0x9B;
                vramcnt[1] = (data >> 8) & 0x9B;
                vramcnt[2] = (data >> 16) & 0x9F;
                vramcnt[3] = (data >> 24) & 0x9F;

                MapVRAM();
            }
        );

        mmio->Register<u16>(0x04000304,
            [this](u32) {
                return POWCNT1;
            },
            [this](u32, u16 data) {
                POWCNT1 = data & 0x820F;
            }
        );

        mmio->Register<u32>(0x04000304,
            [this](u32) {
                return POWCNT1;
            },
            [this](u32, u32 data) {
                POWCNT1 = data & 0x820F;
            }
        );
    } else {
        log_fatal("handle");
    }
}
