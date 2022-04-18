#include <core/hw/gpu/gpu.h>
#include <core/core.h>

GPU::GPU(System& system) : system(system), engine_a(this, 1), engine_b(this , 0), render_engine(this), geometry_engine(this) {

}

void GPU::Reset() {
    bank_a.fill(0);
    bank_b.fill(0);
    bank_c.fill(0);
    bank_d.fill(0);
    bank_e.fill(0);
    bank_f.fill(0);
    bank_g.fill(0);
    bank_h.fill(0);
    bank_i.fill(0);

    POWCNT1 = 0;
    VCOUNT = 0;
    vramcnt_a = 0;
    vramcnt_b = 0;
    vramcnt_c = 0;
    vramcnt_d = 0;
    vramcnt_e = 0;
    vramcnt_f = 0;
    vramcnt_g = 0;
    vramcnt_h = 0;
    vramcnt_i = 0;

    VRAMSTAT = 0;

    DISPSTAT7 = 0;
    DISPSTAT9 = 0;

    DISPCAPCNT = 0;

    engine_a.Reset();
    engine_b.Reset();
    geometry_engine.Reset();
    render_engine.Reset();

    scanline_start_event = system.scheduler.RegisterEvent("ScanlineStart", [this]() {
        RenderScanlineStart();
        system.scheduler.AddEvent(524, &scanline_finish_event);
    });

    scanline_finish_event = system.scheduler.RegisterEvent("ScanlineFinish", [this]() {
        RenderScanlineFinish();
        system.scheduler.AddEvent(1606, &scanline_start_event);
    });

    system.scheduler.AddEvent(1606, &scanline_start_event);

    update_vram_mapping();
}

void GPU::RenderScanlineStart() {
    if (VCOUNT < 192) {
        engine_a.RenderScanline(VCOUNT);
        engine_b.RenderScanline(VCOUNT);

        // trigger an arm9 dma transfer on hblank (only for visible scanlines)
        system.dma[1].Trigger(2);
    }

    DISPSTAT7 |= (1 << 1);
    DISPSTAT9 |= (1 << 1);
    
    if (DISPSTAT7 & (1 << 4)) {
        system.cpu_core[0].SendInterrupt(InterruptType::HBlank);
    }

    if (DISPSTAT9 & (1 << 4)) {
        system.cpu_core[1].SendInterrupt(InterruptType::HBlank);
    }

    if (VCOUNT == 215) {
        render_engine.Render();
    }

    // ARM9 DMA exclusive
    // check if scanline is between 2 and 193 inclusive
    // if so trigger a start of display dma transfer
    // TODO: on scanline 194 automatically clear the enable bit in DMA
    if ((VCOUNT > 1) && (VCOUNT < 194)) {
        system.dma[1].Trigger(3);
    }
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
            system.cpu_core[0].SendInterrupt(InterruptType::VBlank);
        }

        if (DISPSTAT9 & (1 << 3)) {
            system.cpu_core[1].SendInterrupt(InterruptType::VBlank);
        }

        system.dma[0].Trigger(1);
        system.dma[1].Trigger(1);
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
            system.cpu_core[0].SendInterrupt(InterruptType::VCounter);
        }

    } else if (DISPSTAT7 & (1 << 2)) {
        DISPSTAT7 &= ~(1 << 2);
    }

    if (((DISPSTAT9 >> 8) | ((DISPSTAT9 & (1 << 7)) << 1)) == VCOUNT) {
        DISPSTAT9 |= (1 << 2);

        if (DISPSTAT9 & (1 << 5)) {
            system.cpu_core[1].SendInterrupt(InterruptType::VCounter);
        }

    } else if (DISPSTAT9 & (1 << 2)) {
        DISPSTAT9 &= ~(1 << 2);
    }
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

int GPU::get_bank_mst(u8 vramcnt) {
    return vramcnt & 0x7;
}

int GPU::get_bank_offset(u8 vramcnt) {
    return (vramcnt >> 3) & 0x3;
}

bool GPU::get_bank_enabled(u8 vramcnt) {
    return (vramcnt & (1 << 7));
}

void GPU::update_vram_mapping() {
    // reset all the vram pages
    reset_vram_mapping();

    // we will map vram blocks in increments of 4kb
    if (get_bank_enabled(vramcnt_a)) {
        u8 ofs = get_bank_offset(vramcnt_a);
        switch (get_bank_mst(vramcnt_a)) {
        case 0:
            lcdc.map(bank_a.data(), 0, 32);
            break;
        case 1:
            bga.map(bank_a.data(), ofs * 32, 32);
            break;
        case 2:
            obja.map(bank_a.data(), (ofs & 0x1) * 32, 32);
            break;
        case 3:
            texture_data.map(bank_a.data(), ofs * 32, 32);
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt_a));
        }
    }

    if (get_bank_enabled(vramcnt_b)) {
        u8 ofs = get_bank_offset(vramcnt_b);
        switch (get_bank_mst(vramcnt_b)) {
        case 0:
            lcdc.map(bank_b.data(), 32, 32);
            break;
        case 1:
            bga.map(bank_b.data(), ofs * 32, 32);
            break;
        case 2:
            obja.map(bank_b.data(), (ofs & 0x1) * 32, 32);
            break;
        case 3:
            texture_data.map(bank_b.data(), ofs * 32, 32);
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt_b));
        }
    }
    
    if (get_bank_enabled(vramcnt_c)) {
        u8 ofs = get_bank_offset(vramcnt_c);
        switch (get_bank_mst(vramcnt_c)) {
        case 0:
            lcdc.map(bank_c.data(), 0x40, 32);
            break;
        case 1:
            bga.map(bank_c.data(), ofs * 32, 32);
            break;
        case 2:
            arm7_vram.map(bank_c.data(), (ofs & 0x1) * 32, 32);
            break;
        case 3:
            texture_data.map(bank_c.data(), ofs * 32, 32);
            break;
        case 4:
            bgb.map(bank_c.data(), 0, 32);
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt_c));
        }
    }
    
    if (get_bank_enabled(vramcnt_d)) {
        u8 ofs = get_bank_offset(vramcnt_d);
        switch (get_bank_mst(vramcnt_d)) {
        case 0:
            lcdc.map(bank_d.data(), 0x60, 32);
            break;
        case 1:
            bga.map(bank_d.data(), ofs * 32, 32);
            break;
        case 2:
            arm7_vram.map(bank_d.data(), (ofs & 0x1) * 32, 32);
            break;
        case 3:
            texture_data.map(bank_d.data(), ofs * 32, 32);
            break;
        case 4:
            objb.map(bank_d.data(), 0, 32);
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt_d));
        }
    }
    
    if (get_bank_enabled(vramcnt_e)) {
        u8 ofs = get_bank_offset(vramcnt_e);
        switch (get_bank_mst(vramcnt_e)) {
        case 0:
            lcdc.map(bank_e.data(), 0x80, 16);
            break;
        case 1:
            bga.map(bank_e.data(), 0, 16);
            break;
        case 2:
            obja.map(bank_e.data(), 0, 16);
            break;
        case 3:
            texture_palette.map(bank_e.data(), 0, 16);
            break;
        case 4:
            // handle extpal later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt_e));
        }
    }

    if (get_bank_enabled(vramcnt_f)) {
        u8 ofs = get_bank_offset(vramcnt_f);
        switch (get_bank_mst(vramcnt_f)) {
        case 0:
            lcdc.map(bank_f.data(), 0x90, 4);
            break;
        case 1:
            bga.map(bank_f.data(), (ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10, 4);
            break;
        case 2:
            obja.map(bank_f.data(), (ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10, 4);
            break;
        case 3:
            texture_palette.map(bank_f.data(), ((ofs & 0x1) + ((ofs >> 1) & 0x1) * 0x4) * 16, 4);
            break;
        case 4:
            // handle ext palette later
            break;
        case 5:
            // obj ext palette handle later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt_f));
        }
    }

    if (get_bank_enabled(vramcnt_g)) {
        u8 ofs = get_bank_offset(vramcnt_g);
        switch (get_bank_mst(vramcnt_g)) {
        case 0:
            lcdc.map(bank_g.data(), 0x94, 4);
            break;
        case 1:
            bga.map(bank_g.data(), (ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10, 4);
            break;
        case 2:
            obja.map(bank_g.data(), (ofs & 0x1) * 0x4 + ((ofs >> 1) & 0x1) * 0x10, 4);
            break;
        case 3:
            texture_palette.map(bank_g.data(), ((ofs & 0x1) + ((ofs >> 1) & 0x1) * 0x4) * 16, 4);
            break;
        case 4:
            // handle extended palette later
            break;
        case 5:
            // handle extended palette later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt_g));
        }
    }

    if (get_bank_enabled(vramcnt_h)) {
        u8 ofs = get_bank_offset(vramcnt_h);
        switch (get_bank_mst(vramcnt_h)) {
        case 0:
            lcdc.map(bank_h.data(), 0x98, 8);
            break;
        case 1:
            bgb.map(bank_h.data(), 0, 8);
            break;
        case 2:
            // handle extpal later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt_h));
        }
    }

    if (get_bank_enabled(vramcnt_i)) {
        u8 ofs = get_bank_offset(vramcnt_i);
        switch (get_bank_mst(vramcnt_i)) {
        case 0:
            lcdc.map(bank_i.data(), 0xA0, 4);
            break;
        case 1:
            bgb.map(bank_i.data(), 0x8, 4);
            break;
        case 2:
            objb.map(bank_i.data(), 0, 4);
            break;
        case 3:
            // obj extended palette
            // handle later
            break;
        default:
            log_fatal("handle mst %d", get_bank_mst(vramcnt_i));
        }
    }
}

void GPU::reset_vram_mapping() {
    lcdc.reset();
    bga.reset();
    obja.reset();
    bgb.reset();
    objb.reset();
    arm7_vram.reset();
    texture_data.reset();
}

template u8 GPU::read_vram(u32 addr);
template u16 GPU::read_vram(u32 addr);
template u32 GPU::read_vram(u32 addr);
template <typename T>
T GPU::read_vram(u32 addr) {
    u8 region = (addr >> 20) & 0xF;

    switch (region) {
    case 0x0: case 0x1:
        return bga.read<T>(addr);
    case 0x2: case 0x3:
        return bgb.read<T>(addr);
    case 0x4: case 0x5:
        return obja.read<T>(addr);
    case 0x6: case 0x7:
        return objb.read<T>(addr);
    default:
        return lcdc.read<T>(addr);
    }
}

template void GPU::write_vram(u32 addr, u8 data);
template void GPU::write_vram(u32 addr, u16 data);
template void GPU::write_vram(u32 addr, u32 data);
template <typename T>
void GPU::write_vram(u32 addr, T data) {
    u8 region = (addr >> 20) & 0xF;

    switch (region) {
    case 0x0: case 0x1:
        bga.write<T>(addr, data);
        break;
    case 0x2: case 0x3:
        bgb.write<T>(addr, data);
        break;
    case 0x4: case 0x5:
        obja.write<T>(addr, data);
        break;
    case 0x6: case 0x7:
        objb.write<T>(addr, data);
        break;
    default:
        lcdc.write<T>(addr, data);
        break;
    }
}

template auto GPU::ReadExtPaletteBGA(u32 addr) -> u8;
template auto GPU::ReadExtPaletteBGA(u32 addr) -> u16;
template auto GPU::ReadExtPaletteBGA(u32 addr) -> u32;
template <typename T>
auto GPU::ReadExtPaletteBGA(u32 addr) -> T {
    T return_value = 0;

    if (get_bank_enabled(vramcnt_e)) {
        // only lower 32kb are used
        // vram bank e can then hold all 4 8kb slots
        if (in_range(0, 0x8000) && (get_bank_mst(vramcnt_e) == 4)) {
            memcpy(&return_value, &bank_e[addr & 0xFFFF], sizeof(T));
        }
    }

    if (get_bank_enabled(vramcnt_f)) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        u32 offset = get_bank_offset(vramcnt_f) & 0x1 ? 0x4000 : 0;
        if (in_range(offset, 0x4000) && (get_bank_mst(vramcnt_f) == 4)) {
            memcpy(&return_value, &bank_f[addr & 0x3FFF], sizeof(T));
        }
    }

    if (get_bank_enabled(vramcnt_g)) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        u32 offset = get_bank_offset(vramcnt_g) & 0x1 ? 0x4000 : 0;
        if (in_range(offset, 0x4000) && (get_bank_mst(vramcnt_g) == 4)) {
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

    if (get_bank_enabled(vramcnt_h)) {
        // vram bank h can cover all slots 0-3
        if (in_range(0, 0x8000) && (get_bank_mst(vramcnt_h) == 2)) {
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
    if (get_bank_enabled(vramcnt_f)) {
        if (in_range(0, 0x2000) && (get_bank_mst(vramcnt_f) == 5)) {
            memcpy(&return_value, &bank_f[addr & 0x1FFF], sizeof(T));
        }
    }

    if (get_bank_enabled(vramcnt_g)) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        if (in_range(0, 0x2000) && (get_bank_mst(vramcnt_g) == 5)) {
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
    if (get_bank_enabled(vramcnt_i)) {
        if (in_range(0, 0x2000) && (get_bank_mst(vramcnt_i) == 3)) {
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
    if (get_bank_enabled(vramcnt_c)) {
        if (in_range(0x06000000 + (get_bank_offset(vramcnt_c) * 0x20000), 0x20000) && (get_bank_mst(vramcnt_c) == 2)) {
            memcpy(&return_value, &bank_c[addr & 0x1FFFF], sizeof(T));
        }
    }

    if (get_bank_enabled(vramcnt_d)) {
        if (in_range(0x06000000 + (get_bank_offset(vramcnt_d) * 0x20000), 0x20000) && (get_bank_mst(vramcnt_d) == 2)) {
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
    if (get_bank_enabled(vramcnt_c)) {
        if (in_range(0x06000000 + (get_bank_offset(vramcnt_c) * 0x20000), 0x20000) && (get_bank_mst(vramcnt_c) == 2)) {
            memcpy(&bank_c[addr & 0x1FFFF], &data, sizeof(T));
        }
    }

    if (get_bank_enabled(vramcnt_d)) {
        if (in_range(0x06000000 + (get_bank_offset(vramcnt_d) * 0x20000), 0x20000) && (get_bank_mst(vramcnt_d) == 2)) {
            memcpy(&bank_d[addr & 0x1FFFF], &data, sizeof(T));
        }
    }
}
