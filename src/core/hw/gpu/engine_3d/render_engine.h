#pragma once

#include <common/types.h>
#include <common/log.h>

struct GPU;

struct RenderEngine {
    RenderEngine(GPU* gpu);
    void Reset();

    u16 DISP3DCNT;
    u8 RDLINES_COUNT;
    u8 EDGE_COLOR[0x10];
    u8 ALPHA_TEST_REF;
    u32 CLEAR_COLOR;
    u16 CLEAR_DEPTH;
    u16 CLRIMAGE_OFFSET;
    u32 FOG_COLOR;
    u16 FOG_OFFSET;
    u8 FOG_TABLE[0x20];
    u8 TOON_TABLE[0x40];

    GPU* gpu;
};