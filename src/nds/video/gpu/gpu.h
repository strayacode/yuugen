#pragma once

#include <array>
#include "common/types.h"
#include "common/ring_buffer.h"
#include "common/scheduler.h"

namespace nds {

class GPU {
public:
    GPU(common::Scheduler& scheduler);

    void reset();
    void write_disp3dcnt(u32 value, u32 mask);
    u32 read_gxstat();
    void write_gxstat(u32 value, u32 mask);
    void queue_command(u32 addr, u32 data);
    void render();

    std::array<u32, 256 * 192> framebuffer;

private:
    struct Entry {
        u8 command{0};
        u32 parameter{0};
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

    // geometry commands
    void set_matrix_mode();
    void pop_current_matrix();

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
            bool fifo_full : 1;
            bool fifo_less_than_half_full : 1;
            bool fifo_empty : 1;
            bool busy : 1;
            u32 : 2;
            InterruptType fifo_irq : 2;
        };

        u32 data;
    };

    DISP3DCNT disp3dcnt;
    GXSTAT gxstat;
    common::RingBuffer<Entry, 256> gxfifo;
    common::RingBuffer<Entry, 4> gxpipe;
    bool busy{false};
    common::Scheduler& scheduler;
    common::EventType geometry_command_event;
    MatrixMode matrix_mode{MatrixMode::Projection};
};

} // namespace nds