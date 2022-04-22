#pragma once

#include <memory>
#include "Common/Types.h"
#include "VideoCommon/Renderer2D.h"
#include "VideoCommon/Renderer3D.h"
#include "Core/scheduler/scheduler.h"

class System;

class GPU {
public:
    void GPU(System& system);
    void reset();

    u8 read_byte(u32 addr);
    u16 read_half(u32 addr);
    u32 read_word(u32 addr);

    void write_byte(u32 addr, u8 data);
    void write_half(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    enum class RendererType {
        Software,
    };

    enum class Screen {
        Top,
        Bottom,
    };

    void create_renderers(RendererType type);
    const u32* get_framebuffer(Screen screen);

    std::unique_ptr<Renderer2D> renderer_2d[2];
    std::unique_ptr<Renderer3D> renderer_3d;

    // shared mmio
    u16 powcnt1 = 0;
    
    // 2d engine mmio

    // 3d engine mmio

private:
    void render_scanline_start();
    void render_scanline_end();

    EventType scanline_start_event;
    EventType scanline_end_event;

    System& system;
};