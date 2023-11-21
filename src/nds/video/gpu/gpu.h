#pragma once

#include <array>
#include <memory>
#include "common/types.h"
#include "common/ring_buffer.h"
#include "common/scheduler.h"
#include "nds/hardware/dma.h"
#include "nds/video/vram_region.h"
#include "nds/video/gpu/matrix_stack.h"
#include "nds/video/gpu/vertex.h"
#include "nds/video/gpu/polygon.h"
#include "nds/video/gpu/backend/renderer.h"
#include "nds/hardware/irq.h"

namespace nds {

class GPU {
public:
    GPU(common::Scheduler& scheduler, DMA& dma, IRQ& irq, VRAMRegion& texture_data, VRAMRegion& texture_palette);

    void reset();
    const u32* get_framebuffer() { return renderer->get_framebuffer(); };

    u32 read_disp3dcnt() const { return disp3dcnt.data; }
    void write_disp3dcnt(u32 value, u32 mask);
    void write_gxfifo(u32 value);

    u32 read_gxstat();
    u32 read_clip_matrix(u32 addr);
    void write_gxstat(u32 value, u32 mask);
    void write_clear_colour(u32 value, u32 mask);
    void write_clear_depth(u16 value, u32 mask);
    void write_clrimage_offset(u16 value, u32 mask);
    void write_fog_colour(u32 value, u32 mask);
    void write_fog_offset(u16 value, u32 mask);
    void queue_command(u32 addr, u32 data);
    void do_swap_buffers();
    void render();

    union DISP3DCNT {
        struct {
            bool texture_mapping : 1;
            bool polygon_shading : 1;
            bool alpha_test : 1;
            bool alpha_blending : 1;
            bool anti_aliasing : 1;
            bool edge_marking : 1;
            bool alpha_mode : 1;
            bool fog_enable : 1;
            u32 fog_depth_shift : 4;
            bool rdlines_underflow : 1;
            bool polygon_vertex_ram_overflow : 1;
            bool rear_plane_mode : 1;
            u32 : 17;
        };

        u32 data;
    };

    DISP3DCNT disp3dcnt;
    
private:
    struct Entry {
        u8 command{0};
        u32 parameter{0};
    };

    enum class PolygonType {
        Triangle = 0,
        Quad = 1,
        TriangleStrip = 2,
        QuadStrip = 3,
    };

    enum class MatrixMode {
        Projection = 0,
        Modelview = 1,
        Simultaneous = 2,
        Texture = 3,
    };

    void check_gxfifo_irq();
    void queue_entry(Entry entry);
    Entry dequeue_entry();
    void execute_command();

    Matrix multiply_matrix_matrix(const Matrix& a, const Matrix& b);
    Vertex multiply_vertex_matrix(const Vertex& a, const Matrix& b);

    void update_clip_matrix();
    void submit_vertex();
    void submit_polygon();
    Vertex normalise_vertex(const Vertex& vertex);
    bool cull(const Vertex& v0, const Vertex& v1, const Vertex& v2);

    // geometry commands
    void set_matrix_mode();
    void pop_current_matrix();
    void load_unit_matrix();
    void swap_buffers();
    void set_texture_parameters();
    void set_polygon_attributes();
    void set_viewport();
    void multiply_4x4();
    void multiply_4x3();
    void push_current_matrix();
    void multiply_translation();
    void multiply_3x3();
    void begin_vertex_list();
    void set_vertex_colour();
    void add_vertex16();
    void end_vertex_list();
    void set_shininess();
    void set_normal_vector();
    void set_texture_coordinates();
    void set_vertex_xy();
    void multiply_scale();
    void restore_current_matrix();
    void add_vertex10();
    void set_texture_palette_address();
    void set_diffuse_ambient_reflect();
    void set_specular_reflect_emission();
    void store_current_matrix();
    void set_light_colour();
    void load_4x4();
    void set_light_vector();
    void load_4x3();
    void set_vertex_xz();
    void set_vertex_yz();
    void set_relative_vertex_coordinates();

    enum InterruptType : u32 {
        Never = 0,
        LessThanHalfFull = 1,
        Empty = 2,
        Reserved = 3,
    };

    union GXSTAT {
        struct {
            bool test_busy : 1;
            bool boxtest_result : 1;
            u32 : 6;
            u32 position_vector_matrix_stack_level : 5;
            u32 projection_matrix_stack_level : 1;
            bool matrix_stack_busy : 1;
            bool matrix_stack_error : 1;
            u32 fifo_num_entries : 9;
            bool fifo_less_than_half_full : 1;
            bool fifo_empty : 1;
            bool busy : 1;
            u32 : 2;
            InterruptType fifo_irq : 2;
        };

        u32 data;
    };

    GXSTAT gxstat;
    u32 gxfifo{0};
    int gxfifo_write_count{0};
    common::RingBuffer<Entry, 256> fifo;
    common::RingBuffer<Entry, 4> pipe;
    bool busy{false};
    
    common::EventType geometry_command_event;
    MatrixMode matrix_mode{MatrixMode::Projection};

    MatrixStack<1> projection;
    MatrixStack<31> modelview;
    MatrixStack<31> direction;
    MatrixStack<1> texture;
    Matrix clip;

    std::array<std::array<Vertex, 6144>, 2> vertex_ram;
    int vertex_ram_size{0};
    std::array<std::array<Polygon, 2048>, 2> polygon_ram;
    int polygon_ram_size{0};

    // tracks which vertex/polygon ram is currently being used
    int current_buffer{0};

    bool swap_buffers_requested{false};
    bool w_buffering{false};

    u32 clear_colour{0};
    u16 clear_depth{0};
    u16 clrimage_offset{0};
    u32 fog_colour{0};
    u16 fog_offset{0};
    std::array<u8, 0x10> edge_colour;
    std::array<u8, 0x20> fog_table;
    std::array<u8, 0x40> toon_table;

    Vertex current_vertex;
    Polygon current_polygon;

    struct Viewport {
        u32 x0{0};
        u32 y0{0};
        u32 x1{0};
        u32 y1{0};
    };

    Viewport viewport;

    PolygonType polygon_type{PolygonType::Triangle};
    std::array<Vertex, 10> vertex_list;
    int vertex_count{0};
    int polygon_count{0};

    std::unique_ptr<Renderer> renderer;

    common::Scheduler& scheduler;
    DMA& dma;
    IRQ& irq;
    VRAMRegion& texture_data;
    VRAMRegion& texture_palette;
};

} // namespace nds