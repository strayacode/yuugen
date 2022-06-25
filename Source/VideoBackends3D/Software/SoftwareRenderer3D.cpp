#include "VideoCommon/VideoUnit.h"
#include "VideoBackends3D/Software/SoftwareRenderer3D.h"

SoftwareRenderer3D::SoftwareRenderer3D(VideoUnit& video_unit) : Renderer3D(video_unit) {}

void SoftwareRenderer3D::render() {
    framebuffer.fill(0);

    // depth values are 24 bits (0 - 0xFFFFFF)
    depth_buffer.fill(0xFFFFFF);

    for (int i = 0; i < 192; i++) {
        render_scanline(i);
    }
}

void SoftwareRenderer3D::render_scanline(int y) {
    for (int i = 0; i < renderer_num_polygons; i++) {
        render_polygon_scanline(renderer_polygon_ram[i], y);
    }
}

void SoftwareRenderer3D::render_polygon_scanline(Polygon& polygon, int y) {
    int start = 0;
    int end = 0;

    // determine the indices of the top left and bottom right vertex
    for (int i = 0; i < polygon.size; i++) {
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

    int left = start;
    int right = start;
    int next_left = polygon.Next(left);
    int next_right = polygon.Prev(right);

    while (polygon.vertices[next_left].y <= y && next_left != end) {
        left = next_left;
        next_left = polygon.Next(left);
    }

    while (polygon.vertices[next_right].y <= y && next_right != end) {
        right = next_right;
        next_right = polygon.Prev(right);
    }

    s32 left_x0 = polygon.vertices[left].x;
    s32 left_x1 = polygon.vertices[next_left].x;
    s32 left_y0 = polygon.vertices[left].y;
    s32 left_y1 = polygon.vertices[next_left].y;

    if (left_y0 == left_y1) {
        left_y1++;
    }

    s32 right_x0 = polygon.vertices[right].x;
    s32 right_x1 = polygon.vertices[next_right].x;
    s32 right_y0 = polygon.vertices[right].y;
    s32 right_y1 = polygon.vertices[next_right].y;

    if (right_y0 == right_y1) {
        right_y1++;
    }

    left_slope.Setup(left_x0, left_y0, left_x1, left_y1);
    right_slope.Setup(right_x0, right_y0, right_x1, right_y1);

    s32 span_start = left_slope.SpanStart(y);
    s32 span_end = right_slope.SpanEnd(y);

    // calculate the current w value along each slope at y
    s32 w0 = slope_interpolator.interpolate(polygon.vertices[left].w, polygon.vertices[next_left].w, y, left_y0, left_y1, polygon.vertices[left].w, polygon.vertices[next_left].w);
    s32 w1 = slope_interpolator.interpolate(polygon.vertices[right].w, polygon.vertices[next_right].w, y, right_y0, right_y1, polygon.vertices[right].w, polygon.vertices[next_right].w);
    
    // calculate the current depth value along each slope at y
    u32 z0 = slope_interpolator.interpolate_linear(polygon.vertices[left].z, polygon.vertices[next_left].z, y, left_y0, left_y1);
    u32 z1 = slope_interpolator.interpolate_linear(polygon.vertices[right].z, polygon.vertices[next_right].z, y, right_y0, right_y1);

    // calculate the current colour value along each slope at y
    Colour c0 = slope_interpolator.interpolate_colour(polygon.vertices[left].colour, polygon.vertices[next_left].colour, y, left_y0, left_y1, 0, 0);
    Colour c1 = slope_interpolator.interpolate_colour(polygon.vertices[right].colour, polygon.vertices[next_right].colour, y, right_y0, right_y1, 0, 0);

    // calculate the current texture coords along each slope at y
    s16 s0 = slope_interpolator.interpolate(polygon.vertices[left].s, polygon.vertices[next_left].s, y, left_y0, left_y1, polygon.vertices[left].w, polygon.vertices[next_left].w);
    s16 s1 = slope_interpolator.interpolate(polygon.vertices[right].s, polygon.vertices[next_right].s, y, right_y0, right_y1, polygon.vertices[right].w, polygon.vertices[next_right].w);
    s16 t0 = slope_interpolator.interpolate(polygon.vertices[left].t, polygon.vertices[next_left].t, y, left_y0, left_y1, polygon.vertices[left].w, polygon.vertices[next_left].w);
    s16 t1 = slope_interpolator.interpolate(polygon.vertices[right].t, polygon.vertices[next_right].t, y, right_y0, right_y1, polygon.vertices[right].w, polygon.vertices[next_right].w);

    if (span_start > span_end) {
        std::swap(span_start, span_end);
        std::swap(w0, w1);
        std::swap(z0, z1);
        std::swap(c0, c1);
        std::swap(s0, s1);
        std::swap(t0, t1);
    }

    if ((y >= left_y0 && y < left_y1) && (y >= right_y0 && y < right_y1)) {
        for (int x = span_start; x <= span_end; x++) {
            if (x < 0 || x > 255) continue;

            // do depth test
            u32 depth = 0;

            if (w_buffering) {
                depth = scanline_interpolator.interpolate(w0, w1, x, span_start, span_end, w0, w1);
            } else {
                depth = scanline_interpolator.interpolate_linear(z0, z1, x, span_start, span_end);
            }

            if (!depth_test(depth_buffer[(256 * y) + x], depth, (polygon.polygon_attributes >> 14) & 0x1)) continue;

            // calculate colour value for scanline at x
            Colour c = scanline_interpolator.interpolate_colour(c0, c1, x, span_start, span_end, w0, w1);

            // calculate texture coords for scanline at x
            s16 s = scanline_interpolator.interpolate(s0, s1, x, span_start, span_end, w0, w1);
            s16 t = scanline_interpolator.interpolate(t0, t1, x, span_start, span_end, w0, w1);

            if (disp3dcnt & 0x1) {
                // log_fatal("handle texture");
                framebuffer[(y * 256) + x] = decode_texture(s, t, polygon.texture_attributes).to_u16();
            } else {
                framebuffer[(y * 256) + x] = c.to_u16();
            }

            depth_buffer[(y * 256) + x] = depth;
        }
    }
}

bool SoftwareRenderer3D::depth_test(u32 old_depth, u32 depth, bool equal) {
    if (equal) {
        int margin = w_buffering ? 0xFF : 0x200;
        return std::abs(static_cast<s32>(depth) - static_cast<s32>(old_depth)) <= margin;
    } else {
        return depth < old_depth;
    }
}