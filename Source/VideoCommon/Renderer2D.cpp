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

    mmio.register_mmio<u16>(
        0x04000000 + offset,
        mmio.direct_read<u16, u32>(&dispcnt, 0xFFFF),
        mmio.complex_write<u16>([this](u32, u16 data) {
            dispcnt = (dispcnt & ~0xFFFF) | data;
        })
    );

    mmio.register_mmio<u32>(
        0x04000000 + offset,
        mmio.direct_read<u32>(&dispcnt),
        mmio.direct_write<u32>(&dispcnt)
    );

    mmio.register_mmio<u16>(
        0x04000008 + offset,
        mmio.direct_read<u16>(&bgcnt[0]),
        mmio.direct_write<u16>(&bgcnt[0])
    );

    mmio.register_mmio<u32>(
        0x04000008 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgcnt[0] = data & 0xffff;
            bgcnt[1] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x0400000A + offset,
        mmio.direct_read<u16>(&bgcnt[1]),
        mmio.direct_write<u16>(&bgcnt[1])
    );

    mmio.register_mmio<u16>(
        0x0400000C + offset,
        mmio.direct_read<u16>(&bgcnt[2]),
        mmio.direct_write<u16>(&bgcnt[2])
    );

    mmio.register_mmio<u32>(
        0x0400000C + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgcnt[2] = data & 0xffff;
            bgcnt[3] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x0400000E + offset,
        mmio.direct_read<u16>(&bgcnt[3]),
        mmio.direct_write<u16>(&bgcnt[3])
    );

    mmio.register_mmio<u16>(
        0x04000010 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bghofs[0])
    );

    mmio.register_mmio<u32>(
        0x04000010 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bghofs[0] = data & 0xffff;
            bgvofs[0] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x04000012 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgvofs[0])
    );

    mmio.register_mmio<u16>(
        0x04000014 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bghofs[1])
    );

    mmio.register_mmio<u32>(
        0x04000014 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bghofs[1] = data & 0xffff;
            bgvofs[1] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x04000016 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgvofs[1])
    );

    mmio.register_mmio<u16>(
        0x04000018 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bghofs[2])
    );

    mmio.register_mmio<u32>(
        0x04000018 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bghofs[2] = data & 0xffff;
            bgvofs[2] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x0400001A + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgvofs[2])
    );

    mmio.register_mmio<u16>(
        0x0400001C + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bghofs[3])
    );

    mmio.register_mmio<u32>(
        0x0400001C + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bghofs[3] = data & 0xffff;
            bgvofs[3] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x0400001E + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgvofs[3])
    );

    mmio.register_mmio<u16>(
        0x04000020 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgpa[0])
    );

    mmio.register_mmio<u32>(
        0x04000020 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgpa[0] = data & 0xffff;
            bgpb[0] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x04000022 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgpb[0])
    );

    mmio.register_mmio<u16>(
        0x04000024 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgpc[0])
    );
    
    mmio.register_mmio<u32>(
        0x04000024 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgpc[0] = data & 0xffff;
            bgpd[0] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x04000026 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgpd[0])
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

    mmio.register_mmio<u16>(
        0x04000030 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgpa[1])
    );

    mmio.register_mmio<u32>(
        0x04000030 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgpa[1] = data & 0xffff;
            bgpb[1] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x04000032 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgpb[1])
    );

    mmio.register_mmio<u16>(
        0x04000034 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgpc[1])
    );
    
    mmio.register_mmio<u32>(
        0x04000034 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bgpc[1] = data & 0xffff;
            bgpd[1] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x04000036 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bgpd[1])
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

    mmio.register_mmio<u16>(
        0x04000040 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&winh[0])
    );

    mmio.register_mmio<u32>(
        0x04000040 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            winh[0] = data & 0xffff;
            winh[1] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x04000042 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&winh[1])
    );

    mmio.register_mmio<u16>(
        0x04000044 + offset,
        mmio.direct_read<u16>(&winv[0]),
        mmio.direct_write<u16>(&winv[0])
    );

    mmio.register_mmio<u32>(
        0x04000044 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            winv[0] = data & 0xffff;
            winv[1] = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x04000046 + offset,
        mmio.direct_read<u16>(&winv[1]),
        mmio.direct_write<u16>(&winv[1])
    );

    mmio.register_mmio<u16>(
        0x04000048 + offset,
        mmio.direct_read<u16>(&winin),
        mmio.direct_write<u16>(&winin)
    );

    mmio.register_mmio<u32>(
        0x04000048 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            winin = data & 0xffff;
            winout = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x0400004A + offset,
        mmio.direct_read<u16>(&winout),
        mmio.direct_write<u16>(&winout)
    );

    mmio.register_mmio<u8>(
        0x0400004C + offset,
        mmio.invalid_read<u8>(),
        mmio.complex_write<u8>([this](u32, u8 data) {
            mosaic = (mosaic & ~0xFF) | data;
        })
    );

    mmio.register_mmio<u8>(
        0x0400004D + offset,
        mmio.invalid_read<u8>(),
        mmio.complex_write<u8>([this](u32, u8 data) {
            mosaic = (mosaic & 0xFF) | (data << 8);
        })
    );

    mmio.register_mmio<u16>(
        0x0400004C + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&mosaic)
    );

    mmio.register_mmio<u32>(
        0x0400004C + offset,
        mmio.invalid_read<u32>(),
        mmio.direct_write<u32>(&mosaic)
    );

    mmio.register_mmio<u16>(
        0x04000050 + offset,
        mmio.direct_read<u16>(&bldcnt),
        mmio.direct_write<u16>(&bldcnt)
    );

    mmio.register_mmio<u32>(
        0x04000050 + offset,
        mmio.invalid_read<u32>(),
        mmio.complex_write<u32>([this](u32, u32 data) {
            bldcnt = data & 0xffff;
            bldalpha = data >> 16;
        })
    );

    mmio.register_mmio<u16>(
        0x04000052 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bldalpha)
    );

    mmio.register_mmio<u16>(
        0x04000054 + offset,
        mmio.invalid_read<u16>(),
        mmio.direct_write<u16>(&bldy)
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

    mmio.register_mmio<u32>(
        0x0400006C + offset,
        mmio.invalid_read<u32>(),
        mmio.direct_write<u32, u16>(&master_bright)
    );
}