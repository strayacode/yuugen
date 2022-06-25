#pragma once

#include <array>
#include "Common/GPUTypes.h"
#include "VideoCommon/Renderer3D.h"
#include "VideoBackends3D/Software/Interpolator.h"

class VideoUnit;

class SoftwareRenderer3D : public Renderer3D {
public:
    SoftwareRenderer3D(VideoUnit& video_unit);
    
    void render() override;

private:
    void render_scanline(int y);
    void render_polygon_scanline(Polygon& polygon, int y);
    Colour decode_texture(int s, int t, TextureAttributes attributes);
    bool depth_test(u32 old_depth, u32 depth, bool equal);
    
    enum class TextureFormat {
        None,
        A3I5,
        Palette4Colour,
        Palette16Colour,
        Palette256Colour,
        Compressed,
        A5I3,
        DirectColour,
    };

    Interpolator<9> slope_interpolator;
    Interpolator<8> scanline_interpolator;
    Slope left_slope;
    Slope right_slope;
};