#include <core/hw/gpu/engine_3d/geometry_engine.h>

void GeometryEngine::SetMatrixMode() {
    u32 parameter = DequeueEntry().parameter;
    matrix_mode = parameter & 0x3;
}

void GeometryEngine::PopCurrentMatrix() {
    u32 parameter = DequeueEntry().parameter;
    u8 stack_offset = (s8)((parameter & 0x3F) << 2) >> 2;

    switch (matrix_mode) {
        
    }
}