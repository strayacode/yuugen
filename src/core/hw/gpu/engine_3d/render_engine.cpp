#include <core/hw/gpu/engine_3d/render_engine.h>
#include <core/hw/gpu/gpu.h>

RenderEngine::RenderEngine(GPU* gpu) : gpu(gpu) {

}

void RenderEngine::Reset() {
    DISP3DCNT = 0;
    screen_x1 = screen_x2 = screen_y1 = screen_y2 = 0;
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));

    vertex_ram.clear();
    polygon_ram.clear();
    vertex_ram.reserve(6144);
    vertex_ram.resize(6144);
}

void RenderEngine::Render() {
    // clear the framebuffer before starting
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));

    // just draw vertices for now lmao
    for (int i = 0; i < vertex_ram.size(); i++) {
        u32 offset = (vertex_ram[i].y * 256) + vertex_ram[i].x;
        framebuffer[offset] = 0xFFFFFF;
    }
}