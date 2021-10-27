#pragma once

#include <common/types.h>
#include <common/memory_helpers.h>
#include <core/hw/gpu/engine_2d/gpu_2d.h>
#include <core/hw/gpu/engine_3d/render_engine.h>
#include <core/hw/gpu/engine_3d/geometry_engine.h>
#include <core/hw/gpu/vram_page.h>
#include <string.h>
#include <functional>
#include <array>
#include <core/scheduler/scheduler.h>

class System;

enum class Screen {
    Top,
    Bottom,
};

class GPU {
public:
    GPU(System& system);

    auto GetFramebuffer(Screen screen) -> const u32*;
    void Reset();

    auto GetVRAMCNTMST(u8 vramcnt) -> int;
    auto GetVRAMCNTOffset(u8 vramcnt) -> int;
    auto GetVRAMCNTEnabled(u8 vramcnt) -> bool; 

    void WriteDISPSTAT7(u16 data);
    void WriteDISPSTAT9(u16 data);

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

    template <typename T>
    auto ReadARM7(u32 addr) -> T;

    template <typename T>
    void WriteARM7(u32 addr, T data);

    void RenderScanlineStart();
    void RenderScanlineFinish();

    void MapVRAM();

    void VRAMMappingReset();

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

    System& system;

    u8 bank_a[0x20000];
    u8 bank_b[0x20000];
    u8 bank_c[0x20000];
    u8 bank_d[0x20000];
    u8 bank_e[0x10000];
    u8 bank_f[0x4000];
    u8 bank_g[0x4000];
    u8 bank_h[0x8000];
    u8 bank_i[0x4000];

    std::array<VRAMPage, 0xA4> lcdc;
    std::array<VRAMPage, 0x80> bga;
    std::array<VRAMPage, 0x40> obja;
    std::array<VRAMPage, 0x20> bgb;
    std::array<VRAMPage, 0x20> objb;
    std::array<VRAMPage, 0x20> arm7_vram;    

    GPU2D engine_a, engine_b;

    RenderEngine render_engine;
    GeometryEngine geometry_engine;

    u16 VCOUNT;

    u32 DISPCAPCNT;

    EventType scanline_start_event;
    EventType scanline_finish_event;
};