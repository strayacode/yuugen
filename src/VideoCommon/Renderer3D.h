#pragma once

#include <array>
#include <functional>
#include <queue>
#include <array>
#include <memory>
#include <vector>
#include "Common/Types.h"
#include "Common/GPUTypes.h"
#include "Core/scheduler/scheduler.h"

enum class GeometryEngineState {
    Halted,
    Running,
};

enum class MatrixMode {
    Projection = 0,
    Modelview = 1,
    Simultaneous = 2,
    Texture = 3,
};

enum class PolygonType {
    Triangle = 0,
    Quad = 1,
    TriangleStrip = 2,
    QuadStrip = 3,
};

struct Entry {
    u8 command = 0;
    u32 parameter = 0;
};

class GPU;

class Renderer3D {
public:
    Renderer3D(GPU& gpu);

    void reset();
    virtual void render() = 0;
    const u32* get_framebuffer() { return framebuffer.data(); }

    u8 read_byte(u32 addr);
    u16 read_half(u32 addr);
    u32 read_word(u32 addr);

    void write_byte(u32 addr, u8 data);
    void write_half(u32 addr, u16 data);
    void write_word(u32 addr, u32 data);

    // mmio
    u16 disp3dcnt = 0;
    u16 clear_depth = 0;
    u32 clear_colour = 0;

private:
    void queue_entry(u32 addr, u32 data);
    Entry dequeue_entry();
    void run_command();
    u32 read_gxstat();
    void check_gxfifo_interrupt();
    void add_vertex();
    void add_polygon();
    Vertex NormaliseVertex(Vertex vertex);
    Matrix MultiplyMatrixMatrix(const Matrix& a, const Matrix& b);
    Vertex MultiplyVertexMatrix(const Vertex& a, const Matrix& b);
    void update_clip_matrix();
    void do_swap_buffers();

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

    std::array<u32, 256 * 192> framebuffer;

    u32 gxstat;
    u32 gxfifo;
    int gxfifo_write_count;
    MatrixMode matrix_mode;
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
    int vertex_count;

    Polygon polygon_ram[2048];
    int polygon_ram_size;

    Vertex current_vertex;
    Polygon current_polygon;

    u32 screen_x1;
    u32 screen_x2;
    u32 screen_y1;
    u32 screen_y2;

    PolygonType polygon_type;

    GPU& gpu;

    EventType geometry_command_event;

    u16 disp_1dot_depth;

    // texture attributes per polygon
    TextureAttributes texture_attributes;
};