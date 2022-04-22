#pragma once

#include <memory>
#include "Common/Types.h"
#include "VideoCommon/Renderer2D.h"
#include "VideoCommon/Renderer3D.h"

class GPU {
public:
    void reset();

    enum class RendererType {
        Software,
    };

    enum class Screen {
        Top,
        Bottom,
    };

    void create_renderers(RendererType type);
    const u32* get_framebuffer(Screen screen);

    std::unique_ptr<Renderer2D> renderer_2d[2];
    std::unique_ptr<Renderer3D> renderer_3d;

    // shared mmio
    u16 powcnt1 = 0;
    
    // 2d engine mmio

    // 3d engine mmio
private:
};