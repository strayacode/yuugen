#pragma once

#include "VideoCommon/Renderer3D.h"

class GPU;

class SoftwareRenderer3D : public Renderer3D {
public:
    SoftwareRenderer3D(GPU& gpu);
    
    void render() override;
private:
};