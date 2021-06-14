#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/matrix.h>
#include <common/polygon.h>
#include <common/vertex.h>
#include <queue>
#include <functional>
#include <vector>

struct GPU;

struct Entry {
    u8 command;
    u32 parameter;
};

enum CurrentState : u8 {
    STATE_HALTED = 0,
    STATE_RUNNING = 1,
};

// notes:
// packed commands allows 4 commands per gxfifo write
// unpacked commands only allows 1 command per gxfifo write
// after a command is sent we can read in a 32 bit parameter

// the pipe will be used when the gxfifo is empty

// queueing commands into the fifo:
// we can also write to addresses 0x04000440...0x040005FF
// to queue geometry commands into the fifo
// each write will pass a parameter for a command
// e.g. N writes for N parameters and 1 write for no parameters
// to add the entry to the fifo


// the clip matrix is internally readjusted when recalculating position and projection matrices
// when this happens, we have clip_matrix = current position * current projection

// also in fixed point notation, 1.0 is equivalent to 1 << 12

struct GeometryEngine {
    GeometryEngine(GPU* gpu);
    void Reset();
    void QueueCommand(u32 addr, u32 data);
    void InterpretCommand();
    void QueueEntry(Entry entry);
    auto DequeueEntry() -> Entry;

    void CommandSetMatrixMode();
    void CommandPopCurrentMatrix();
    void CommandPushCurrentMatrix();
    void CommandLoadUnitMatrix();
    void CommandSwapBuffers();
    void CommandSetTextureParameters();
    void CommandSetPolygonAttributes();
    void CommandSetViewport();
    void CommandMultiply4x4();
    void CommandMultiply4x3();
    void CommandMultiply3x3();
    void CommandLoad4x4();
    void CommandLoad4x3();
    void CommandMultiplyTranslation();
    void CommandMultiplyScale();
    void CommandBeginVertexList();
    void CommandSetVertexColour();
    void CommandAddVertex16();
    void CommandAddVertex10();
    void CommandShininess();
    void CommandSetTexturePaletteAddress();
    void CommandSetLightDirectionVector();
    void CommandSetDiffuseAmbientReflect();
    void CommandSetSpecularReflectEmission();
    void CommandSetLightColour();
    void CommandStoreCurrentMatrix();
    void CommandRestoreCurrentMatrix();
    void CommandSetTextureCoordinates();
    void CommandSetVertexXY();
    void CommandSetVertexXZ();
    void CommandSetVertexYZ();
    void CommandSetVertexRelative();
    void CommandSetVertexNormal();

    void DoSwapBuffers();

    void CheckGXFIFOInterrupt();

    auto MultiplyMatrixMatrix(const Matrix& a, const Matrix& b) -> Matrix;
    auto MultiplyVertexMatrix(const Vertex& a, const Matrix& b) -> Vertex;
    auto MultiplyVertexVertex(const Vertex& a, const Vertex& b) -> u32;

    void PrintMatrix(const Matrix& a);

    void AddVertex();
    void AddPolygon();

    void WriteGXFIFO(u32 data);

    void UpdateClipMatrix();

    auto ReadGXSTAT() -> u32;
    void WriteGXSTAT();

    std::queue<Entry> fifo;
    std::queue<Entry> pipe;

    u32 gxfifo;
    int gxfifo_write_count;

    u32 GXSTAT;
    u32 RAM_COUNT;
    u16 DISP_1DOT_DEPTH;
    u8 POS_RESULT[0x10];
    u8 VEC_RESULT[0x06];
    u8 CLIPMTX_RESULT[0x40];
    u8 VECMTX_RESULT[0x24];

    u8 matrix_mode;

    u8 polygon_type;

    // TODO: sort this out correctly later
    u16 vertex_colour;

    u32 vertex_count;

    u8 state;

    Matrix projection_current;
    Matrix projection_stack;
    Matrix position_current;
    Matrix position_stack[31];
    Matrix directional_current;
    Matrix directional_stack[31];
    // not sure about this but will look into later
    Matrix texture_current;
    Matrix texture_stack;

    Matrix clip_current;

    u8 projection_pointer;
    u8 position_pointer;

    std::vector<Polygon> polygon_ram;
    std::vector<Vertex> vertex_ram;

    Vertex recent_vertex;
    Polygon recent_polygon;

    GPU* gpu;

    std::function<void()> InterpretCommandTask = std::bind(&GeometryEngine::InterpretCommand, this);
};