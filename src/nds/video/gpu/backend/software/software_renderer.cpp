#include <algorithm>
#include "common/logger.h"
#include "nds/video/gpu/backend/software/software_renderer.h"
#include "nds/video/vram_region.h"

namespace nds {

SoftwareRenderer::SoftwareRenderer(GPU::DISP3DCNT& disp3dcnt, VRAMRegion& texture_data, VRAMRegion& texture_palette) : disp3dcnt(disp3dcnt), texture_data(texture_data), texture_palette(texture_palette) {}

void SoftwareRenderer::reset() {
    framebuffer.fill(0);
    depth_buffer.fill(0xffffff);
}

void SoftwareRenderer::render() {
    // TODO: ideally we should render scanline by scanline
    // figure out how this works on real hardware
    framebuffer.fill(0);

    // depth values are 24 bits (0 - 0xffffff)
    depth_buffer.fill(0xffffff);

    for (int i = 0; i < 192; i++) {
        render_scanline(i);
    }
}

void SoftwareRenderer::submit_polygons(Polygon* polygons, int num_polygons, bool w_buffering) {
    this->polygons = polygons;
    this->num_polygons = num_polygons;
    this->w_buffering = w_buffering;
}

void SoftwareRenderer::render_scanline(int y) {
    for (int i = 0; i < num_polygons; i++) {
        render_polygon_scanline(polygons[i], y);
    }
}

void SoftwareRenderer::render_polygon_scanline(Polygon& polygon, int y) {
    int start = 0;
    int end = 0;

    // determine the index of the top left and bottom right vertex
    for (int i = 0; i < polygon.size; i++) {
        const auto* vertex = polygon.vertices[i];
        if (vertex->y < polygon.vertices[start]->y) {
            start = i;
        } else if ((vertex->y == polygon.vertices[start]->y) && (vertex->x < polygon.vertices[start]->x)) {
            start = i;
        }

        if (vertex->y > polygon.vertices[end]->y) {
            end = i;
        } else if ((vertex->y == polygon.vertices[end]->y) && (vertex->x > polygon.vertices[end]->x)) {
            end = i;
        }
    }

    int left = start;
    int right = start;
    int next_left = polygon.next(left);
    int next_right = polygon.prev(right);

    while (polygon.vertices[next_left]->y <= y && next_left != end) {
        left = next_left;
        next_left = polygon.next(left);
    }

    while (polygon.vertices[next_right]->y <= y && next_right != end) {
        right = next_right;
        next_right = polygon.prev(right);
    }

    const auto left_vertex = polygon.vertices[left];
    const auto next_left_vertex = polygon.vertices[next_left];
    const auto right_vertex = polygon.vertices[right];
    const auto next_right_vertex = polygon.vertices[next_right];

    Slope left_slope{*left_vertex, *next_left_vertex};
    Slope right_slope{*right_vertex, *next_right_vertex};

    Interpolator<9> slope_interpolator;
    Interpolator<8> span_interpolator;

    s32 span_start = left_slope.span_start(y);
    s32 span_end = right_slope.span_end(y);

    // calculate the current w value along each slope at y
    s32 w0 = slope_interpolator.interpolate(left_vertex->w, next_left_vertex->w, y, left_vertex->y, next_left_vertex->y, left_vertex->w, next_left_vertex->w);
    s32 w1 = slope_interpolator.interpolate(right_vertex->w, next_right_vertex->w, y, right_vertex->y, next_right_vertex->y, right_vertex->w, next_right_vertex->w);
    
    // calculate the current depth value along each slope at y
    u32 z0 = slope_interpolator.interpolate_linear(left_vertex->z, next_left_vertex->z, y, left_vertex->y, next_left_vertex->y);
    u32 z1 = slope_interpolator.interpolate_linear(right_vertex->z, next_right_vertex->z, y, right_vertex->y, next_right_vertex->y);

    // calculate the current colour value along each slope at y
    Colour c0 = slope_interpolator.interpolate_colour(left_vertex->colour, next_left_vertex->colour, y, left_vertex->y, next_left_vertex->y, 0, 0);
    Colour c1 = slope_interpolator.interpolate_colour(right_vertex->colour, next_right_vertex->colour, y, right_vertex->y, next_right_vertex->y, 0, 0);

    // calculate the current texture coords along each slope at y
    s16 s0 = slope_interpolator.interpolate(left_vertex->s, next_left_vertex->s, y, left_vertex->y, next_left_vertex->y, left_vertex->w, next_left_vertex->w);
    s16 s1 = slope_interpolator.interpolate(right_vertex->s, next_right_vertex->s, y, right_vertex->y, next_right_vertex->y, right_vertex->w, next_right_vertex->w);
    s16 t0 = slope_interpolator.interpolate(left_vertex->t, next_left_vertex->t, y, left_vertex->y, next_left_vertex->y, left_vertex->w, next_left_vertex->w);
    s16 t1 = slope_interpolator.interpolate(right_vertex->t, next_right_vertex->t, y, right_vertex->y, next_right_vertex->y, right_vertex->w, next_right_vertex->w);

    if (span_start > span_end) {
        std::swap(span_start, span_end);
        std::swap(w0, w1);
        std::swap(z0, z1);
        std::swap(c0, c1);
        std::swap(s0, s1);
        std::swap(t0, t1);
    }

    if ((y >= left_vertex->y && y < next_left_vertex->y) && (y >= right_vertex->y && y < next_right_vertex->y)) {
        for (int x = span_start; x <= span_end; x++) {
            if (x < 0 || x > 255) continue;

            u32 addr = (256 * y) + x;
            u32 depth = 0;

            if (w_buffering) {
                depth = span_interpolator.interpolate(w0, w1, x, span_start, span_end, w0, w1);
            } else {
                depth = span_interpolator.interpolate_linear(z0, z1, x, span_start, span_end);
            }

            if (!depth_test(depth_buffer[addr], depth, polygon.polygon_attributes.depth_test_equal)) {
                continue;
            }

            // calculate colour value for scanline at x
            Colour c = span_interpolator.interpolate_colour(c0, c1, x, span_start, span_end, w0, w1);

            // calculate texture coords for scanline at x
            s16 s = span_interpolator.interpolate(s0, s1, x, span_start, span_end, w0, w1);
            s16 t = span_interpolator.interpolate(t0, t1, x, span_start, span_end, w0, w1);

            if (disp3dcnt.texture_mapping) {
                framebuffer[addr] = decode_texture(s, t, polygon);
            } else {
                framebuffer[addr] = c.to_u16();
            }

            depth_buffer[addr] = depth;
        }
    }
}

bool SoftwareRenderer::depth_test(u32 old_depth, u32 depth, bool equal) {
    if (equal) {
        int margin = w_buffering ? 0xff : 0x200;
        return std::abs(static_cast<s32>(depth) - static_cast<s32>(old_depth)) <= margin;
    } else {
        return depth < old_depth;
    }
}

} // namespace