#pragma once

#include <memory>
#include "Common/Types.h"
#include "VideoCommon/Renderer2D.h"
#include "VideoCommon/Renderer3D.h"
#include "VideoCommon/VRAMRegion.h"
#include "Core/scheduler/scheduler.h"

enum class RendererType {
    Software,
};

enum class Screen {
    Top,
    Bottom,
};

class System;

class GPU {
public:
    GPU(System& system);
    void reset();

    u8 read_byte(u32 addr);
    u16 read_half(u32 addr);
    u32 read_word(u32 addr);

    void write_byte(u32 addr, u8 data);
    void write_half(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    void create_renderers(RendererType type);
    const u32* get_framebuffer(Screen screen);

    void update_vram_mapping();
    void reset_vram_mapping();

    template <typename T>
    T read_vram(u32 addr);

    template <typename T>
    void write_vram(u32 addr, T data);

    const VRAMRegion<128>& get_texture_data() { return texture_data; }
    const VRAMRegion<96>& get_texture_palette() { return texture_palette; }
    u8* get_palette_ram() { return palette_ram.data(); }
    u8* get_oam() { return oam.data(); }

    std::unique_ptr<Renderer2D> renderer_2d[2];
    std::unique_ptr<Renderer3D> renderer_3d;

    // shared mmio
    u16 powcnt1 = 0;
    u8 vramcnt_a = 0;
    u8 vramcnt_b = 0;
    u8 vramcnt_c = 0;
    u8 vramcnt_d = 0;
    u8 vramcnt_e = 0;
    u8 vramcnt_f = 0;
    u8 vramcnt_g = 0;
    u8 vramcnt_h = 0;
    u8 vramcnt_i = 0;
    
    // 2d engine mmio

    // 3d engine mmio

private:
    void render_scanline_start();
    void render_scanline_end();

    int get_bank_mst(u8 vramcnt);
    int get_bank_offset(u8 vramcnt);
    bool get_bank_enabled(u8 vramcnt);

    EventType scanline_start_event;
    EventType scanline_end_event;

    VRAMRegion<656> lcdc;
    VRAMRegion<512> bga;
    VRAMRegion<256> obja;
    VRAMRegion<128> bgb;
    VRAMRegion<128> objb;
    VRAMRegion<128> arm7_vram;
    VRAMRegion<128> texture_data;
    VRAMRegion<96> texture_palette;

    std::array<u8, 0x20000> bank_a = {};
    std::array<u8, 0x20000> bank_b = {};
    std::array<u8, 0x20000> bank_c = {};
    std::array<u8, 0x20000> bank_d = {};
    std::array<u8, 0x10000> bank_e = {};
    std::array<u8, 0x4000> bank_f = {};
    std::array<u8, 0x4000> bank_g = {};
    std::array<u8, 0x8000> bank_h = {};
    std::array<u8, 0x4000> bank_i = {};

    std::array<u8, 0x800> palette_ram = {};
    std::array<u8, 0x800> oam = {};

    System& system;
};