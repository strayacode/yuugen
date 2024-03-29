#include <algorithm>
#include "common/logger.h"
#include "common/bits.h"
#include "nds/video/gpu/gpu.h"
#include "nds/video/gpu/backend/software/software_renderer.h"

namespace nds {

static constexpr std::array<u8, 256> parameter_table = {{
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

const char* get_command_name(u8 command) {
    switch (command) {
    case 0x00:
        return "nop";
    case 0x10:
        return "mtx_mode";
    case 0x11:
        return "mtx_push";
    case 0x12:
        return "mtx_pop";
    case 0x13:
        return "mtx_store";
    case 0x14:
        return "mtx_restore";
    case 0x15:
        return "mtx_identity";
    case 0x16:
        return "mtx_load_4x4";
    case 0x17:
        return "mtx_load_4x3";
    case 0x18:
        return "mtx_mult_4x4";
    case 0x19:
        return "mtx_mult_4x3";
    case 0x1a:
        return "mtx_mult_3x3";
    case 0x1b:
        return "mtx_scale";
    case 0x1c:
        return "mtx_trans";
    case 0x20:
        return "color";
    case 0x21:
        return "normal";
    case 0x22:
        return "texcoord";
    case 0x23:
        return "vtx_16";
    case 0x24:
        return "vtx_10";
    case 0x25:
        return "vtx_xy";
    case 0x26:
        return "vtx_xz";
    case 0x27:
        return "vtx_yz";
    case 0x28:
        return "vtx_diff";
    case 0x29:
        return "polygon_attr";
    case 0x2a:
        return "teximage_param";
    case 0x2b:
        return "pltt_base";
    case 0x30:
        return "dif_amb";
    case 0x31:
        return "spe_emi";
    case 0x32:
        return "light_vector";
    case 0x33:
        return "light_color";
    case 0x34:
        return "shininess";
    case 0x40:
        return "begin_vtxs";
    case 0x41:
        return "end_vtxs";
    case 0x50:
        return "swap_buffers";
    case 0x60:
        return "viewport";
    case 0x70:
        return "box_test";
    case 0x71:
        return "pos_test";
    case 0x72:
        return "vec_test";
    default:
        return "<unknown>";
    }
}

GPU::GPU(common::Scheduler& scheduler, DMA& dma, IRQ& irq, VRAMRegion& texture_data, VRAMRegion& texture_palette) : scheduler(scheduler), dma(dma), irq(irq), texture_data(texture_data), texture_palette(texture_palette) {}

void GPU::reset() {
    disp3dcnt.data = 0;
    gxstat.data = 0;
    gxfifo = 0;
    gxfifo_write_count = 0;
    fifo.reset();
    pipe.reset();
    gxstat.busy = false;

    geometry_command_event = scheduler.register_event("GeometryCommand", [this]() {
        process_and_validate_command();
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
    w_buffering = false;
    clear_colour = 0;
    clear_depth = 0;
    clrimage_offset = 0;
    fog_colour = 0;
    fog_offset = 0;
    edge_colour.fill(0);
    fog_table.fill(0);
    toon_table.fill(0);
    alpha_test_ref = 0;

    viewport.x0 = 0;
    viewport.y0 = 0;
    viewport.x1 = 0;
    viewport.y1 = 0;

    polygon_type = PolygonType::Triangle;
    vertex_list.fill(Vertex{});
    vertex_count = 0;
    polygon_count = 0;

    renderer = std::make_unique<SoftwareRenderer>(disp3dcnt, texture_data, texture_palette);
}

void GPU::write_disp3dcnt(u32 value, u32 mask) {
    disp3dcnt.data = (disp3dcnt.data & ~mask) | (value & mask);
}

void GPU::write_gxfifo(u32 value) {
    if (gxfifo == 0) {
        gxfifo = value;
    } else {
        u8 command = gxfifo & 0xff;
        queue_entry({command, value});

        gxfifo_write_count++;

        if (gxfifo_write_count == parameter_table[command]) {
            gxfifo >>= 8;
            gxfifo_write_count = 0;
        }
    }

    while ((gxfifo != 0) && (parameter_table[gxfifo & 0xff] == 0)) {
        u8 command = gxfifo & 0xff;
        queue_entry({command, 0});
        gxfifo >>= 8;
    }
}

u32 GPU::read_gxstat() {
    u32 value = 0;
    value |= fifo.get_size() << 16;

    if (fifo.is_full()) {
        value |= 1 << 24;
    }

    if (fifo.get_size() < 128) {
        value |= 1 << 25;
    }

    if (fifo.is_empty()) {
        value |= 1 << 26;
    }

    value |= gxstat.fifo_irq << 30;
    return value;
}

u32 GPU::read_clip_matrix(u32 addr) {
    update_clip_matrix();

    int x = (addr - 0x04000640) % 4;
    int y = (addr - 0x04000640) / 4;
    return clip.field[y][x];
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

void GPU::write_fog_colour(u32 value, u32 mask) {
    fog_colour = (fog_colour & ~mask) | (value & mask);
}

void GPU::write_fog_offset(u16 value, u32 mask) {
    fog_offset = (fog_offset & ~mask) | (value & mask);
}

void GPU::write_edge_colour(u32 addr, u16 value) {
    const auto index = (addr >> 1) & 0x7;
    edge_colour[index] = value;
}

void GPU::write_fog_table(u32 addr, u32 value, u32 mask) {
    const auto index = (addr >> 2) & 0x7;
    fog_table[index] = (fog_table[index] & ~mask) | (value & mask);
}

void GPU::write_toon_table(u32 addr, u16 value) {
    const auto index = (addr >> 1) & 0x1f;
    toon_table[index] = value;
}

void GPU::write_alpha_test_ref(u8 value) {
    alpha_test_ref = value;
}

void GPU::queue_command(u32 addr, u32 data) {
    u8 command = common::get_field<2, 7>(addr);
    queue_entry({command, data});
}

void GPU::do_swap_buffers() {
    if (!swap_buffers_requested) {
        return;
    }

    // normalise all the vertices
    for (int i = 0; i < vertex_ram_size; i++) {
        vertex_ram[current_buffer][i] = normalise_vertex(vertex_ram[current_buffer][i]);
    }

    renderer->submit_polygons(polygon_ram[current_buffer].data(), polygon_ram_size, w_buffering);
    swap_buffers_requested = false;
    current_buffer ^= 1;
    vertex_ram_size = 0;
    polygon_ram_size = 0;

    // Since the geometry engine was halted until this point,
    // resume command processing.
    process_and_validate_command();
}

void GPU::render() {
    renderer->render();
}

void GPU::check_gxfifo_irq() {
    if (gxstat.fifo_irq == InterruptType::LessThanHalfFull && fifo.get_size() < 128) {
        irq.raise(IRQ::Source::GXFIFO);
    } else if (gxstat.fifo_irq == InterruptType::Empty && fifo.is_empty()) {
        irq.raise(IRQ::Source::GXFIFO);
    }
}

void GPU::queue_entry(Entry entry) {
    if (fifo.is_empty() && !pipe.is_full()) {
        pipe.push(entry);
    } else {
        while (fifo.is_full()) {
            // Run commands until the fifo isn't full.
            gxstat.busy = false;
            scheduler.cancel_event(&geometry_command_event);
            process_command();
        }

        fifo.push(entry);
        dma.set_gxfifo_half_empty(fifo.get_size() < 128);
    }

    if (!gxstat.busy) {
        process_and_validate_command();
    }
}

GPU::Entry GPU::dequeue_entry() {
    auto entry = pipe.pop();

    // if the pipe is running half empty
    // then move 2 entries from the fifo to the pipe
    if (pipe.get_size() < 3) {
        if (!fifo.is_empty()) {
            pipe.push(fifo.pop());
        }

        if (!fifo.is_empty()) {
            pipe.push(fifo.pop());
        }

        check_gxfifo_irq();

        if (fifo.get_size() < 128) {
            dma.set_gxfifo_half_empty(true);
            dma.trigger(DMA::Timing::GXFIFO);
        } else {
            dma.set_gxfifo_half_empty(false);
        }
    }

    return entry;
}

void GPU::process_and_validate_command() {
    if (pipe.is_empty() || swap_buffers_requested) {
        gxstat.busy = false;
        return;
    }

    process_command();
}

void GPU::process_command() {
    const auto total_size = fifo.get_size() + pipe.get_size();
    const auto command = pipe.get_front().command;
    const auto parameter_count = parameter_table[command];

    if (total_size < parameter_count) {
        gxstat.busy = false;
        return;
    }

    execute_command(command, parameter_count);

    if (!swap_buffers_requested) {
        gxstat.busy = true;
        scheduler.add_event(1, &geometry_command_event);
    }
}

void GPU::execute_command(u8 command, u8 parameter_count) {
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
    case 0x13:
        store_current_matrix();
        break;
    case 0x14:
        restore_current_matrix();
        break;
    case 0x15:
        load_unit_matrix();
        break;
    case 0x16:
        load_4x4();
        break;
    case 0x17:
        load_4x3();
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
    case 0x1b:
        multiply_scale();
        break;
    case 0x1c:
        multiply_translation();
        break;
    case 0x20:
        set_vertex_colour();
        break;
    case 0x21:
        set_normal_vector();
        break;
    case 0x22:
        set_texture_coordinates();
        break;
    case 0x23:
        add_vertex16();
        break;
    case 0x24:
        add_vertex10();
        break;
    case 0x25:
        set_vertex_xy();
        break;
    case 0x26:
        set_vertex_xz();
        break;
    case 0x27:
        set_vertex_yz();
        break;
    case 0x28:
        set_relative_vertex_coordinates();
        break;
    case 0x29:
        set_polygon_attributes();
        break;
    case 0x2a:
        set_texture_parameters();
        break;
    case 0x2b:
        set_texture_palette_address();
        break;
    case 0x30:
        set_diffuse_ambient_reflect();
        break;
    case 0x31:
        set_specular_reflect_emission();
        break;
    case 0x32:
        set_light_vector();
        break;
    case 0x33:
        set_light_colour();
        break;
    case 0x34:
        set_shininess();
        break;
    case 0x40:
        begin_vertex_list();
        break;
    case 0x41:
        end_vertex_list();
        break;
    case 0x50:
        swap_buffers();
        break;
    case 0x60:
        set_viewport();
        break;
    default:
        dequeue_entry();

        for (int i = 1; i < parameter_count; i++) {
            dequeue_entry();
        }

        LOG_WARN("handle command %02x with parameter count %d", command, parameter_count);
        break;
    }
}

Matrix GPU::multiply_matrix_matrix(const Matrix& a, const Matrix& b) {
    Matrix multiplied_matrix;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            s64 result = 0;
            for (int i = 0; i < 4; i++) {
                result += (static_cast<s64>(a.field[y][i]) * static_cast<s64>(b.field[i][x]));
            }

            multiplied_matrix.field[y][x] = result >> 12;
        }
    }

    return multiplied_matrix;
}

Vertex GPU::multiply_vertex_matrix(const Vertex& a, const Matrix& b) {
    Vertex multiplied_vertex = a;
    multiplied_vertex.x = (static_cast<s64>(a.x) * b.field[0][0] + static_cast<s64>(a.y) * b.field[1][0] + static_cast<s64>(a.z) * b.field[2][0] + static_cast<s64>(a.w) * b.field[3][0]) >> 12;
    multiplied_vertex.y = (static_cast<s64>(a.x) * b.field[0][1] + static_cast<s64>(a.y) * b.field[1][1] + static_cast<s64>(a.z) * b.field[2][1] + static_cast<s64>(a.w) * b.field[3][1]) >> 12;
    multiplied_vertex.z = (static_cast<s64>(a.x) * b.field[0][2] + static_cast<s64>(a.y) * b.field[1][2] + static_cast<s64>(a.z) * b.field[2][2] + static_cast<s64>(a.w) * b.field[3][2]) >> 12;
    multiplied_vertex.w = (static_cast<s64>(a.x) * b.field[0][3] + static_cast<s64>(a.y) * b.field[1][3] + static_cast<s64>(a.z) * b.field[2][3] + static_cast<s64>(a.w) * b.field[3][3]) >> 12;
    return multiplied_vertex;
}

void GPU::update_clip_matrix() {
    clip = multiply_matrix_matrix(modelview.current, projection.current);
}

void GPU::submit_vertex() {
    if (vertex_ram_size >= 6144) {
        disp3dcnt.polygon_vertex_ram_overflow = true;
        return;
    }

    update_clip_matrix();
    current_vertex.w = 1 << 12;

    auto clipped_vertex = multiply_vertex_matrix(current_vertex, clip);
    auto& vertex = vertex_list[vertex_count++];
    vertex = clipped_vertex;

    // TODO: here we should transform texture coordinates if vertex source is used
    if (current_polygon.texture_attributes.parameters.transformation_mode == 3) {
        LOG_TODO("GPU: use vertex source");
    }

    switch (polygon_type) {
    case PolygonType::Triangle:
        if (vertex_count == 3) {
            submit_polygon();
        }
        
        break;
    case PolygonType::Quad:
        if (vertex_count == 4) {
            submit_polygon();
        }

        break;
    case PolygonType::TriangleStrip:
        if (polygon_count % 2 == 1) {
            std::swap(vertex_list[0], vertex_list[1]);
            submit_polygon();
            vertex_list[1] = vertex_list[2];
        } else if (vertex_count == 3) {
            submit_polygon();
            vertex_list[0] = vertex_list[1];
            vertex_list[1] = vertex_list[2];
        }
        
        break;
    case PolygonType::QuadStrip:
        if (vertex_count == 4) {
            std::swap(vertex_list[2], vertex_list[3]);
            submit_polygon();
            vertex_list[0] = vertex_list[3];
            vertex_list[1] = vertex_list[2];
        }
        
        break;
    }
}

void GPU::submit_polygon() {
    polygon_count++;

    const bool is_strip = polygon_type == PolygonType::TriangleStrip || polygon_type == PolygonType::QuadStrip;
    if (is_strip) {
        vertex_count = 2;
    } else {
        vertex_count = 0;
    }

    if (polygon_ram_size >= 2048) {
        disp3dcnt.polygon_vertex_ram_overflow = true;
        return;
    }

    if (cull(vertex_list[0], vertex_list[1], vertex_list[2])) {
        return;
    }

    auto& vertex_ram = this->vertex_ram[current_buffer];
    auto& polygon_ram = this->polygon_ram[current_buffer];

    // TODO: implement clipping
    int size = 3 + (static_cast<int>(polygon_type) & 0x1);

    // by this point the polygon will have been clipped and culled,
    // so we can add the vertices of the vertex list to vertex ram
    for (int i = 0; i < size; i++) {
        if (vertex_ram_size >= 6144) {
            disp3dcnt.polygon_vertex_ram_overflow = true;
            return;
        }

        vertex_ram[vertex_ram_size++] = vertex_list[i];
    }

    // now construct the polygon
    // TODO: figure out what to set the clockwise field to
    Polygon polygon;
    polygon.texture_attributes = current_polygon.texture_attributes;
    polygon.polygon_attributes = current_polygon.polygon_attributes;

    // TODO: eventually size should account for potential vertices that were added from clipping
    polygon.size = size;

    polygon.vertices.fill(nullptr);

    // make the polygon vertices point to the vertices we just added to vertex ram
    for (int i = 0; i < polygon.size; i++) {
        polygon.vertices[i] = &vertex_ram[vertex_ram_size - size + i];
    }

    // submit the polygon to polygon ram
    polygon_ram[polygon_ram_size++] = polygon;
}

Vertex GPU::normalise_vertex(const Vertex& vertex) {
    Vertex normalised = vertex;

    if (vertex.w == 0) {
        normalised.x = 0;
        normalised.y = 0;
        normalised.z = vertex.z;
        normalised.w = vertex.w;
    } else {
        const s64 w = vertex.w;
        const s64 width = (viewport.x1 - viewport.x0 + 1) & 0x1ff;
        const s64 height = (viewport.y1 - viewport.y0 + 1) & 0xff;
        const s64 w_doubled = w << 1;
        
        normalised.x = (((static_cast<s64>(vertex.x) + w) * width) / w_doubled) + static_cast<s64>(viewport.x0);
        normalised.y = (((-static_cast<s64>(vertex.y) + w) * height) / w_doubled) + static_cast<s64>(viewport.y0);
        normalised.z = (((vertex.z << 14) / vertex.w) + 0x3fff) << 9;

        // TODO: should w be normalised?
    }

    return normalised;
}

bool GPU::cull(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
    // take cross product of (v0 - v1) x (v2 - v1)
    s64 x0 = v0.x - v1.x;
    s64 x1 = v2.x - v1.x;
    s64 y0 = v0.y - v1.y;
    s64 y1 = v2.y - v1.y;
    s64 w0 = v0.w - v1.w;
    s64 w1 = v2.w - v1.w;

    s64 xcross = y0 * w1 - w0 * y1;
    s64 ycross = w0 * x1 - x0 * w1;
    s64 wcross = x0 * y1 - y0 * x1;

    // reduce cross product results to 32-bit to avoid overflow
    while (xcross != static_cast<s32>(xcross) || ycross != static_cast<s32>(ycross) || wcross != static_cast<s32>(wcross)) {
        xcross >>= 4;
        ycross >>= 4;
        wcross >>= 4;
    }

    // calculate dot product of cross . v0
    s64 dot = xcross * v0.x + ycross * v0.y + wcross * v0.w;

    // if dot is negative, then it's back facing
    // if dot is positive, then it's front facing
    bool front_facing = dot < 0;
    bool render_back = current_polygon.polygon_attributes.back_surface;
    bool render_front = current_polygon.polygon_attributes.front_surface;
    return (!render_back && !front_facing) || (!render_front && front_facing);
}

} // namespace nds