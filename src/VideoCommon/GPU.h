#pragma once

#include <memory>
#include <thread>
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
    ~GPU();
    void reset();

    void create_renderers(RendererType type);
    const u32* get_framebuffer(Screen screen);

    enum class Bank {
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
    };

    void update_vram_mapping(Bank bank, u8 data);
    void reset_vram_mapping();

    template <typename T>
    T read_vram(u32 addr);

    template <typename T>
    void write_vram(u32 addr, T data);

    template <typename T>
    T read_ext_palette_bga(u32 addr);

    template <typename T>
    T read_ext_palette_bgb(u32 addr);

    template <typename T>
    T read_ext_palette_obja(u32 addr);

    template <typename T>
    T read_ext_palette_objb(u32 addr);

    const VRAMRegion<128>& get_texture_data() { return texture_data; }
    const VRAMRegion<96>& get_texture_palette() { return texture_palette; }
    u8* get_palette_ram() { return palette_ram.data(); }
    u8* get_oam() { return oam.data(); }

    void render_thread_start();
    void render_thread_stop();

    std::unique_ptr<Renderer2D> renderer_2d[2];
    std::unique_ptr<Renderer3D> renderer_3d;

    // shared mmio
    u16 powcnt1 = 0;
    u16 vcount = 0;
    std::array<u16, 2> dispstat = {};
    std::array<u8, 9> vramcnt = {};
    u32 dispcapcnt = 0;
    
    // 2d engine mmio

    // 3d engine mmio

    // state required by the renderers
    std::array<u8, 0x20000> bank_a = {};
    std::array<u8, 0x20000> bank_b = {};
    std::array<u8, 0x20000> bank_c = {};
    std::array<u8, 0x20000> bank_d = {};
    std::array<u8, 0x10000> bank_e = {};
    std::array<u8, 0x4000> bank_f = {};
    std::array<u8, 0x4000> bank_g = {};
    std::array<u8, 0x8000> bank_h = {};
    std::array<u8, 0x4000> bank_i = {};

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

    std::array<u8, 0x800> palette_ram = {};
    std::array<u8, 0x800> oam = {};

    std::thread render_thread;
    bool render_thread_running = false;

    System& system;
};