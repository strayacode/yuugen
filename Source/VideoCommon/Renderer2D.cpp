#include "Common/Log.h"
#include "VideoCommon/Renderer2D.h"
#include "VideoCommon/VideoUnit.h"

Renderer2D::Renderer2D(VideoUnit& video_unit, Engine engine) : video_unit(video_unit), engine(engine) {
    if (engine == Engine::A) {
        palette_ram = video_unit.get_palette_ram();
        oam = video_unit.get_oam();
        vram_addr = 0x06000000;
        obj_addr = 0x06400000;
    } else {
        palette_ram = video_unit.get_palette_ram() + 0x400;
        oam = video_unit.get_oam() + 0x400;
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

void Renderer2D::build_mmio(MMIO& mmio) {
    u32 offset = engine == Engine::A ? 0 : 0x1000;

    mmio.register_mmio<u32>(
        0x04000000 + offset,
        mmio.direct_read<u32>(&dispcnt),
        mmio.direct_write<u32>(&dispcnt)
    );

    mmio.register_mmio<u32>(
        0x04000008 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgcnt[0] = data & 0xffff;
            bgcnt[1] = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x0400000C + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgcnt[2] = data & 0xffff;
            bgcnt[3] = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x04000010 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bghofs[0] = data & 0xffff;
            bgvofs[0] = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x04000014 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bghofs[1] = data & 0xffff;
            bgvofs[1] = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x04000018 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bghofs[2] = data & 0xffff;
            bgvofs[2] = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x0400001C + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bghofs[3] = data & 0xffff;
            bgvofs[3] = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x04000020 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgpa[0] = data & 0xffff;
            bgpb[0] = data >> 16;
        })
    );
    
    mmio.register_mmio<u32>(
        0x04000024 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgpc[0] = data & 0xffff;
            bgpd[0] = data >> 16;
        })
    );
    
    mmio.register_mmio<u32>(
        0x04000028 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgx[0] = data;
            internal_x[0] = data;
        })
    );

    mmio.register_mmio<u32>(
        0x0400002C + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgy[0] = data;
            internal_y[0] = data;
        })
    );

    mmio.register_mmio<u32>(
        0x04000030 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgpa[1] = data & 0xffff;
            bgpb[1] = data >> 16;
        })
    );
    
    mmio.register_mmio<u32>(
        0x04000034 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgpc[1] = data & 0xffff;
            bgpd[1] = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x04000038 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgx[1] = data;
            internal_x[1] = data;
        })
    );

    mmio.register_mmio<u32>(
        0x0400003C + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgy[1] = data;
            internal_y[1] = data;
        })
    );

    mmio.register_mmio<u32>(
        0x04000040 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            winh[0] = data & 0xffff;
            winh[1] = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x04000044 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            winv[0] = data & 0xffff;
            winv[1] = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x04000048 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            winin = data & 0xffff;
            winout = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x0400004C + offset,
        mmio.invalid_read<u32>(),
        mmio.direct_write<u32>(&mosaic)
    );

    mmio.register_mmio<u32>(
        0x04000050 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bldcnt = data & 0xffff;
            bldalpha = data >> 16;
        })
    );

    mmio.register_mmio<u32>(
        0x04000054 + offset,
        mmio.invalid_read<u32>(),
        mmio.direct_write<u32>(&bldy)
    );

    mmio.register_mmio<u32>(
        0x04000058 + offset,
        mmio.stub_read<u32>(),
        mmio.stub_write<u32>()
    );

    mmio.register_mmio<u32>(
        0x0400005C + offset,
        mmio.stub_read<u32>(),
        mmio.stub_write<u32>()
    );

    mmio.register_mmio<u16>(
        0x0400006C + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&master_bright)
    );
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
    addr &= 0xFFF;

    switch (addr) {
    case 0x4C:
        mosaic = (mosaic & ~0xFF) | data;
        break;
    case 0x4D:
        mosaic = (mosaic & 0xFF) | (data << 8);
        break;
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