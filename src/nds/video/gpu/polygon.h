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
        u32 parameters{0};
        u32 palette_base{0};
    };

    TextureAttributes texture_attributes;
    u32 polygon_attributes{0};
    int size{0};
    bool clockwise{false};
};

} // namespace nds