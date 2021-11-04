#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/memory_helpers.h>
#include <string.h>

#define COLOUR_TRANSPARENT 0x8000

struct OBJPixel {
    u16 colour;
    u8 priority;
};

class GPU;

class GPU2D {
public:
    GPU2D(GPU* gpu, int engine_id);

    void Reset();
    const u32* GetFramebuffer();
    void RenderScanline(u16 line);
    void RenderVRAMDisplay(u16 line);
    void RenderBlankScreen(u16 line);
    void RenderGraphicsDisplay(u16 line);
    void RenderText(int bg_index, u16 line);
    void RenderExtended(int bg_index, u16 line);
    void RenderObjects(u16 line);
    void RenderAffine(int bg_index, u16 line);
    void RenderLarge(int bg_index, u16 line);

    u32 Convert15To24(u32 colour);

    template <typename T>
    T ReadPaletteRAM(u32 addr);

    template <typename T>
    T ReadOAM(u32 addr);

    template <typename T>
    void WritePaletteRAM(u32 addr, T data);

    template <typename T>
    void WriteOAM(u32 addr, T data);

    void ComposeScanline(u16 line);
    void ComposePixel(u16 line, u16 x);

    void WriteBGX(int bg_index, u32 data);
    void WriteBGY(int bg_index, u32 data);

    u32 framebuffer[256 * 192];

    u16 bg_layers[4][256 * 192];

    OBJPixel obj_layer[256 * 192];

    u32 DISPCNT;

    u32 vram_addr;
    u32 obj_addr;

    u16 BGCNT[4];
    u16 BGHOFS[4];
    u16 BGVOFS[4];
    u16 BGPA[2];
    u16 BGPB[2];
    u16 BGPC[2];
    u16 BGPD[2];
    u32 BGX[2];
    u32 BGY[2];

    u32 internal_x[2];
    u32 internal_y[2];

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
    u8 palette_ram[0x400];

    // each gpu engine holds 1kb of oam which allows for a total of 128 oam entries for each gpu engine (128 sprites)
    u8 oam[0x400];

    GPU* gpu;

    int engine_id;

    static constexpr int dimensions[3][4][2] = {{{8, 8}, {16, 16}, {32, 32}, {64, 64}}, {{16, 8}, {32, 8}, {32, 16}, {64, 32}}, {{8, 16}, {8, 32}, {16, 32}, {32, 64}}};
};