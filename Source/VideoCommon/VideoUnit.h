#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include "Common/Types.h"
#include "VideoCommon/Renderer2D.h"
#include "VideoCommon/Renderer3D.h"
#include "VideoCommon/VRAM.h"
#include "VideoBackends2D/Software/SoftwareRenderer2D.h"
#include "VideoBackends3D/Software/SoftwareRenderer3D.h"
#include "Core/Scheduler.h"
#include "Core/arm/MMIO.h"

enum class RendererType {
    Software,
};

enum class Screen {
    Top,
    Bottom,
};

class System;

class VideoUnit {
public:
    VideoUnit(System& system);
    ~VideoUnit();

    void reset();
    void build_mmio(MMIO& mmio, Arch arch);

    const u32* get_framebuffer(Screen screen);

    enum class ThreadState {
        Idle,
        DrawingA,
        DrawingB,
    };

    u8* get_palette_ram() { return palette_ram.data(); }
    u8* get_oam() { return oam.data(); }

    void start_render_thread();
    void stop_render_thread();

    SoftwareRenderer2D renderer_2d[2];
    SoftwareRenderer3D renderer_3d;

    // shared mmio
    u16 powcnt1 = 0;
    u16 vcount = 0;
    std::array<u16, 2> dispstat = {};
    u32 dispcapcnt = 0;
    
    VRAM vram;

    System& system;

private:
    void render_scanline_start();
    void render_scanline_end();

    EventType scanline_start_event;
    EventType scanline_end_event;

    std::array<u8, 0x800> palette_ram = {};
    std::array<u8, 0x800> oam = {};

    std::thread render_thread;
    bool render_thread_running = false;
    std::atomic<ThreadState> thread_state;
};