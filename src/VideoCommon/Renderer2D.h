#pragma once

#include <array>
#include "Common/Types.h"

class GPU;

enum class Engine {
    A,
    B,
};

class Renderer2D {
public:
    Renderer2D(GPU& gpu, Engine engine);

    void reset();
    virtual void render_scanline(int line) = 0;
    const u32* get_framebuffer() { return framebuffer.data(); }

    u8 read_byte(u32 addr);
    u16 read_half(u32 addr);
    u32 read_word(u32 addr);

    void write_byte(u32 addr, u8 data);
    void write_half(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    // mmio
    u32 dispcnt = 0;

private:
    std::array<u32, 256 * 192> framebuffer = {};
    u8* palette_ram = nullptr;
    u8* oam = nullptr;
    u32 vram_addr = 0;
    u32 obj_addr = 0;

    GPU& gpu;
    Engine engine;
};