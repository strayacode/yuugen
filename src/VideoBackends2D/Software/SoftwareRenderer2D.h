#pragma once

#include <memory>
#include "VideoCommon/Renderer2D.h"

class GPU;

class SoftwareRenderer2D : public Renderer2D {
public:
    SoftwareRenderer2D(GPU& gpu, Engine engine);
    void render_scanline(int line) override;

private:
    void render_blank_screen(int line);
    void render_graphics_display(int line);
    void render_vram_display(int line);
    void render_text(int bg, int line);
    void render_extended(int bg, int line);
    void render_affine(int bg, int line);
    void render_objects(int line);

    void compose_scanline(int line);
    void compose_pixel(int x, int line);

    inline void draw_pixel(int x, int y, u32 colour) {
        framebuffer[(256 * y) + x] = colour;
    }

    u16 decode_obj_pixel_4bpp(u32 base, int number, int x, int y);
    u16 decode_obj_pixel_8bpp(u32 base, int number, int x, int y);

    static constexpr int dimensions[3][4][2] = {{{8, 8}, {16, 16}, {32, 32}, {64, 64}}, {{16, 8}, {32, 8}, {32, 16}, {64, 32}}, {{8, 16}, {8, 32}, {16, 32}, {32, 64}}};
};