#include "common/logger.h"
#include "common/bits.h"
#include "nds/video/gpu/gpu.h"

namespace nds {

static constexpr std::array<int, 256> parameter_table = {{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 1, 1, 1, 0, 16, 12, 16, 12, 9, 3, 3, 0, 0, 0,
    1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
}};

GPU::GPU(common::Scheduler& scheduler, DMA& dma) : scheduler(scheduler), dma(dma) {}

void GPU::reset() {
    framebuffer.fill(0);
    disp3dcnt.data = 0;
    gxstat.data = 0; 
    gxfifo.reset();
    gxpipe.reset();
    busy = false;

    geometry_command_event = scheduler.register_event("GeometryCommand", [this]() {
        busy = false;
        execute_command();
    });

    matrix_mode = MatrixMode::Projection;

    projection.reset();
    modelview.reset();
    direction.reset();
    texture.reset();
    clip.reset();

    for (int i = 0; i < 2; i++) {
        vertex_ram[i].fill(Vertex{});
        polygon_ram[i].fill(Polygon{});
    }

    vertex_ram_size = 0;
    polygon_ram_size = 0;
    current_buffer = 0;
    swap_buffers_requested = false;
    clear_colour = 0;
    clear_depth = 0;
    clrimage_offset = 0;

    viewport.x0 = 0;
    viewport.y0 = 0;
    viewport.x1 = 0;
    viewport.y1 = 0;

    polygon_type = PolygonType::Triangle;
    vertex_count = 0;
}

void GPU::write_disp3dcnt(u32 value, u32 mask) {
    disp3dcnt.data = (disp3dcnt.data & ~mask) | (value & mask);
}

u32 GPU::read_gxstat() {
    u32 value = 0;
    value |= gxfifo.get_size() << 16;

    if (gxfifo.is_full()) {
        value |= 1 << 24;
    }

    if (gxfifo.get_size() < 128) {
        value |= 1 << 25;
    }

    if (gxfifo.is_empty()) {
        value |= 1 << 26;
    }

    value |= gxstat.fifo_irq << 30;
    return value;
}

void GPU::write_gxstat(u32 value, u32 mask) {
    gxstat.data = (gxstat.data & ~mask) | (value & mask);
    check_gxfifo_irq();
}

void GPU::write_clear_colour(u32 value, u32 mask) {
    clear_colour = (clear_colour & ~mask) | (value & mask);
}

void GPU::write_clear_depth(u16 value, u32 mask) {
    clear_depth = (clear_depth & ~mask) | (value & mask);
}

void GPU::write_clrimage_offset(u16 value, u32 mask) {
    clrimage_offset = (clrimage_offset & ~mask) | (value & mask);
}

void GPU::queue_command(u32 addr, u32 data) {
    u8 command = common::get_field<2, 7>(addr);
    queue_entry({command, data});
}

void GPU::render() {
    // TODO: ideally we should render scanline by scanline
    // figure out how this works on real hardware
}

void GPU::check_gxfifo_irq() {
    if (gxstat.fifo_irq == InterruptType::LessThanHalfFull && gxfifo.get_size() < 128) {
        logger.todo("send less than half full gxfifo irq");
    } else if (gxstat.fifo_irq == InterruptType::Empty && gxfifo.is_empty()) {
        logger.todo("send empty gxfifo irq");
    }
}

void GPU::queue_entry(Entry entry) {
    if (gxfifo.is_empty() && !gxpipe.is_full()) {
        gxpipe.push(entry);
    } else {
        gxfifo.push(entry);

        if (gxfifo.is_full()) {
            logger.todo("gxfifo is full");
        }
    }

    execute_command();
}

GPU::Entry GPU::dequeue_entry() {
    auto entry = gxpipe.pop();

    // if the pipe is running half empty
    // then move 2 entries from the fifo to the pipe
    if (gxpipe.get_size() < 3) {
        if (!gxfifo.is_empty()) {
            gxpipe.push(gxfifo.pop());
        }

        if (!gxfifo.is_empty()) {
            gxpipe.push(gxfifo.pop());
        }

        check_gxfifo_irq();

        if (gxfifo.get_size() < 128) {
            dma.trigger(DMA::Timing::GXFIFO);
        }
    }

    return entry;
}

void GPU::execute_command() {
    auto total_size = gxfifo.get_size() + gxpipe.get_size();
    if (busy || (total_size == 0)) {
        return;
    }

    u8 command = gxpipe.get_front().command;
    u8 parameter_count = parameter_table[command];

    if (total_size >= parameter_count) {
        switch (command) {
        case 0x10:
            set_matrix_mode();
            break;
        case 0x11:
            push_current_matrix();
            break;
        case 0x12:
            pop_current_matrix();
            break;
        case 0x15:
            load_unit_matrix();
            break;
        case 0x18:
            multiply_4x4();
            break;
        case 0x19:
            multiply_4x3();
            break;
        case 0x1a:
            multiply_3x3();
            break;
        case 0x1c:
            multiply_translation();
            break;
        case 0x20:
            set_vertex_colour();
            break;
        case 0x23:
            add_vertex16();
            break;
        case 0x29:
            set_polygon_attributes();
            break;
        case 0x2a:
            set_texture_parameters();
            break;
        case 0x40:
            begin_vertex_list();
            break;
        case 0x50:
            swap_buffers();
            break;
        case 0x60:
            set_viewport();
            break;
        default:
            // if (parameter_count == 0) {
            //     dequeue_entry();
            // }

            // for (int i = 0; i < parameter_count; i++) {
            //     dequeue_entry();
            // }

            logger.todo("GPU: handle command %02x", command);
        }

        busy = true;
        scheduler.add_event(1, &geometry_command_event);
    }
}

Matrix GPU::multiply_matrix_matrix(const Matrix& a, const Matrix& b) {
    Matrix multiplied_matrix;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            s64 result = 0;
            for (int i = 0; i < 4; i++) {
                result += (static_cast<s64>(a.field[y][i]) * b.field[i][x]);
            }

            multiplied_matrix.field[y][x] = result >> 12;
        }
    }

    return multiplied_matrix;
}

Vertex GPU::multiply_vertex_matrix(const Vertex& a, const Matrix& b) {
    Vertex multiplied_vertex;
    multiplied_vertex.x = (static_cast<s64>(a.x) * b.field[0][0] + static_cast<s64>(a.y) * b.field[1][0] + static_cast<s64>(a.z) * b.field[2][0] + static_cast<s64>(a.w) * b.field[3][0]) >> 12;
    multiplied_vertex.y = (static_cast<s64>(a.x) * b.field[0][1] + static_cast<s64>(a.y) * b.field[1][1] + static_cast<s64>(a.z) * b.field[2][1] + static_cast<s64>(a.w) * b.field[3][1]) >> 12;
    multiplied_vertex.z = (static_cast<s64>(a.x) * b.field[0][2] + static_cast<s64>(a.y) * b.field[1][2] + static_cast<s64>(a.z) * b.field[2][2] + static_cast<s64>(a.w) * b.field[3][2]) >> 12;
    multiplied_vertex.w = (static_cast<s64>(a.x) * b.field[0][3] + static_cast<s64>(a.y) * b.field[1][3] + static_cast<s64>(a.z) * b.field[2][3] + static_cast<s64>(a.w) * b.field[3][3]) >> 12;
    return multiplied_vertex;
}

void GPU::update_clip_matrix() {
    clip = multiply_matrix_matrix(modelview.current, projection.current);
}

void GPU::add_vertex() {
    if (vertex_ram_size >= 6144) {
        logger.todo("GPU: handle when vertex ram is full");
    }

    auto& current_vertex_ram = vertex_ram[current_buffer];

    update_clip_matrix();

    current_vertex.w = 1 << 12;
    current_vertex = multiply_vertex_matrix(current_vertex, clip);

    switch (current_polygon.texture_attributes.parameters.transformation_mode) {
    case 1:
        logger.todo("GPU: handle texcoord source");
        break;
    case 2:
        logger.todo("GPU: handle normal source");
        break;
    case 3:
        logger.todo("GPU: handle vertex source");
        break;
    }

    current_vertex_ram[vertex_ram_size++] = current_vertex;
    vertex_count++;

    switch (polygon_type) {
    case PolygonType::Triangle:
        if (vertex_count % 3 == 0) {
            add_polygon();
        }
        
        break;
    case PolygonType::Quad:
        if (vertex_count % 4 == 0) {
            add_polygon();
        }

        break;
    case PolygonType::TriangleStrip:
        if (vertex_count >= 3) {
            add_polygon();
        }

        break;
    case PolygonType::QuadStrip:
        if (vertex_count >= 4 && vertex_count % 2 == 0) {
            add_polygon();
        }

        break;
    }
}

void GPU::add_polygon() {
    if (polygon_ram_size >= 2048) {
        logger.todo("GPU: handle when polygon ram is full");
    }

    // TODO: implement clipping

    int size = 3 + (static_cast<int>(polygon_type) & 0x1);
    logger.todo("handle polygon with %d vertices type %d", size, polygon_type);
    // current_polygon.size = size;
    // current_polygon.texture_attributes = texture_attributes;
    // current_polygon.polygon_attributes = polygon_attributes;

    // for (int i = 0; i < current_polygon.size; i++) {
    //     current_polygon.vertices[i] = &vertex_ram[vertex_ram_size - size + i];
    // }
    
    // // make sure quad strips are in an anticlockwise arrangement
    // if (polygon_type == PolygonType::QuadStrip) {
    //     std::swap(current_polygon.vertices[2], current_polygon.vertices[3]);
    // }

    // bool cull = cull_polygon(current_polygon);

    // if (cull) {
    //     switch (polygon_type) {
    //     case PolygonType::Triangle: case PolygonType::Quad:
    //         vertex_ram_size -= size;
    //         return;
    //     default:
    //         log_fatal("handle culling for polygon type %d", static_cast<int>(polygon_type));
    //     }
    // }

    // for (int i = 0; i < current_polygon.size; i++) {
    //     *current_polygon.vertices[i] = normalise_vertex(*current_polygon.vertices[i]);
    // }

    // polygon_ram[polygon_ram_size++] = current_polygon;
}

} // namespace nds