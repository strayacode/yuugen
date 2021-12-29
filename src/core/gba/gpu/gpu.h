#pragma once

#include <array>
#include "common/types.h"
#include "core/scheduler/scheduler.h"
#include <string.h>

class GBA;

class GBAGPU {
public:
    GBAGPU(GBA& gba);

    void Reset();
    void RenderScanlineStart();
    void RenderScanlineFinish();
    void RenderScanline(u16 line);
    void RenderMode3(u16 line);
    void RenderMode4(u16 line);
    void ComposeScanline(u16 line);
    void ComposePixel(u16 line, u16 x);
    const u32* GetFramebuffer(int screen);
    u32 Convert15To24(u32 colour);

    u16 dispcnt;
    u16 dispstat;
    u16 vcount;
    std::array<u8, 0x400> palette_ram;
    std::array<u8, 0x18000> vram;
    std::array<u32, 240 * 160> framebuffer;
    u16 bg_layers[4][240 * 160];

private:
    GBA& gba;

    EventType scanline_start_event;
    EventType scanline_finish_event;
};