#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>

struct GPU;

struct GPU2D {
    GPU2D(GPU* gpu, int engine_id);

    u32 framebuffer[256 * 192];

    u32 DISPCNT;

    u16 BGCNT[4];
    u16 BGHOFS[4];
    u16 BGVOFS[4];
    u16 BG2P[4];
    u32 BG2X;
    u32 BG2Y;
    u32 BG3X;
    u32 BG3Y;
    u16 BG3P[4];
    u16 WINH[2];
    u16 WINV[2];
    u16 WININ;
    u16 WINOUT;
    u32 MOSAIC;
    u16 BLDCNT;
    u16 BLDALPHA;
    u32 BLDY;

    u16 MASTER_BRIGHT;

    // each gpu engine holds 1kb of palette ram with the first 512 bytes for bg palette and the remaining memory for obj palette
    u16 palette_ram[0x400];

    // each gpu engine holds 1kb of oam which allows for a total of 128 oam entries for each gpu engine (128 sprites)
    u16 oam[0x400];

    void WritePaletteRAM(u32 addr, u16 data);
    void WriteOAM(u32 addr, u16 data);

    u16 ReadPaletteRAM(u32 addr);

    void Reset();
    const u32* GetFramebuffer();
    u32 Convert15To24(u32 colour);

    void RenderScanline(u16 line);

    void RenderBlankScreen(u16 line);
    void RenderVRAMDisplay(u16 line);
    void RenderGraphicsDisplay(u16 line);
    void RenderText(int bg_index, u16 line);
    void RenderExtended(int bg_index, u16 line);

    GPU* gpu;

    int engine_id;
};