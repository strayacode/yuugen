#pragma once

#include <array>
#include "Common/Types.h"

class Renderer2D {
public:
    virtual void render_scanline(int line) = 0;
    const u32* get_framebuffer() { return framebuffer.data(); }

private:
    std::array<u32, 256 * 192> framebuffer;
};