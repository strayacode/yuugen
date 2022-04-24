#include "Core/hw/gpu/engine_3d/render_engine.h"
#include "Core/hw/gpu/engine_3d/Interpolator.h"
#include "Core/hw/gpu/gpu.h"

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
    polygon_ram_size = 0;
}

void RenderEngine::Render() {
    for (int i = 0; i < 256 * 192; i++) {
        framebuffer[i] = 0;
        depth_buffer[i] = 0;
    }

    for (int i = 0; i < polygon_ram_size; i++) {
        RenderPolygon(polygon_ram[i]);
    }
}

void RenderEngine::RenderPolygon(Polygon& polygon) {
    int start = 0;
    int end = 0;
    Interpolator<9> slope_interpolator;
    Interpolator<8> scanline_interpolator;

    for (int i = 0; i < polygon.size; i++) {
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

    // TODO: clean up code
    int left = start;
    int right = start;
    int new_left = polygon.Next(left);
    int new_right = polygon.Prev(right);

    // the colour for each slope which gets interpolated for each scanline
    Colour c[2];

    // w values that get interpolated along slopes
    s32 w[2];

    // texture coordinates that get interpolated along slopes
    s16 s[2];
    s16 t[2];

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
        if (polygon.vertices[new_left].y <= y && new_left != end) {
            left = new_left;
            new_left = polygon.Next(left);

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

        if (polygon.vertices[new_right].y <= y && new_right != end) {
            right = new_right;
            new_right = polygon.Prev(right);

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

        // TODO: should we use x coordinates for x major slopes?
        // TODO: make this cleaner
        w[0] = slope_interpolator.interpolate(
            polygon.vertices[left].w,
            polygon.vertices[new_left].w,
            y,
            left_y0,
            left_y1,
            polygon.vertices[left].w,
            polygon.vertices[new_left].w
        );

        w[1] = slope_interpolator.interpolate(
            polygon.vertices[right].w,
            polygon.vertices[new_right].w,
            y,
            right_y0,
            right_y1,
            polygon.vertices[right].w,
            polygon.vertices[new_right].w
        );

        c[0] = slope_interpolator.interpolate_colour(
            polygon.vertices[left].colour,
            polygon.vertices[new_left].colour,
            y,
            left_y0,
            left_y1,
            0,
            0
        );

        c[1] = slope_interpolator.interpolate_colour(
            polygon.vertices[right].colour,
            polygon.vertices[new_right].colour,
            y,
            right_y0,
            right_y1,
            0,
            0
        );

        s[0] = slope_interpolator.interpolate(
            polygon.vertices[left].s,
            polygon.vertices[new_left].s,
            y,
            left_y0,
            left_y1,
            polygon.vertices[left].w,
            polygon.vertices[new_left].w
        );

        s[1] = slope_interpolator.interpolate(
            polygon.vertices[right].s,
            polygon.vertices[new_right].s,
            y,
            right_y0,
            right_y1,
            polygon.vertices[right].w,
            polygon.vertices[new_right].w
        );

        t[0] = slope_interpolator.interpolate(
            polygon.vertices[left].t,
            polygon.vertices[new_left].t,
            y,
            left_y0,
            left_y1,
            polygon.vertices[left].w,
            polygon.vertices[new_left].w
        );

        t[1] = slope_interpolator.interpolate(
            polygon.vertices[right].t,
            polygon.vertices[new_right].t,
            y,
            right_y0,
            right_y1,
            polygon.vertices[right].w,
            polygon.vertices[new_right].w
        );

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

        if (left_span_start > right_span_end) {
            std::swap(left_span_start, right_span_end);
            std::swap(c[0], c[1]);
            std::swap(s[0], s[1]);
            std::swap(t[0], t[1]);
            // std::swap(w[0], w[1]);
        }

        if ((y >= left_y0 && y < left_y1) && (y >= right_y0 && y < right_y1)) {
            for (int x = left_span_start; x <= right_span_end; x++) {
                if (x < 0 || x > 255) continue;

                Colour colour = scanline_interpolator.interpolate_colour(
                    c[0],
                    c[1],
                    x,
                    left_span_start,
                    right_span_end,
                    w[0],
                    w[1]
                );

                s16 texcoord[2];

                texcoord[0] = scanline_interpolator.interpolate(
                    s[0],
                    s[1],
                    x,
                    left_span_start,
                    right_span_end,
                    w[0],
                    w[1]
                );

                texcoord[1] = scanline_interpolator.interpolate(
                    t[0],
                    t[1],
                    x,
                    left_span_start,
                    right_span_end,
                    w[0],
                    w[1]
                );

                if (disp3dcnt & 0x1) {
                    framebuffer[(y * 256) + x] = sample_texture(texcoord[0] >> 4, texcoord[1] >> 4, polygon.texture_attributes).to_u16();
                } else {
                    framebuffer[(y * 256) + x] = colour.to_u16();
                }
            }
        }
    }
}