#include <core/hw/gpu/engine_3d/render_engine.h>
#include <core/hw/gpu/gpu.h>

RenderEngine::RenderEngine(GPU* gpu) : gpu(gpu) {

}

void RenderEngine::Reset() {
    DISP3DCNT = 0;
    screen_x1 = screen_x2 = screen_y1 = screen_y2 = 0;
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));
}