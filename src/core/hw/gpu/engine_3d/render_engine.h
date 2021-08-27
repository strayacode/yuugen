#pragma once

#include <common/types.h>
#include <common/vertex.h>

class GPU;

class RenderEngine {
public:
    RenderEngine(GPU* gpu);
    void Reset();
    void Render();

    u16 disp3dcnt;
    u32 clear_colour;
    u16 clear_depth;

    Vertex vertex_ram[6144];
    int vertex_ram_size;

    GPU* gpu;
};