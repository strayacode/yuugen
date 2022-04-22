#pragma once

#include <array>
#include "Common/Types.h"

class Renderer2D {
public:
    virtual void render_scanline() = 0;

private:
    std::array<u32, 256 * 192> framebuffer;
}