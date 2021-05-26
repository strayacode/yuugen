#pragma once

#include <common/types.h>
#include <common/memory_helpers.h>
#include <core/hw/gpu/gpu_2d.h>
#include <core/hw/gpu/gpu_3d.h>
#include <string.h>
#include <functional>

struct Core;

enum Screen {
    BOTTOM_SCREEN,
    TOP_SCREEN,
};

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
    void WriteLCDC(u32 addr, T data);

    template <typename T>
    void WriteBGA(u32 addr, T data);

    template <typename T>
    void WriteBGB(u32 addr, T data);

    template <typename T>
    auto ReadLCDC(u32 addr) -> T;

    template <typename T>
    auto ReadBGA(u32 addr) -> T;

    template <typename T>
    auto ReadBGB(u32 addr) -> T;

    template <typename T>
    auto ReadARM7(u32 addr) -> T;

    template <typename T>
    void WriteARM7(u32 addr, T data);

    template <typename T>
    void WriteOBJA(u32 addr, T data);

    template <typename T>
    void WriteOBJB(u32 addr, T data);

    template <typename T>
    auto ReadOBJA(u32 addr) -> T;

    template <typename T>
    auto ReadOBJB(u32 addr) -> T;

    template <typename T>
    auto ReadExtPaletteBGA(u32 addr) -> T;

    template <typename T>
    auto ReadExtPaletteBGB(u32 addr) -> T;

    template <typename T>
    auto ReadExtPaletteOBJA(u32 addr) -> T;

    template <typename T>
    auto ReadExtPaletteOBJB(u32 addr) -> T;

    void RenderScanlineStart();
    void RenderScanlineFinish();

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

    GPU2D engine_a, engine_b;

    GPU3D engine_3d;

    u16 VCOUNT;

    u32 DISPCAPCNT;

    std::function<void()> RenderScanlineStartTask = std::bind(&GPU::RenderScanlineStart, this);
    std::function<void()> RenderScanlineFinishTask = std::bind(&GPU::RenderScanlineFinish, this);
};