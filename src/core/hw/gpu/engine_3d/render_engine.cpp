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
        // printf("vertex %d (%d, %d) has colour %08x\n", i, x, y, polygon.vertices[i].colour.to_u16());
        // determine the indices of the top left and bottom right vertex
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
    // TODO: clean up code
    int left = start;
    int right = start;
    int new_left = polygon.Prev(left);
    int new_right = polygon.Next(right);

    s32 left_x0 = polygon.vertices[left].x;
    s32 left_x1 = polygon.vertices[new_left].x;
    s32 left_y0 = polygon.vertices[left].y;
    s32 left_y1 = polygon.vertices[new_left].y;

    if (left_y0 > left_y1) {
        std::swap(left_x0, left_x1);
        std::swap(left_y0, left_y1);
    }

    if (left_y0 == left_y1) {
        left_y1++;
    }

    s32 right_x0 = polygon.vertices[right].x;
    s32 right_x1 = polygon.vertices[new_right].x;
    s32 right_y0 = polygon.vertices[right].y;
    s32 right_y1 = polygon.vertices[new_right].y;

    if (right_y0 > right_y1) {
        std::swap(right_x0, right_x1);
        std::swap(right_y0, right_y1);
    }

    if (right_y0 == right_y1) {
        right_y1++;
    }

    Slope left_slope;
    left_slope.Setup(left_x0, left_y0, left_x1, left_y1);
    Slope right_slope;
    right_slope.Setup(right_x0, right_y0, right_x1, right_y1);

    for (int y = 0; y < 192; y++) {
        // if the current scanline gets to the end of one of the slopes then reconfigure that slope
        if (polygon.vertices[new_left].y <= y) {
            left = new_left;
            new_left = polygon.Prev(left);

            left_x0 = polygon.vertices[left].x;
            left_x1 = polygon.vertices[new_left].x;
            left_y0 = polygon.vertices[left].y;
            left_y1 = polygon.vertices[new_left].y;

            if (left_y0 > left_y1) {
                std::swap(left_x0, left_x1);
                std::swap(left_y0, left_y1);
            }

            if (left_y0 == left_y1) {
                left_y1++;
            }

            left_slope.Setup(left_x0, left_y0, left_x1, left_y1);
        }

        if (polygon.vertices[new_right].y <= y) {
            right = new_right;
            new_right = polygon.Next(right);

            right_x0 = polygon.vertices[right].x;
            right_x1 = polygon.vertices[new_right].x;
            right_y0 = polygon.vertices[right].y;
            right_y1 = polygon.vertices[new_right].y;

            if (right_y0 > right_y1) {
                std::swap(right_x0, right_x1);
                std::swap(right_y0, right_y1);
            }

            if (right_y0 == right_y1) {
                right_y1++;
            }

            right_slope.Setup(right_x0, right_y0, right_x1, right_y1);
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

        if ((y >= left_y0 && y < left_y1) || (y >= left_y1 && y < left_y0)) {
            for (int x = left_span_start; x <= left_span_end; x++) {
                if (x >= 0 && x < 256) {
                    Colour colour = interpolate_colour(
                        polygon.vertices[left].colour,
                        polygon.vertices[new_left].colour,
                        y - left_y0,
                        left_y1 - left_y0
                    );

                    framebuffer[(y * 256) + x] = colour.to_u16();
                }
            }
        }

        if ((y >= right_y0 && y < right_y1) || (y >= right_y1 && y < right_y0)) {
            for (int x = right_span_start; x <= right_span_end; x++) {
                if (x >= 0 && x < 256) {
                    Colour colour = interpolate_colour(
                        polygon.vertices[right].colour,
                        polygon.vertices[new_right].colour,
                        y - right_y0,
                        right_y1 - right_y0
                    );

                    printf("c1 %08x c2 %08x interpolated %08x %d %d\n",
                        polygon.vertices[right].colour.to_u32(),
                        polygon.vertices[new_right].colour.to_u32(),
                        colour.to_u32(),
                        y - right_y0,
                        right_y1 - right_y0
                    );
                    
                    framebuffer[(y * 256) + x] = colour.to_u16();
                }
            }
        }
    }
}

Colour RenderEngine::interpolate_colour(Colour c1, Colour c2, u32 a, u32 p) {
    Colour c3;
    c3.r = lerp(c1.r, c2.r, a, p);
    c3.g = lerp(c1.g, c2.g, a, p);
    c3.b = lerp(c1.b, c2.b, a, p);

    return c3;
}

u32 RenderEngine::lerp(u32 u1, u32 u2, u32 a, u32 p) {
    return u1 + (u2 - u1) * a / p;
}