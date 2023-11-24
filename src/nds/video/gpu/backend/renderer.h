#pragma once

#include "common/types.h"
#include "nds/video/gpu/polygon.h"

namespace nds {

struct Renderer {
    virtual ~Renderer() = default;

    virtual void reset() = 0;
    virtual void render() = 0;
    virtual u32* fetch_framebuffer() = 0;
    virtual void submit_polygons(Polygon* polygons, int num_polygons, bool w_buffering) = 0;
};

} // namespace nds