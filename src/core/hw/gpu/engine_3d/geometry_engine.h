#pragma once

#include <common/types.h>
#include <common/matrix.h>
#include <common/vertex.h>
#include <common/polygon.h>
#include <common/matrix_stack.h>
#include <common/log.h>
#include <common/circular_buffer.h>
#include <common/log_file.h>
#include <functional>
#include <queue>
#include <array>
#include <memory>

struct Entry {
    u8 command = 0;
    u32 parameter = 0;
};

// notes:
// the clip matrix is internally recalculated
// each time the projection or position matrices are
// changed
// 3 matrix modes:
// projection mode (for the projection matrix stack with 1 entry)
// modelview mode (for the modelview matrix stack with 31 entries)
// modelview and direction mode (also direction matrix stack with 31 entries)
// texture mode (texture matrix stack with 0 or 1 entries?)

// optimisation:
// only update clip matrix when we need to read it

enum class GeometryEngineState {
    Halted,
    Running,
};

class GPU;

class GeometryEngine {
public:
    GeometryEngine(GPU* gpu);
    void Reset();
    auto ReadGXSTAT() -> u32;
    void WriteGXSTAT(u32 data);
    void WriteGXFIFO(u32 data);
    void QueueCommand(u32 addr, u32 data);
    void QueueEntry(Entry entry);
    auto DequeueEntry() -> Entry;
    void InterpretCommand();
    void CheckGXFIFOInterrupt();
    void UpdateClipMatrix();
    auto MultiplyMatrixMatrix(const Matrix& a, const Matrix& b) -> Matrix;
    auto MultiplyVertexMatrix(const Vertex& a, const Matrix& b) -> Vertex;
    void PrintMatrix(const Matrix& a);
    void PrintVertex(const Vertex& a);
    void DoSwapBuffers();
    void AddVertex();
    u32 ReadClipMatrix(u32 addr);
    u32 ReadVectorMatrix(u32 addr);

    // geometry commands
    // matrix operations
    void SetMatrixMode();
    void PushCurrentMatrix();
    void StoreCurrentMatrix();
    void PopCurrentMatrix();
    void RestoreCurrentMatrix();
    void LoadUnitMatrix();
    void Load4x4();
    void Load4x3();
    void SwapBuffers();
    void Multiply4x4();
    void Multiply4x3();
    void Multiply3x3();
    void MultiplyTranslation();
    void MultiplyScale();

    // vertex / polygon / texture operations
    void SetTextureParameters();
    void SetPolygonAttributes();
    void BeginVertexList();
    void EndVertexList();
    void SetVertexColour();
    void AddVertex16();
    void AddVertex10();
    void SetTexturePaletteAddress();
    void SetTextureCoordinates();
    void SetRelativeVertexCoordinates();
    void SetVertexXY();
    void SetVertexXZ();
    void SetVertexYZ();
    
    // other
    void SetViewport();
    void SetShininess();
    void SetLightVector();
    void SetDiffuseAmbientReflect();
    void SetSpecularReflectEmission();
    void SetLightColour();
    void SetNormalVector();
    void BoxTest();

    u32 gxstat;
    u32 gxfifo;
    int gxfifo_write_count;
    u8 matrix_mode;
    bool busy;
    std::queue<Entry> fifo;
    std::queue<Entry> pipe;

    MatrixStack<1> projection;
    MatrixStack<31> modelview;
    MatrixStack<31> direction;
    MatrixStack<1> texture;
    Matrix clip;

    GeometryEngineState state;

    Vertex vertex_ram[6144];
    int vertex_ram_size;

    Vertex current_vertex;

    u32 screen_x1;
    u32 screen_x2;
    u32 screen_y1;
    u32 screen_y2;

    GPU* gpu;

    std::unique_ptr<LogFile> log_file;
};