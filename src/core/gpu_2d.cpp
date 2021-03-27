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

u16 GPU2D::ReadOAM(u32 addr) {
    return ((oam[(addr & 0x3FF) + 1] << 8) | (oam[addr & 0x3FF]));
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

void GPU2D::UpdatePixel(u32 pixel, u32 colour) {
    for (int offset = 0; offset < 4; offset++) {
        framebuffer[(pixel * 4) + offset] = colour & 0xFF;
        colour >>= 8;
    }
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
    case 5:
        if (DISPCNT & (1 << 8)) {
            RenderText(0, line);
        }
        if (DISPCNT & (1 << 9)) {
            RenderText(1, line);
        }
        if (DISPCNT & (1 << 10)) {
            RenderExtended(2, line);
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
            u32 screen_addr = 0x06000000 + screen_base + (((i / 8) % 32) * 2);
            // iterate through each pixel in that row
            u32 tile_info;
            if (engine_id == 1) {
                tile_info = gpu->ReadBGA(screen_addr);
            } else {
                tile_info = gpu->ReadBGB(screen_addr);
            }

            if (DISPCNT & (1 << 30)) {
                log_warn("implement support for bg extended palettes");
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
            if (vertical_flip) {
                log_fatal("handle");
            }
            // times by 64 as each tile is 64 bytes long
            u32 character_addr = 0x06000000 + character_base + (tile_number * 64);

            // TODO: make this faster
            for (int j = 0; j < 8; j++) {
                // now get the individual pixels from the tile
                u32 offset = character_addr + ((line % 8) * 8) + ((horizontal_flip) ? (7 - j) : j);
                // u32 offset = character_addr + ((line % 8) * 8) + j;

                // TODO: definitely something wrong here, fix later
                u16 data;
                if (engine_id == 1) {
                    data = gpu->ReadBGA(offset);
                } else {
                    data = gpu->ReadBGB(offset);
                }

                u16 colour = ReadPaletteRAM(data & 0xFF);

                framebuffer[(256 * line) + i + j] = Convert15To24(colour);
            }
        }
    } else {
        // 16 palettes with 16 colours per palette
        // each tile occupies 32 bytes of memory, with the first row using 4 bytes so
        // a pair of tiles will use 1 byte, with the lower 4 bits defining the colour
        // index in palette for the first pixel and the upper 4 bits defining the colour
        // index in palette for the second pixel
        // log_fatal("handle 16 palettes / 16 colours mode");
        for (int i = 0; i < 256; i += 8) {
            // iterate through a row of 32 tiles
            // get the address in memory for the index of the tiledata
            u32 screen_addr = 0x06000000 + screen_base + (((i / 8) % 32) * 2);
            u32 tile_info;
            if (engine_id == 1) {
                tile_info = gpu->ReadBGA(screen_addr);
            } else {
                tile_info = gpu->ReadBGB(screen_addr);
            }

            if (DISPCNT & (1 << 30)) {
                log_fatal("handle");
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
            // the palette number is used to select from 1 of 16 possible palettes
            u8 palette_number = (tile_info >> 12) & 0xF;
            // also each colour in a palette is 2 bytes long and we have 16 colours,
            // so each palette is 32 bytes

            if (vertical_flip) {
                log_fatal("handle");
            }
            // times tile_number by 32 as each tile is 32 bytes long
            u32 character_addr = 0x06000000 + character_base + (tile_number * 32);
            u32 palette_indices = (ReadPaletteRAM(character_addr + 2) << 16) | (ReadPaletteRAM(character_addr));
            // we use 4 bits for the palette index of a pixel
            for (int j = 0; j < 8; j++) {
                // draw each pixel in a row of a tile

                u16 colour = ReadPaletteRAM((palette_number * 32) + (palette_indices & 0xF));

                framebuffer[(256 * line) + i + j] = Convert15To24(colour);
                // shift by 4 bits to access the next palette index
                palette_indices >>= 4;
            }
        }
    }
}

void GPU2D::RenderExtended(int bg_index, u16 line) {
    u32 screen_base = ((BGCNT[bg_index] >> 8) & 0x1F) * 0x4000;
    u8 screen_size = (BGCNT[bg_index] >> 14) & 0x3;
    printf("screen size: %d\n", screen_size);


    // using bits 7 and 2 of bgcnt see which "extended mode we choose"
    // bit 2 of bgcnt = 1 and bit 7 of bgcnt = 1 means rot/scal direct color bitmap
    // bit 2 of bgcnt = 0 and bit 7 of bgcnt = 1 means rot/scal 256 color bitmap
    // bit 7 of bgcnt = 0 means rot/scal with 16 bit bgmap entries (text+affine mixup)
    if (BGCNT[bg_index] & (1 << 7)) {
        if (BGCNT[bg_index] & (1 << 2)) {
            // rot/scal direct color bitmap
            log_fatal("implement support for rot/scal direct color bitmap");
        } else {
            // rot/scal 256 color bitmap
            log_fatal("implement support for rot/scal 256 color bitmap");
        }
    } else {
        // rot/scal with 16 bit bgmap entries (text+affine mixup)
        log_fatal("implement support for rot/scal with 16 bit bgmap entries (text+affine mixup)");
    }
    


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