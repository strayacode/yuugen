#include <core/hw/gpu/gpu.h>
#include <core/hw/gpu/gpu_2d.h>

GPU2D::GPU2D(GPU* gpu, int engine_id) : gpu(gpu), engine_id(engine_id) {

}

void GPU2D::Reset() {
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));
    memset(palette_ram, 0, 0x400);
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));
    memset(BGCNT, 0, 4 * sizeof(u16));
    memset(BGHOFS, 0, 4 * sizeof(u16));
    memset(BGVOFS, 0, 4 * sizeof(u16));
    memset(BG2P, 0, 4 * sizeof(u16));
    memset(BG3P, 0, 4 * sizeof(u16));
    memset(WINH, 0, 2 * sizeof(u16));
    memset(WINV, 0, 2 * sizeof(u16));

    for (int i = 0; i < 4; i++) {
        memset(layers[i], 0, 256 * 192 * sizeof(u32));
    }

    DISPCNT = 0;
    BG2X = 0;
    BG2Y = 0;
    BG3X = 0;
    BG3Y = 0;
    WININ = 0;
    WINOUT = 0;
    MOSAIC = 0;
    BLDCNT = 0;
    BLDALPHA = 0;
    BLDY = 0;
    MASTER_BRIGHT = 0;

    if (engine_id == 1) {
        vram_addr = 0x06000000;
    } else {
        vram_addr = 0x06200000;
    }
}

void GPU2D::WritePaletteRAM(u32 addr, u16 data) {
    memcpy(&palette_ram[addr & 0x3FF], &data, 2);
}

auto GPU2D::ReadPaletteRAM(u32 addr) -> u16 {
    return ((palette_ram[(addr & 0x3FF) + 1] << 8) | (palette_ram[addr & 0x3FF]));
}

void GPU2D::WriteOAM(u32 addr, u16 data) {
    memcpy(&oam[addr & 0x3FF], &data, 2);
}

auto GPU2D::ReadOAM(u32 addr) -> u16 {
    return ((oam[(addr & 0x3FF) + 1] << 8) | (oam[addr & 0x3FF]));
}

auto GPU2D::GetFramebuffer() -> const u32* {
    return &framebuffer[0];
}

// converts a 15 bit rgb colour to a 24 bit rgb colour
auto GPU2D::Convert15To24(u32 colour) -> u32 {
    u8 b = ((colour & 0x1F) * 255) / 31;
    u8 g = (((colour >> 5) & 0x1F) * 255) / 31;
    u8 r = (((colour >> 10) & 0x1F) * 255) / 31;
    return (b << 16) | (g << 8) | r;
}

void GPU2D::RenderScanline(u16 line) {
    // get the display mode (bits 16..17)
    u8 display_mode = (DISPCNT >> 16) & 0x3;
    switch (display_mode) {
    case 0:
        RenderBlankScreen(line);
        break;
    case 1:
        RenderGraphicsDisplay(line);
        break;
    case 2:
        RenderVRAMDisplay(line);
        break;
    default:
        log_fatal("[GPU2D] 2D display mode %d is not implemented yet!", display_mode);
    }
}

void GPU2D::RenderBlankScreen(u16 line) {
    memset(&framebuffer[256 * line], 0xFF, 256 * sizeof(u32));
}

void GPU2D::RenderVRAMDisplay(u16 line) {
    for (int i = 0; i < 256; i++) {
        u16 data;
        u32 addr = ((256 * line) + i) * 2;
        
        switch ((DISPCNT >> 18) & 0x3) {
        case 0:
            data = (gpu->VRAM_A[addr + 1] << 8 | gpu->VRAM_A[addr]);
            break;
        case 1:
            data = (gpu->VRAM_B[addr + 1] << 8 | gpu->VRAM_B[addr]);
            break;
        case 2:
            data = (gpu->VRAM_C[addr + 1] << 8 | gpu->VRAM_C[addr]);
            break;
        case 3:
            data = (gpu->VRAM_D[addr + 1] << 8 | gpu->VRAM_D[addr]);
            break;
        }
        framebuffer[(256 * line) + i] = Convert15To24(data);
    }
}

void GPU2D::RenderGraphicsDisplay(u16 line) {
    u8 bg_mode = DISPCNT & 0x7;
    switch (bg_mode) {
    case 0:
        if (DISPCNT & (1 << 8)) {
            RenderText(0, line);
        }
        if (DISPCNT & (1 << 9)) {
            RenderText(1, line);
        }
        if (DISPCNT & (1 << 10)) {
            RenderText(2, line);
        }
        if (DISPCNT & (1 << 11)) {
            RenderText(3, line);
        }
        break;
    default:
        log_fatal("[GPU2D] BG mode %d is unimplemented", bg_mode);
    }

    ComposeScanline(line);
}