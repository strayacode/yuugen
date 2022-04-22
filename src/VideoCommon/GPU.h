#pragma once

#include <memory>
#include "VideoCommon/Renderer2D.h"
#include "VideoCommon/Renderer3D.h"

class GPU {
public:
    std::unique_ptr<Renderer2D> renderer_2d[2];
    std::unique_ptr<Renderer3D> renderer_3d;
    
private:
};