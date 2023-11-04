#pragma once

#include "common/types.h"
#include "nds/video/gpu/polygon.h"

namespace nds {

struct Renderer {
    virtual ~Renderer() = default;

    virtual void reset() = 0;
    virtual void render() = 0;
    virtual const u32* get_framebuffer() = 0;
    virtual void submit_polygons(Polygon* polygons, int num_polygons) = 0;
};

} // namespace nds