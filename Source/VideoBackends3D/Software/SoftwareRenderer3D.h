#pragma once

#include "Common/GPUTypes.h"
#include "VideoCommon/Renderer3D.h"

class GPU;

class SoftwareRenderer3D : public Renderer3D {
public:
    SoftwareRenderer3D(GPU& gpu);
    
    void render() override;

private:
    void render_polygon(Polygon& polygon);
    Colour decode_texture(int s, int t, TextureAttributes attributes);

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
};