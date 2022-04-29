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
    Colour sample_texture(int s, int t, TextureAttributes attributes);
};