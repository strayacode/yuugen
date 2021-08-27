#pragma once

#include <common/types.h>
#include <common/vertex.h>

class GPU;

class RenderEngine {
public:
    RenderEngine(GPU* gpu);
    void Reset();

    u16 disp3dcnt;
    u32 clear_colour;
    u16 clear_depth;

    Vertex vertex_ram[6188];

    u8 screen_x1;
    u8 screen_x2;
    u8 screen_y1;
    u8 screen_y2;

    GPU* gpu;
};