#include "core/gba/gba.h"
#include "core/gba/gpu/gpu.h"

GBAGPU::GBAGPU(GBA& gba) : gba(gba) {

}

void GBAGPU::Reset() {
    dispcnt = 0;
    dispstat = 0;
    vcount = 0;

    scanline_start_event = gba.scheduler.RegisterEvent("ScanlineStart", [this]() {
        RenderScanlineStart();
        gba.scheduler.AddEvent(272, &scanline_finish_event);
    });

    scanline_finish_event = gba.scheduler.RegisterEvent("ScanlineFinish", [this]() {
        RenderScanlineFinish();
        gba.scheduler.AddEvent(960, &scanline_start_event);
    });

    gba.scheduler.AddEvent(960, &scanline_start_event);
}


void GBAGPU::RenderScanlineStart() {
    // start of hblank
    if (vcount < 160) {
        RenderScanline(vcount);
    }

    dispstat |= (1 << 1);

    // hblank irq
    if (dispstat & (1 << 4)) {
        gba.cpu_core.SendInterrupt(InterruptType::HBlank);
    }

    // TODO: hblank dmas
}

void GBAGPU::RenderScanlineFinish() {
    // end of hblank
    dispstat &= ~(1 << 1);

    switch (++vcount) {
    case 160:
        // start of vblank
        dispstat |= 1;

        if (dispstat & (1 << 3)) {
            gba.cpu_core.SendInterrupt(InterruptType::VBlank);
        }

        // TODO: vblank dmas
        break;
    case 227:
        // end of vblank
        dispstat &= ~1;
        break;
    case 228:
        // end of frame
        vcount = 0;
        break;
    }

    if ((dispstat >> 8) == vcount) {
        dispstat |= (1 << 2);

        if (dispstat & (1 << 5)) {
            gba.cpu_core.SendInterrupt(InterruptType::VCounter);
        }
    } else if (dispstat & (1 << 2)) {
        dispstat &= ~(1 << 2);
    }
}

void GBAGPU::RenderScanline(u16 line) {
    u8 bg_mode = dispcnt & 0x7;

    switch (bg_mode) {
    // case 3:
    //     RenderMode3(line);
    //     break;
    case 4:
        RenderMode4(line);
        break;
    default:
        log_fatal("[GPU] Handle bg mode %d", bg_mode);
    }

    ComposeScanline(line);
}

void GBAGPU::RenderMode3(u16 line) {
    // mode 3 is pretty simple
    // every 2 bytes corresponds to a 16 bit colour value

    u8 frame_select = dispcnt & (1 << 4);

    for (int i = 0; i < 240; i++) {
        // read the colour from vram
        u16 colour = 0;
        // TODO: add function for this
        memcpy(&colour, &vram[i * 2], 2);

        // TODO: handle palette index 0
        bg_layers[2][(240 * line) + i] = colour;
    }
}

void GBAGPU::RenderMode4(u16 line) {
    // mode 4 is pretty simple
    // each byte is a index in palette ram

    u8 frame_select = dispcnt & (1 << 4);
    for (int i = 0; i < 240; i++) {
        u8 palette_index = vram[(240 * line) + i];

        // now read a colour from palette ram
        u16 colour = 0;
        // TODO: add function for this
        memcpy(&colour, &palette_ram[palette_index * 2], 2);

        // TODO: handle palette index 0
        bg_layers[2][(240 * line) + i] = colour;
    }
}

const u32* GBAGPU::GetFramebuffer(int screen) {
    return &framebuffer[0];
}

u32 GBAGPU::Convert15To24(u32 colour) {
    u8 b = ((colour & 0x1F) * 255) / 31;
    u8 g = (((colour >> 5) & 0x1F) * 255) / 31;
    u8 r = (((colour >> 10) & 0x1F) * 255) / 31;
    return (b << 16) | (g << 8) | r;
}

