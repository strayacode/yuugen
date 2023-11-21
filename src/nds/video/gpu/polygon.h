#pragma once

#include "common/types.h"
#include "nds/video/gpu/vertex.h"

namespace nds {

class Polygon {
public:
    int next(int current) {
        if (current == size - 1) {
            return 0;
        } else {
            return current + 1;
        }
    }

    int prev(int current) {
        if (current == 0) {
            return size - 1;
        } else {
            return current - 1;
        }
    }

    std::array<Vertex*, 10> vertices;

    enum TextureFormat : u32 {
        None,
        A3I5,
        Colour4,
        Colour16,
        Colour256,
        Compressed,
        A5I3,
        Direct,
    };

    struct TextureAttributes {
        union TextureParameters {
            struct {
                u32 vram_offset : 16;
                bool repeat_in_s_direction : 1;
                bool repeat_in_t_direction : 1;
                bool flip_in_s_direction : 1;
                bool flip_in_t_direction : 1;
                u32 width : 3;
                u32 height : 3;
                TextureFormat texture_format : 3;
                bool colour0 : 1;
                u32 transformation_mode : 2;
            };

            u32 data;
        };

        TextureParameters parameters{};
        u32 palette_base{0};
    };

    enum PolygonMode : u32 {
        Modulation,
        Decal,
        Toon,
        Shadow,
    };

    union PolygonAttributes {
        struct {
            u32 light_config : 4;
            PolygonMode polygon_mode : 2;
            bool back_surface : 1;
            bool front_surface : 1;
            u32 : 3;
            bool translucent_depth : 1;
            bool clip_far_plane : 1;
            bool render_1dot : 1;
            bool depth_test_equal : 1;
            bool fog_enable : 1;
            u32 alpha : 5;
            u32 : 3;
            u32 id : 6;
            u32 : 2;
        };

        u32 data;
    };

    TextureAttributes texture_attributes;
    PolygonAttributes polygon_attributes;
    int size{0};
    bool clockwise{false};
};

} // namespace nds