#pragma once

#include <common/vertex.h>
#include <common/types.h>

class Polygon {
public:
    Polygon() {
        vertices = nullptr;
        size = 0;
    }

    // returns the index of the next vertex
    int Next(int current) {
        if (current == size - 1) {
            return 0;
        } else {
            return current + 1;
        }
    }

    // returns the index of the previous vertex
    int Prev(int current) {
        if (current == 0) {
            return size - 1;
        } else {
            return current - 1;
        }
    }

    Vertex* vertices = nullptr;
    int size = 0;
};