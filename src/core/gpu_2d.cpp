#include <core/gpu.h>
#include <core/gpu_2d.h>

GPU2D::GPU2D(GPU* gpu, int engine_id) : gpu(gpu), engine_id(engine_id) {

}

void GPU2D::Reset() {
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));
    memset(palette_ram, 0, 0x400 * sizeof(u16));
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));
    memset(BGCNT, 0, 4 * sizeof(u16));
    memset(BGHOFS, 0, 4 * sizeof(u16));
    memset(BGVOFS, 0, 4 * sizeof(u16));
    memset(BG2P, 0, 4 * sizeof(u16));
    memset(BG3P, 0, 4 * sizeof(u16));
    memset(WINH, 0, 2 * sizeof(u16));
    memset(WINV, 0, 2 * sizeof(u16));

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
}

void GPU2D::WritePaletteRAM(u32 addr, u16 data) {
    memcpy(&palette_ram[addr & 0x3FF], &data, 2);
}

u16 GPU2D::ReadPaletteRAM(u32 addr) {
    return ((palette_ram[(addr & 0x3FF) + 1] << 8) | (palette_ram[addr & 0x3FF]));
}

void GPU2D::WriteOAM(u32 addr, u16 data) {
    memcpy(&oam[addr & 0x3FF], &data, 2);
}


const u32* GPU2D::GetFramebuffer() {
    return &framebuffer[0];
}

// converts a 15 bit rgb colour to a 24 bit rgb colour
u32 GPU2D::Convert15To24(u32 colour) {
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
        // display off (screen becomes white)
        RenderBlankScreen(line);
        break;
    case 1:
        RenderGraphicsDisplay(line);
        break;
    case 2:
        // render using vram display mode
        RenderVRAMDisplay(line);
        break;
    default:
        log_fatal("2d display mode %d is not implemented yet!", display_mode);
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
        // in bg mode 0 all bg layers are rendering text except for bg0 which can render 3d
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
    case 3:
        // in bg mode 0 all bg layers are rendering text except for bg0 which can render 3d and also bg3 which renders only extended
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
            RenderExtended(3, line);
        }
        break;
    default:
        log_fatal("bg mode %d is not implemented yet!", bg_mode);
    }
    // log_warn("dispcnt is %08x for reference lol", DISPCNT);
}

void GPU2D::RenderText(int bg_index, u16 line) {
    u32 character_base = (((BGCNT[bg_index] >> 2) & 0x3) * 0x4000) + (((DISPCNT >> 24) & 0x7) * 0x10000);
    u32 screen_base = (((BGCNT[bg_index] >> 8) & 0x1F) * 0x800) + (((DISPCNT >> 27) & 0x7) * 0x10000);
    screen_base += ((line / 8) % 32) * 64;
    u8 screen_size = (BGCNT[bg_index] >> 14) & 0x3;

    if (BGCNT[bg_index] & (1 << 7)) {
        // 256 colours / 1 palette
        // 1 tile occupies 64 bytes with 1 row occupying 8 bytes
        for (int i = 0; i < 256; i += 8) {
            // iterate through each row of tiles
            u32 screen_addr = 0x06000000 + screen_base + ((i / 8) * 2);
            // iterate through each pixel in that row
            u32 tile_info;
            if (engine_id == 1) {
                tile_info = gpu->ReadBGA(screen_addr);
            } else {
                tile_info = gpu->ReadBGB(screen_addr);
            }

            // now we need to decode what the tile info means
            // Bit   Expl.
            // 0-9   Tile Number     (0-1023) (a bit less in 256 color mode, because
            //                  there'd be otherwise no room for the bg map)
            // 10    Horizontal Flip (0=Normal, 1=Mirrored)
            // 11    Vertical Flip   (0=Normal, 1=Mirrored)
            //12-15 Palette Number  (0-15)    (Not used in 256 color/1 palette mode)
            u32 tile_number = tile_info & 0x3FF;
            u8 horizontal_flip = (tile_info >> 10) & 0x1;
            u8 vertical_flip = (tile_info >> 11) & 0x1;
            // times by 64 as each tile is 64 bytes long
            u32 character_addr = 0x06000000 + character_base + (tile_number * 64);

            // for 256 colour / 1 palette mode each byte represents an index to a 16 colour in palette_ram
            for (int j = 0; j < 8; j += 2) {
                u16 data;
                // this now gives us the index in palette_ram, and we will then use the first byte for the first pixel and 2nd byte for 2nd pixel
                if (engine_id == 1) {
                    data = gpu->ReadBGA(character_addr + ((line % 8) * 8) + j);
                } else {
                    data = gpu->ReadBGB(character_addr + ((line % 8) * 8) + j);
                }
                
                
                
                // now get the actual colour from the palette for 2 pixels
                u16 colour1 = ReadPaletteRAM(data && 0xFF);
                u16 colour2 = ReadPaletteRAM(data >> 8);
            
                // write to the framebuffer
                framebuffer[(256 * line) + i + j] = Convert15To24(colour1);
                framebuffer[(256 * line) + i + j + 1] = Convert15To24(colour2);
            }
        }
    }
    
    // log_debug("colours/palettes: %d", (BGCNT[bg_index] >> 7) & 0x1);
    // log_debug("screen size: %d", screen_size);
    // log_debug("char base %08x screen base %08x", character_base, screen_base);
    // log_debug("baaa");
}

void GPU2D::RenderExtended(int bg_index, u16 line) {
    u32 screen_base = ((BGCNT[bg_index] >> 8) & 0x1F) * 0x800;
    // log_fatal("screen base %08x", screen_base * 0x4000);
    for (int i = 0; i < 256; i++) {
        // just print out colour values
        u16 data;
        u32 addr = 0x06000000 + screen_base + (((256 * line) + i) * 2);
        // TODO: add proper bg layers later
        if (engine_id == 1) {
            data = gpu->ReadBGA(addr);
        } else {
            data = gpu->ReadBGB(addr);
        }
        framebuffer[(256 * line) + i] = Convert15To24(data);
        
    }
    // log_fatal("handle bit2 %d bit7 %d", bit2, bit7);
}