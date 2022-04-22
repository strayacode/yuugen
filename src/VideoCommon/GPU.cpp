#include "Common/Log.h"
#include "VideoCommon/GPU.h"

void GPU::reset() {
    powcnt1 = 0;
}

void GPU::create_renderers(RendererType type) {
    switch (type) {
    case RendererType::Software:
        renderer_2d[0] = std::make_unique<SoftwareRenderer2D>();
        renderer_2d[1] = std::make_unique<SoftwareRenderer2D>();
        renderer_3d = std::make_unique<SoftwareRenderer3D>();
        break;
    default:
        log_fatal("GPU: unknown renderer type %d", static_cast<int>(type));
    }
}

const u32* GPU::get_framebuffer(Screen screen) {
    if (((powcnt1 >> 15) & 0x1) == (screen == Screen::Top)) {
        return renderer_2d[0]->get_framebuffer();
    } else {
        return renderer_2d[1]->get_framebuffer()
    }
}