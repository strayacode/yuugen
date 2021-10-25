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
    polygon_ram_size = 0;
}

void RenderEngine::Render() {
    memset(framebuffer, 0, 256 * 192 * sizeof(u32));

    // iterate through all the polygons instead and use our line interpolation stuff
    for (int i = 0; i < polygon_ram_size; i++) {
        Polygon polygon = polygon_ram[i];

        for (int j = 0; j < polygon.size; j++) {
            Vertex vertex = polygon.vertices[j];

            if ((vertex.x >= 0 && vertex.x < 256) && (vertex.y >= 0 && vertex.y < 192)) {
                framebuffer[(vertex.y * 256) + vertex.x] = 0xFFFFFFFF;
            }
        }
    }
}