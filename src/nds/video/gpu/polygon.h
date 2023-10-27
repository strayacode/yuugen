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

    struct TextureAttributes {
        union TextureParameters {
            struct {
                u32 vram_offset : 16;
                bool repeat_in_s_direction : 1;
                bool repeat_in_t_direction : 1;
                bool flip_in_s_direction : 1;
                bool flip_in_t_direction : 1;
                u32 s_size : 3;
                u32 t_size : 3;
                u32 texture_format : 3;
                bool colour0 : 1;
                u32 transformation_mode : 2;
            };

            u32 data;
        };

        TextureParameters parameters{};
        u32 palette_base{0};
    };

    TextureAttributes texture_attributes;
    u32 polygon_attributes{0};
    int size{0};
    bool clockwise{false};
};

} // namespace nds