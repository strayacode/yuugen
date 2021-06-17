#pragma once

#include <common/types.h>
#include <common/memory_helpers.h>
#include <core/hw/gpu/engine_2d/gpu_2d.h>
#include <core/hw/gpu/engine_3d/render_engine.h>
#include <core/hw/gpu/engine_3d/geometry_engine.h>
#include <string.h>
#include <functional>
#include <array>

struct Core;

enum Screen {
    BOTTOM_SCREEN,
    TOP_SCREEN,
};

// notes on the new vram mapping approach:
// split each vram region into 4kb pages
// on vramcnt writes remap certain regions
// to avoid overhead hopefully just remap a specific block
// for a specific vramcnt

struct GPU {
    GPU(Core* core);

    auto GetFramebuffer(Screen screen) -> const u32*;
    void Reset();

    auto GetVRAMCNTMST(u8 vramcnt) -> int;
    auto GetVRAMCNTOffset(u8 vramcnt) -> int;
    auto GetVRAMCNTEnabled(u8 vramcnt) -> bool; 


    void WriteDISPSTAT7(u16 data);
    void WriteDISPSTAT9(u16 data);

    template <typename T>
    auto ReadARM7(u32 addr) -> T;

    template <typename T>
    void WriteARM7(u32 addr, T data);

    template <typename T>
    auto ReadExtPaletteBGA(u32 addr) -> T;

    template <typename T>
    auto ReadExtPaletteBGB(u32 addr) -> T;

    template <typename T>
    auto ReadExtPaletteOBJA(u32 addr) -> T;

    template <typename T>
    auto ReadExtPaletteOBJB(u32 addr) -> T;

    template <typename T>
    auto ReadVRAM(u32 addr) -> T;

    template <typename T>
    void WriteVRAM(u32 addr, T data);

    void RenderScanlineStart();
    void RenderScanlineFinish();

    void MapVRAM();

    u16 POWCNT1;
    
    u8 VRAMCNT_A;
    u8 VRAMCNT_B;
    u8 VRAMCNT_C;
    u8 VRAMCNT_D;
    u8 VRAMCNT_E;
    u8 VRAMCNT_F;
    u8 VRAMCNT_G;
    u8 VRAMCNT_H;
    u8 VRAMCNT_I;

    u8 VRAMSTAT;

    u16 DISPSTAT7, DISPSTAT9;

    Core* core;

    // understanding: these blocks of vram make up 656kb and are able to be dynamically mapped to the vram region
    u8 VRAM_A[0x20000];
    u8 VRAM_B[0x20000];
    u8 VRAM_C[0x20000];
    u8 VRAM_D[0x20000];
    u8 VRAM_E[0x10000];
    u8 VRAM_F[0x4000];
    u8 VRAM_G[0x4000];
    u8 VRAM_H[0x8000];
    u8 VRAM_I[0x4000];

    std::array<u8*, 0xA4> lcdc;
    std::array<u8*, 0x80> bga;
    std::array<u8*, 0x20> obja;
    std::array<u8*, 0x40> bgb;
    std::array<u8*, 0x20> objb;    

    GPU2D engine_a, engine_b;

    RenderEngine render_engine;
    GeometryEngine geometry_engine;


    u16 VCOUNT;

    u32 DISPCAPCNT;

    std::function<void()> RenderScanlineStartTask = std::bind(&GPU::RenderScanlineStart, this);
    std::function<void()> RenderScanlineFinishTask = std::bind(&GPU::RenderScanlineFinish, this);
};