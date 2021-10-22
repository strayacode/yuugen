#include <core/hw/gpu/engine_3d/render_engine.h>
#include <core/hw/gpu/gpu.h>

RenderEngine::RenderEngine(GPU* gpu) : gpu(gpu) {

}

void RenderEngine::Reset() {
    disp3dcnt = 0;
    clear_colour = 0;
    clear_depth = 0;
    vertex_ram_size = 0;
    clrimage_offset = 0;
    fog_colour = 0;
    fog_offset = 0;
    alpha_test_ref = 0;
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));
}

void RenderEngine::Render() {
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));

    for (int i = 0; i < vertex_ram_size; i++) {
        Vertex vertex = vertex_ram[i];
        framebuffer[(vertex.y * 256) + vertex.x] = 0xFFFFFFFF;
    }
}