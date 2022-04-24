#include "Common/Log.h"
#include "VideoCommon/Renderer2D.h"
#include "VideoCommon/GPU.h"

Renderer2D::Renderer2D(GPU& gpu, Engine engine) : gpu(gpu), engine(engine) {
    if (engine == Engine::A) {
        palette_ram = gpu.get_palette_ram();
        oam = gpu.get_oam();
        vram_addr = 0x06000000;
        obj_addr = 0x06400000;
    } else {
        palette_ram = gpu.get_palette_ram() + 0x400;
        oam = gpu.get_oam() + 0x400;
        vram_addr = 0x06200000;
        obj_addr = 0x06600000;
    }
}

void Renderer2D::reset() {
    dispcnt = 0;
    bgcnt.fill(0);
    bgcnt.fill(0);
    bghofs.fill(0);
    bgvofs.fill(0);
    bgpa.fill(0);
    bgpb.fill(0);
    bgpc.fill(0);
    bgpd.fill(0);
    bgx.fill(0);
    bgy.fill(0);
    internal_x.fill(0);
    internal_y.fill(0);
    winh.fill(0);
    winv.fill(0);
    winin = 0;
    winout = 0;
    mosaic = 0;
    bldcnt = 0;
    bldalpha = 0;
    bldy = 0;
    master_bright = 0;

    framebuffer.fill(0);
    
    for (int i = 0; i < 4; i++) {
        bg_layers[i].fill(0);
    }

    obj_priority.fill(0);
    obj_colour.fill(0);
}

u8 Renderer2D::read_byte(u32 addr) {
    switch (addr) {
    default:
        log_fatal("Renderer2D: byte read %08x", addr);
    }

    return 0;
}

u16 Renderer2D::read_half(u32 addr) {
    addr &= 0xFFF;

    switch (addr) {
    case 0x00:
        return dispcnt & 0xFFFF;
    case 0x02:
        return dispcnt >> 16;
    case 0x08:
        return bgcnt[0];
    case 0x0A:
        return bgcnt[1];
    case 0x0C:
        return bgcnt[2];
    case 0x0E:
        return bgcnt[3];
    case 0x44:
        return winv[0];
    case 0x46:
        return winv[1];
    case 0x48:
        return winin;
    case 0x4A:
        return winout;
    case 0x50:
        return bldcnt;
    default:
        log_fatal("Renderer2D: half read %08x", addr);
    }

    return 0;
}

u32 Renderer2D::read_word(u32 addr) {
    addr &= 0xFFF;

    switch (addr) {
    case 0x00:
        return dispcnt;
    default:
        log_fatal("Renderer2D: word read %08x", addr);
    }

    return 0;
}

void Renderer2D::write_byte(u32 addr, u8 data) {
    switch (addr) {
    default:
        log_fatal("Renderer2D: byte write %08x = %02x", addr, data);
    }
}

void Renderer2D::write_half(u32 addr, u16 data) {
    addr &= 0xFFF;

    switch (addr) {
    case 0x00:
        dispcnt = (dispcnt & ~0xFFFF) | data;
        break;
    case 0x08:
        bgcnt[0] = data;
        break;
    case 0x0A:
        bgcnt[1] = data;
        break;
    case 0x0C:
        bgcnt[2] = data;
        break;
    case 0x0E:
        bgcnt[3] = data;
        break;
    case 0x10:
        bghofs[0] = data;
        break;
    case 0x12:
        bgvofs[0] = data;
        break;
    case 0x14:
        bghofs[1] = data;
        break;
    case 0x16:
        bgvofs[1] = data;
        break;
    case 0x18:
        bghofs[2] = data;
        break;
    case 0x1A:
        bgvofs[2] = data;
        break;
    case 0x1C:
        bghofs[3] = data;
        break;
    case 0x1E:
        bgvofs[3] = data;
        break;
    case 0x20:
        bgpa[0] = data;
        break;
    case 0x22:
        bgpb[0] = data;
        break;
    case 0x24:
        bgpc[0] = data;
        break;
    case 0x26:
        bgpd[0] = data;
        break;
    case 0x30:
        bgpa[1] = data;
        break;
    case 0x32:
        bgpb[1] = data;
        break;
    case 0x34:
        bgpc[1] = data;
        break;
    case 0x36:
        bgpd[1] = data;
        break;
    case 0x40:
        winh[0] = data;
        break;
    case 0x42:
        winh[1] = data;
        break;
    case 0x44:
        winv[0] = data;
        break;
    case 0x46:
        winv[1] = data;
        break;
    case 0x48:
        winin = data;
        break;
    case 0x4A:
        winout = data;
        break;
    case 0x4C:
        mosaic = data;
        break;
    case 0x50:
        bldcnt = data;
        break;
    case 0x52:
        bldalpha = data;
        break;
    case 0x54:
        bldy = data;
        break;
    default:
        log_fatal("Renderer2D: half write %08x = %04x", addr, data);
    }
}

void Renderer2D::write_word(u32 addr, u32 data) {
    addr &= 0xFFF;

    switch (addr) {
    case 0x00:
        dispcnt = data;
        break;
    case 0x08:
        bgcnt[0] = data & 0xFFFF;
        bgcnt[1] = data >> 16;
        break;
    case 0x0C:
        bgcnt[2] = data & 0xFFFF;
        bgcnt[3] = data >> 16;
        break;
    case 0x10:
        bghofs[0] = data & 0xFFFF;
        bgvofs[0] = data >> 16;
        break;
    case 0x14:
        bghofs[1] = data & 0xFFFF;
        bgvofs[1] = data >> 16;
        break;
    case 0x18:
        bghofs[2] = data & 0xFFFF;
        bgvofs[2] = data >> 16;
        break;
    case 0x1C:
        bghofs[3] = data & 0xFFFF;
        bgvofs[3] = data >> 16;
        break;
    case 0x20:
        bgpa[0] = data & 0xFFFF;
        bgpb[0] = data >> 16;
        break;
    case 0x24:
        bgpc[0] = data & 0xFFFF;
        bgpd[0] = data >> 16;
        break;
    case 0x28:
        bgx[0] = data;
        internal_x[0] = data;
        break;
    case 0x2C:
        bgy[0] = data;
        internal_y[0] = data;
        break;
    case 0x30:
        bgpa[1] = data & 0xFFFF;
        bgpb[1] = data >> 16;
        break;
    case 0x34:
        bgpc[1] = data & 0xFFFF;
        bgpd[1] = data >> 16;
        break;
    case 0x38:
        bgx[1] = data;
        internal_x[1] = data;
        break;
    case 0x3C:
        bgy[1] = data;
        internal_y[1] = data;
        break;
    case 0x40:
        winh[0] = data & 0xFFFF;
        winh[1] = data >> 16;
        break;
    case 0x44:
        winv[0] = data & 0xFFFF;
        winv[1] = data >> 16;
        break;
    case 0x48:
        winin = data & 0xFFFF;
        winout = data >> 16;
        break;
    case 0x4C:
        mosaic = data;
        break;
    case 0x50:
        bldcnt = data & 0xFFFF;
        bldalpha = data >> 16;
        break;
    case 0x54:
        bldy = data;
        break;
    case 0x58: case 0x5C:
        break;
    default:
        log_fatal("Renderer2D: word write %08x = %08x", addr, data);
    }
}