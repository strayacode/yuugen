#pragma once

#include <common/types.h>
#include <common/vertex.h>
#include <common/polygon.h>

class GPU;

class RenderEngine {
public:
    RenderEngine(GPU* gpu);
    void Reset();
    void Render();
    void RenderPolygon(Polygon& polygon);
    Vertex NormaliseVertex(Vertex vertex);
    Colour interpolate_colour(Colour c1, Colour c2, u32 a, u32 p);
    u32 lerp(u32 u1, u32 u2, u32 a, u32 p);

    u16 disp3dcnt;
    u32 clear_colour;
    u16 clear_depth;
    u16 clrimage_offset;
    u32 fog_colour;
    u16 fog_offset;
    u8 alpha_test_ref;

    u8 edge_colour[0x10];
    u8 fog_table[0x20];
    u8 toon_table[0x40];

    Vertex vertex_ram[6144];
    int vertex_ram_size;

    Polygon polygon_ram[2048];
    int polygon_ram_size;

    u32 framebuffer[256 * 192];

    GPU* gpu;
};