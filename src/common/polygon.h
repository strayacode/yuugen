#pragma once

#include <common/vertex.h>
#include <common/types.h>

struct Polygon {
    Vertex* vertex_list = nullptr;
    int size = 0;
};