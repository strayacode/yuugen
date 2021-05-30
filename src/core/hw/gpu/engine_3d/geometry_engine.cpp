#include <core/hw/gpu/engine_3d/geometry_engine.h>
#include <core/hw/gpu/gpu.h>
#include <array>
#include <tuple>

static constexpr std::array<std::pair<int, int>, 38> parameter_count {{
    {0x00, 0},
    {0x10, 1},
    {0x12, 0},
    {0x13, 1},
    {0x14, 1},
    {0x15, 1},
    {0x16, 0},
    {0x17, 16},
    {0x18, 12},
    {0x19, 16},
    {0x1A, 12},
    {0x1B, 9},
    {0x1C, 3},
    {0x20, 3},
    {0x21, 1},
    {0x22, 1},
    {0x23, 1},
    {0x24, 2},
    {0x25, 1},
    {0x26, 1},
    {0x27, 1},
    {0x28, 1},
    {0x29, 1},
    {0x2A, 1},
    {0x2B, 1},
    {0x30, 1},
    {0x31, 1},
    {0x32, 1},
    {0x33, 1},
    {0x34, 32},
    {0x40, 1},
    {0x41, 0},
    {0x50, 1},
    {0x60, 1},
    {0x70, 3},
    {0x71, 2},
    {0x72, 1},
}};

GeometryEngine::GeometryEngine(GPU* gpu) : gpu(gpu) {

}

void GeometryEngine::Reset() {
    matrix_mode = 0;
    // create an empty queue and swap it
    std::queue<Entry> empty_queue;
    fifo.swap(empty_queue);
}

void GeometryEngine::QueueCommand(u32 addr, u32 data) {
    switch (addr) {
    case 0x04000440:
        AddEntry({0x10, data});
        break;
    default:
        log_fatal("[GeometryEngine] Undefined geometry command defined by address %08x with data %08x", addr, data);
    }
}

void GeometryEngine::AddEntry(Entry entry) {
    if (fifo.size() == 0 && pipe.size() < 4) {
        // add the entry to the pipe
        pipe.push(entry);
    } else {
        // add the entry to the fifo
        fifo.push(entry);
    }

    // TODO: process geometry commands
}

void GeometryEngine::InterpretCommand() {

}

void GeometryEngine::CommandMTX_MODE(u32 data) {
    // set the matrix mode
    matrix_mode = data & 0x3;
}