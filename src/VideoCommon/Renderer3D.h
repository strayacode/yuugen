#pragma once

#include <array>
#include "Common/Types.h"

class Renderer3D {
public:
    virtual void render() = 0;

private:
    std::array<u32, 256 * 192> framebuffer;
}