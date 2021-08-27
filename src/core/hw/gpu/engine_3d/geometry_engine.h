#pragma once

#include <common/types.h>
#include <common/matrix.h>
#include <common/vertex.h>
#include <common/polygon.h>
#include <common/log.h>
#include <functional>
#include <queue>
#include <array>

struct Entry {
    u8 command = 0;
    u32 parameter = 0;
};

// notes:
// the clip matrix is internally recalculated
// each time the projection or position matrices are
// changed

class GPU;

class GeometryEngine {
public:
    GeometryEngine(GPU* gpu);
    void Reset();
    auto ReadGXSTAT() -> u32;
    void WriteGXSTAT(u32 data);
    void QueueCommand(u32 addr, u32 data);
    void QueueEntry(Entry entry);
    auto DequeueEntry() -> Entry;
    void InterpretCommand();
    void CheckGXFIFOInterrupt();

    // geometry commands
    // matrix operations
    void SetMatrixMode();
    void PopCurrentMatrix();

    u32 gxstat;
    u8 matrix_mode;
    bool busy;
    std::queue<Entry> fifo;
    std::queue<Entry> pipe;

    GPU* gpu;

    std::function<void()> InterpretCommandEvent;
};