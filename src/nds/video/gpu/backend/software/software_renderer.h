#pragma once

#include <array>
#include "common/types.h"
#include "nds/video/gpu/backend/renderer.h"
#include "nds/video/gpu/backend/software/interpolator.h"
#include "nds/video/gpu/backend/software/slope.h"

namespace nds {

class SoftwareRenderer : public Renderer {
public:
    void reset() override;
    void render() override;
    
    const u32* get_framebuffer() override { return framebuffer.data(); }

    void submit_polygons(Polygon* polygons, int num_polygons, bool w_buffering) override;

private:
    void render_scanline(int y);
    void render_polygon_scanline(Polygon& polygon, int y);
    bool depth_test(u32 old_depth, u32 depth, bool equal);

    std::array<u32, 256 * 192> framebuffer;
    std::array<u32, 256 * 192> depth_buffer;
    Polygon* polygons{nullptr};
    int num_polygons{0};
    bool w_buffering{false};

    Interpolator<9> slope_interpolator;
    Interpolator<8> scanline_interpolator;
    Slope left_slope;
    Slope right_slope;
};

} // namespace nds