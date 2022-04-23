#pragma once

#include <memory>
#include "VideoCommon/Renderer2D.h"

class GPU;

class SoftwareRenderer2D : public Renderer2D {
public:
    SoftwareRenderer2D(GPU& gpu, Engine engine);
    void render_scanline(int line) override;
private:
};