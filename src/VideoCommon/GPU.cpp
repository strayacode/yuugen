#include "Common/Log.h"
#include "Common/Memory.h"
#include "VideoCommon/GPU.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"
#include "VideoBackends3D/Software/SoftwareRenderer3D.h"
#include "Core/system.h"

GPU::GPU(System& system) : system(system) {}

void GPU::reset() {
    powcnt1 = 0;
    vcount = 0;
    dispstat.fill(0);
    vramcnt.fill(0);
    dispcapcnt = 0;
    
    bank_a.fill(0);
    bank_b.fill(0);
    bank_c.fill(0);
    bank_d.fill(0);
    bank_e.fill(0);
    bank_f.fill(0);
    bank_g.fill(0);
    bank_h.fill(0);
    bank_i.fill(0);
    palette_ram.fill(0);
    oam.fill(0);

    renderer_2d[0]->reset();
    renderer_2d[1]->reset();

    scanline_start_event = system.scheduler.RegisterEvent("Scanline Start", [this]() {
        render_scanline_start();
        system.scheduler.AddEvent(524, &scanline_end_event);
    });

    scanline_end_event = system.scheduler.RegisterEvent("Scanline End", [this]() {
        render_scanline_end();
        system.scheduler.AddEvent(1606, &scanline_start_event);
    });

    system.scheduler.AddEvent(1606, &scanline_start_event);

    reset_vram_mapping();
}

void GPU::create_renderers(RendererType type) {
    switch (type) {
    case RendererType::Software:
        renderer_2d[0] = std::make_unique<SoftwareRenderer2D>(*this, Engine::A);
        renderer_2d[1] = std::make_unique<SoftwareRenderer2D>(*this, Engine::B);
        renderer_3d = std::make_unique<SoftwareRenderer3D>();
        break;
    default:
        log_fatal("GPU: unknown renderer type %d", static_cast<int>(type));
    }
}

const u32* GPU::get_framebuffer(Screen screen) {
    if (((powcnt1 >> 15) & 0x1) == (screen == Screen::Top)) {
        return renderer_2d[0]->get_framebuffer();
    } else {
        return renderer_2d[1]->get_framebuffer();
    }
}

void GPU::update_vram_mapping(GPU::Bank bank, u8 data) {
    const u8 masks[] = {0x9B, 0x9B, 0x9F, 0x9F, 0x87, 0x9F, 0x9F, 0x83, 0x83};
    int index = static_cast<int>(bank);

    data &= masks[index];

    if (vramcnt[index] == data) return;

    vramcnt[index] = data;

    // reset all the vram pages
    reset_vram_mapping();

    // remap bank a
    if (get_bank_enabled(vramcnt[0])) {
        u8 ofs = get_bank_offset(vramcnt[0]);
        switch (get_bank_mst(vramcnt[0])) {
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
            log_fatal("handle mst %d", get_bank_mst(vramcnt[0]));
        }
    }

    // remap bank b
    if (get_bank_enabled(vramcnt[1])) {
        u8 ofs = get_bank_offset(vramcnt[1]);
        switch (get_bank_mst(vramcnt[1])) {
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
            log_fatal("handle mst %d", get_bank_mst(vramcnt[1]));
        }
    }
    
    // remap bank c
    if (get_bank_enabled(vramcnt[2])) {
        u8 ofs = get_bank_offset(vramcnt[2]);
        switch (get_bank_mst(vramcnt[2])) {
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
            log_fatal("handle mst %d", get_bank_mst(vramcnt[2]));
        }
    }
    
    // remap bank d
    if (get_bank_enabled(vramcnt[3])) {
        u8 ofs = get_bank_offset(vramcnt[3]);
        switch (get_bank_mst(vramcnt[3])) {
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
            log_fatal("handle mst %d", get_bank_mst(vramcnt[3]));
        }
    }
    
    // remap bank e
    if (get_bank_enabled(vramcnt[4])) {
        switch (get_bank_mst(vramcnt[4])) {
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
            log_fatal("handle mst %d", get_bank_mst(vramcnt[4]));
        }
    }

    // remap bank f
    if (get_bank_enabled(vramcnt[5])) {
        u8 ofs = get_bank_offset(vramcnt[5]);
        switch (get_bank_mst(vramcnt[5])) {
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
            log_fatal("handle mst %d", get_bank_mst(vramcnt[5]));
        }
    }

    // remap bank g
    if (get_bank_enabled(vramcnt[6])) {
        u8 ofs = get_bank_offset(vramcnt[6]);
        switch (get_bank_mst(vramcnt[6])) {
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
            log_fatal("handle mst %d", get_bank_mst(vramcnt[6]));
        }
    }

    // remap bank h
    if (get_bank_enabled(vramcnt[7])) {
        switch (get_bank_mst(vramcnt[7])) {
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
            log_fatal("handle mst %d", get_bank_mst(vramcnt[7]));
        }
    }

    // remap bank i
    if (get_bank_enabled(vramcnt[8])) {
        switch (get_bank_mst(vramcnt[8])) {
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
            log_fatal("handle mst %d", get_bank_mst(vramcnt[8]));
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
    texture_palette.reset();
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

template u8 GPU::read_ext_palette_bga(u32 addr);
template u16 GPU::read_ext_palette_bga(u32 addr);
template u32 GPU::read_ext_palette_bga(u32 addr);
template <typename T>
T GPU::read_ext_palette_bga(u32 addr) {
    if (get_bank_enabled(vramcnt[4])) {
        // only lower 32kb are used
        // vram bank e can then hold all 4 8kb slots
        if (Common::in_range(0, 0x8000, addr) && (get_bank_mst(vramcnt[4]) == 4)) {
            return Common::read<T>(&bank_e[addr & 0xFFFF], 0);
        }
    }

    if (get_bank_enabled(vramcnt[5])) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        u32 offset = get_bank_offset(vramcnt[5]) & 0x1 ? 0x4000 : 0;
        if (Common::in_range(offset, 0x4000, addr) && (get_bank_mst(vramcnt[5]) == 4)) {
            return Common::read<T>(&bank_f[addr & 0x3FFF], 0);
        }
    }

    if (get_bank_enabled(vramcnt[6])) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        u32 offset = get_bank_offset(vramcnt[6]) & 0x1 ? 0x4000 : 0;
        if (Common::in_range(offset, 0x4000, addr) && (get_bank_mst(vramcnt[6]) == 4)) {
            return Common::read<T>(&bank_g[addr & 0x3FFF], 0);
        }
    }

    return 0;
}

template u8 GPU::read_ext_palette_bgb(u32 addr);
template u16 GPU::read_ext_palette_bgb(u32 addr);
template u32 GPU::read_ext_palette_bgb(u32 addr);
template <typename T>
T GPU::read_ext_palette_bgb(u32 addr) {
    if (get_bank_enabled(vramcnt[7])) {
        // vram bank h can cover all slots 0-3
        if (Common::in_range(0, 0x8000, addr) && (get_bank_mst(vramcnt[7]) == 2)) {
            return Common::read<T>(&bank_h[addr & 0x7FFF], 0);
        }
    }

    return 0;
}

template u8 GPU::read_ext_palette_obja(u32 addr);
template u16 GPU::read_ext_palette_obja(u32 addr);
template u32 GPU::read_ext_palette_obja(u32 addr);
template <typename T>
T GPU::read_ext_palette_obja(u32 addr) {
    // only the lower 8kb of a vram bank is used, since for objs only one 8kb slot is used
    if (get_bank_enabled(vramcnt[5])) {
        if (Common::in_range(0, 0x2000, addr) && (get_bank_mst(vramcnt[5]) == 5)) {
            return Common::read<T>(&bank_f[addr & 0x1FFF], 0);
        }
    }

    if (get_bank_enabled(vramcnt[6])) {
        // we will either access slots 0-1 or 2-3 depending on ofs
        if (Common::in_range(0, 0x2000, addr) && (get_bank_mst(vramcnt[6]) == 5)) {
            return Common::read<T>(&bank_g[addr & 0x1FFF], 0);
        }
    }

    return 0;
}

template u8 GPU::read_ext_palette_objb(u32 addr);
template u16 GPU::read_ext_palette_objb(u32 addr);
template u32 GPU::read_ext_palette_objb(u32 addr);
template <typename T>
T GPU::read_ext_palette_objb(u32 addr) {
    // only the lower 8kb of a vram bank is used, since for objs only one 8kb slot is used
    if (get_bank_enabled(vramcnt[8])) {
        if (Common::in_range(0, 0x2000, addr) && (get_bank_mst(vramcnt[8]) == 3)) {
            return Common::read<T>(&bank_i[addr & 0x1FFF], 0);
        }
    }

    return 0;
}

void GPU::render_scanline_start() {
    if (vcount < 192) {
        renderer_2d[0]->render_scanline(vcount);
        renderer_2d[1]->render_scanline(vcount);

        // trigger an arm9 dma transfer on hblank (only for visible scanlines)
        system.dma[1].Trigger(2);
    }

    for (int i = 0; i < 2; i++) {
        dispstat[i] |= 1 << 1;

        if (dispstat[i] & (1 << 4)) {
            system.cpu_core[i].SendInterrupt(InterruptType::HBlank);
        }
    }

    // TODO: handle 3d rendering
    // if (vcount == 215) {
    //     render_engine.Render();
    // }

    // ARM9 DMA exclusive
    // check if scanline is between 2 and 193 inclusive
    // if so trigger a start of display dma transfer
    // TODO: on scanline 194 automatically clear the enable bit in DMA
    if ((vcount > 1) && (vcount < 194)) {
        system.dma[1].Trigger(3);
    }
}

void GPU::render_scanline_end() {
    vcount++;

    for (int i = 0; i < 2; i++) {
        dispstat[i] &= ~(1 << 1);

        switch (vcount) {
        case 192:
            // start of vblank
            dispstat[i] |= 1;

            if (dispstat[i] & (1 << 3)) {
                system.cpu_core[i].SendInterrupt(InterruptType::VBlank);
            }

            system.dma[i].Trigger(1);
            break;
        case 262:
            // end of vblank
            dispstat[i] &= ~1;
            break;
        }

        if (((dispstat[i] >> 8) | ((dispstat[i] & (1 << 7)) << 1)) == vcount) {
            dispstat[i] |= (1 << 2);

            if (dispstat[i] & (1 << 5)) {
                system.cpu_core[i].SendInterrupt(InterruptType::VCounter);
            }

        } else if (dispstat[i] & (1 << 2)) {
            dispstat[i] &= ~(1 << 2);
        }
    }

    // end of frame
    if (vcount == 263) {
        vcount = 0;
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