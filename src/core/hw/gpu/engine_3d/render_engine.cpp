#include <core/hw/gpu/engine_3d/render_engine.h>
#include <core/hw/gpu/gpu.h>

RenderEngine::RenderEngine(GPU* gpu) : gpu(gpu) {

}

void RenderEngine::Reset() {
    disp3dcnt = 0;
    clear_colour = 0;
    clear_depth = 0;
    screen_x1 = screen_x2 = screen_y1 = screen_y2 = 0;
}