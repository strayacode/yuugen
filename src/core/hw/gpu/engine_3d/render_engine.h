#pragma once

class GPU;

class RenderEngine {
public:
    RenderEngine(GPU* gpu);
    void Reset();

    GPU* gpu;
};