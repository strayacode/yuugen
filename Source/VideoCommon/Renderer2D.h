#pragma once

#include <array>
#include "Common/Types.h"
#include "Core/ARM/MMIO.h"

class VideoUnit;

enum class Engine {
    A,
    B,
};

class Renderer2D {
public:
    Renderer2D(VideoUnit& video_unit, Engine engine);

    void reset();
    void build_mmio(MMIO& mmio);

    virtual void render_scanline(int line) = 0;
    const u32* get_framebuffer() { return framebuffer.data(); }

    // mmio
    u32 dispcnt = 0;
    std::array<u16, 4> bgcnt = {};
    std::array<u16, 4> bghofs = {};
    std::array<u16, 4> bgvofs = {};
    std::array<u16, 2> bgpa = {};
    std::array<u16, 2> bgpb = {};
    std::array<u16, 2> bgpc = {};
    std::array<u16, 2> bgpd = {};
    std::array<u32, 2> bgx = {};
    std::array<u32, 2> bgy = {};
    std::array<u32, 2> internal_x = {};
    std::array<u32, 2> internal_y = {};
    std::array<u16, 2> winh = {};
    std::array<u16, 2> winv = {};
    u16 winin = 0;
    u16 winout = 0;
    u32 mosaic = 0;
    u16 bldcnt = 0;
    u16 bldalpha = 0;
    u32 bldy = 0;
    u16 master_bright = 0;
    
    std::array<u32, 256 * 192> framebuffer = {};
    std::array<std::array<u16, 256 * 192>, 4> bg_layers = {};
    std::array<u8, 256 * 192> obj_priority = {};
    std::array<u16, 256 * 192> obj_colour = {};

    u8* palette_ram;
    u8* oam;
    u32 vram_addr = 0;
    u32 obj_addr = 0;

    VideoUnit& video_unit;
    Engine engine;
};