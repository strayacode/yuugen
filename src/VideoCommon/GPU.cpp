#include "Common/Log.h"
#include "VideoCommon/GPU.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"
#include "VideoBackends3D/Software/SoftwareRenderer3D.h"
#include "Core/system.h"

GPU::GPU(System& system) : system(system) {
    // scanline_start_event = system.scheduler.RegisterEvent("Scanline Start", [this]() {
    //     render_scanline_start();
    //     system.scheduler.AddEvent(524, &scanline_end_event);
    // });

    // scanline_end_event = system.scheduler.RegisterEvent("Scanline End", [this]() {
    //     render_scanline_end();
    //     system.scheduler.AddEvent(1606, &scanline_start_event);
    // });

    // system.scheduler.AddEvent(1606, &scanline_start_event);
}

void GPU::reset() {
    powcnt1 = 0;
    vramcnt_a = 0;
    vramcnt_b = 0;
    vramcnt_c = 0;
    vramcnt_d = 0;
    vramcnt_e = 0;
    vramcnt_f = 0;
    vramcnt_g = 0;
    vramcnt_h = 0;
    vramcnt_i = 0;

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
}

u8 GPU::read_byte(u32 addr) {
    return 0;
}

u16 GPU::read_half(u32 addr) {
    return 0;
}

u32 GPU::read_word(u32 addr) {
    return 0;
}


void GPU::write_byte(u32 addr, u8 data) {
    
}

void GPU::write_half(u32 addr, u16 data) {
    
}

void GPU::write_word(u32 addr, u32 data) {
    
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

void GPU::render_scanline_start() {

}

void GPU::render_scanline_end() {

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