#pragma once

#include <common/types.h>
#include <common/vertex.h>
#include <string.h>

class GPU;

class RenderEngine {
public:
    RenderEngine(GPU* gpu);
    void Reset();
    void Render();

    u16 disp3dcnt;
    u32 clear_colour;
    u16 clear_depth;
    u16 clrimage_offset;
    u32 fog_colour;
    u16 fog_offset;

    u8 edge_colour[0x10];
    u8 fog_table[0x20];
    u8 toon_table[0x40];

    Vertex vertex_ram[6144];
    int vertex_ram_size;

    u32 framebuffer[256 * 192];

    GPU* gpu;
};