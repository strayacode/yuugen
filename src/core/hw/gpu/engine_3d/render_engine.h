#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/polygon.h>
#include <common/vertex.h>
#include <vector>

struct GPU;

// some things to think about:
// check before dividing by w for polygon clipping

struct RenderEngine {
    RenderEngine(GPU* gpu);
    void Reset();
    void Render();

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

    std::vector<Polygon> polygon_ram;
    std::vector<Vertex> vertex_ram;

    u16 screen_x1;
    u16 screen_x2;
    u16 screen_y1;
    u16 screen_y2;

    u32 framebuffer[256 * 192];

    GPU* gpu;
};