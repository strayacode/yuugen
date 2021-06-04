#pragma once

#include <common/vertex.h>
#include <common/types.h>

struct Polygon {
    Vertex* vertex_list = nullptr;
    u8 size = 0;
};