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
        RenderPolygon(polygon_ram[i]);
    }
}

void RenderEngine::RenderPolygon(Polygon& polygon) {
    int start = 0;
    int end = 0;

    for (int i = 0; i < polygon.size; i++) {
        // normalise each vertex and determine the indices of the top left and bottom right vertex
        polygon.vertices[i] = NormaliseVertex(polygon.vertices[i]);

        if (polygon.vertices[i].y < polygon.vertices[start].y) {
            start = i;
        } else if ((polygon.vertices[i].y == polygon.vertices[start].y) && (polygon.vertices[i].x < polygon.vertices[start].x)) {
            start = i;
        }

        if (polygon.vertices[i].y > polygon.vertices[end].y) {
            end = i;
        } else if ((polygon.vertices[i].y == polygon.vertices[end].y) && (polygon.vertices[i].x > polygon.vertices[end].x)) {
            end = i;
        }
    }

    // both the left and right slope initially start at the top left vertex
    // TODO: work out winding stuff later
    int left = start;
    int right = start;
    int new_left = polygon.Prev(left);
    int new_right = polygon.Next(right);

    if (polygon.vertices[left].y == polygon.vertices[new_left].y) {
        polygon.vertices[new_left].y++;
    }

    if (polygon.vertices[right].y == polygon.vertices[new_right].y) {
        polygon.vertices[new_right].y++;
    }

    Slope left_slope;
    left_slope.Setup(polygon.vertices[left], polygon.vertices[new_left]);
    Slope right_slope;
    right_slope.Setup(polygon.vertices[right], polygon.vertices[new_right]);

    for (int y = 0; y < 192; y++) {
        // if the current scanline gets to the end of one of the slopes then reconfigure that slope
        if (polygon.vertices[new_left].y <= y) {
            left = new_left;
            new_left = polygon.Prev(left);

            if (polygon.vertices[left].y == polygon.vertices[new_left].y) {
                polygon.vertices[new_left].y++;
            }

            left_slope.Setup(polygon.vertices[left], polygon.vertices[new_left]);
        }

        if (polygon.vertices[new_right].y <= y) {
            right = new_right;
            new_right = polygon.Next(right);

            if (polygon.vertices[right].y == polygon.vertices[new_right].y) {
                polygon.vertices[new_right].y++;
            }

            right_slope.Setup(polygon.vertices[right], polygon.vertices[new_right]);
        }

        s32 left_span_start = left_slope.SpanStart(y);
        s32 left_span_end = left_slope.SpanEnd(y);
        s32 right_span_start = right_slope.SpanStart(y);
        s32 right_span_end = right_slope.SpanEnd(y);

        if (left_slope.Negative()) {
            std::swap(left_span_start, left_span_end);
        }

        if (right_slope.Negative()) {
            std::swap(right_span_start, right_span_end);
        }

        if ((y >= polygon.vertices[left].y && y < polygon.vertices[new_left].y) || (y >= polygon.vertices[new_left].y && y < polygon.vertices[left].y)) {
            for (int x = left_span_start; x <= left_span_end; x++) {
                if (x >= 0 && x < 256) {
                    framebuffer[(y * 256) + x] = 0xFFFFFFFF;
                }
            }
        }

        if ((y >= polygon.vertices[right].y && y < polygon.vertices[new_right].y) || (y >= polygon.vertices[new_right].y && y < polygon.vertices[right].y)) {
            for (int x = right_span_start; x <= right_span_end; x++) {
                if (x >= 0 && x < 256) {
                    framebuffer[(y * 256) + x] = 0xFFFFFFFF;
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