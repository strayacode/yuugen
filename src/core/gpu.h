#pragma once

#include <util/types.h>
#include <util/memory_helpers.h>
#include <core/gpu_2d.h>
#include <string.h>

struct Core;

enum class Screen {
    BOTTOM_SCREEN,
    TOP_SCREEN,
};



struct GPU {
    GPU(Core* core);

    const u32* GetFramebuffer(Screen screen);

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

    void Reset();

    int GetVRAMCNTMST(u8 vramcnt);
    int GetVRAMCNTOffset(u8 vramcnt);
    bool GetVRAMCNTEnabled(u8 vramcnt); 


    void WriteDISPSTAT7(u16 data);
    void WriteDISPSTAT9(u16 data);

    void WriteLCDC(u32 addr, u16 data);
    void WriteBGA(u32 addr, u16 data);
    void WriteBGB(u32 addr, u16 data);

    u16 ReadLCDC(u32 addr);
    u16 ReadBGA(u32 addr);
    u16 ReadBGB(u32 addr);
    u16 ReadARM7(u32 addr);

    void RenderScanlineStart();
    void RenderScanlineFinish();

    GPU2D engine_a, engine_b;

    u16 VCOUNT;

    Core* core;
};