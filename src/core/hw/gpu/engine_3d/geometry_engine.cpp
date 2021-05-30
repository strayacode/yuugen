#include <core/hw/gpu/engine_3d/geometry_engine.h>
#include <core/hw/gpu/gpu.h>

GeometryEngine::GeometryEngine(GPU* gpu) : gpu(gpu) {

}

void GeometryEngine::Reset() {
    matrix_mode = 0;
    // create an empty queue and swap it
    std::queue<Entry> empty_queue;
    gxfifo.swap(empty_queue);
}

void GeometryEngine::InterpretCommand(u32 addr, u32 data) {
    switch (addr) {
    case 0x04000440:
        CommandMTX_MODE(data);
        break;
    default:
        log_fatal("[GeometryEngine] Undefined geometry command defined by address %08x with data %08x", addr, data);
    }
}

void GeometryEngine::CommandMTX_MODE(u32 data) {
    // set the matrix mode
    matrix_mode = data & 0x3;
}