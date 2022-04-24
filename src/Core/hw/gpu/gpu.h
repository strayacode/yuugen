#pragma once

#include <functional>
#include <array>
#include "Common/Log.h"
#include "Common/Types.h"
#include "Core/hw/gpu/engine_2d/gpu_2d.h"
#include "Core/hw/gpu/engine_3d/render_engine.h"
#include "Core/hw/gpu/engine_3d/geometry_engine.h"
#include "Core/hw/gpu/VRAMRegion.h"
#include "Core/scheduler/scheduler.h"

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
    T read_vram(u32 addr);

    template <typename T>
    void write_vram(u32 addr, T data);

    template <typename T>
    auto ReadARM7(u32 addr) -> T;

    template <typename T>
    void WriteARM7(u32 addr, T data);

    void RenderScanlineStart();
    void RenderScanlineFinish();

    void update_vram_mapping();
    void reset_vram_mapping();

    u16 POWCNT1;
    
    u8 vramcnt_a;
    u8 vramcnt_b;
    u8 vramcnt_c;
    u8 vramcnt_d;
    u8 vramcnt_e;
    u8 vramcnt_f;
    u8 vramcnt_g;
    u8 vramcnt_h;
    u8 vramcnt_i;

    u8 VRAMSTAT;

    u16 DISPSTAT7, DISPSTAT9;

    System& system;

    std::array<u8, 0x20000> bank_a;
    std::array<u8, 0x20000> bank_b;
    std::array<u8, 0x20000> bank_c;
    std::array<u8, 0x20000> bank_d;
    std::array<u8, 0x10000> bank_e;
    std::array<u8, 0x4000> bank_f;
    std::array<u8, 0x4000> bank_g;
    std::array<u8, 0x8000> bank_h;
    std::array<u8, 0x4000> bank_i;

    GPU2D engine_a, engine_b;

    RenderEngine render_engine;
    GeometryEngine geometry_engine;

    u16 VCOUNT;

    u32 DISPCAPCNT;

    EventType scanline_start_event;
    EventType scanline_finish_event;

    VRAMRegion<128> texture_data;
    VRAMRegion<96> texture_palette;

private:
    int get_bank_mst(u8 vramcnt);
    int get_bank_offset(u8 vramcnt);
    bool get_bank_enabled(u8 vramcnt);

    VRAMRegion<656> lcdc;
    VRAMRegion<512> bga;
    VRAMRegion<256> obja;
    VRAMRegion<128> bgb;
    VRAMRegion<128> objb;
    VRAMRegion<128> arm7_vram;
};