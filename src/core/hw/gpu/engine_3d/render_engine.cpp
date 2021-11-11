#include <common/slope.h>
#include <core/hw/gpu/engine_3d/render_engine.h>
#include <core/hw/gpu/gpu.h>

RenderEngine::RenderEngine(GPU* gpu) : gpu(gpu) {}

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

    for (int i = 0; i < polygon_ram_size; i++) {
        Polygon polygon = polygon_ram[i];

        // for each vertex in the polygon we want to draw a line from that vertex to the next vertex
        for (int j = 0; j < polygon.size; j++) {
            int next = (j == polygon.size - 1) ? 0 : (j + 1);

            Vertex vertex = NormaliseVertex(polygon.vertices[j]);
            Vertex next_vertex = NormaliseVertex(polygon.vertices[next]);

            s32 x0 = vertex.x;
            s32 x1 = next_vertex.x;
            s32 y0 = vertex.y;
            s32 y1 = next_vertex.y;

            if (y0 > y1) {
                std::swap(x0, x1);
                std::swap(y0, y1);
            }

            if (y0 == y1) {
                y1++;
            }
            
            for (int line = 0; line < 192; line++) {
                if ((line >= y0 && line < y1) || (line >= y1 && line < y0)) {
                    Slope slope;
                    slope.Setup(x0, y0, x1, y1);
                    s32 span_start = slope.SpanStart(line);
                    s32 span_end = slope.SpanEnd(line);

                    if (slope.Negative()) {
                        std::swap(span_start, span_end);
                    }

                    for (int x = span_start; x <= span_end; x++) {
                        if (x >= 0 && x < 256) {
                            framebuffer[(line * 256) + x] = 0xFFFFFFFF;
                        }
                    }
                }
            }
        }
    }
}

Vertex RenderEngine::NormaliseVertex(Vertex vertex) {
    Vertex render_vertex;

    if (vertex.w != 0) {
        render_vertex.x = (( vertex.x * 128) / vertex.w) + 128;
        render_vertex.y = ((-vertex.y * 96)  / vertex.w) + 96;
    }

    render_vertex.colour = vertex.colour;
    return render_vertex;
}