#pragma once

#include <common/vertex.h>
#include <common/types.h>

struct Polygon {
    Vertex* vertices = nullptr;
    int size = 0;
};