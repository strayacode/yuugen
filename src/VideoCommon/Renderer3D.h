#pragma once

#include <array>
#include "Common/Types.h"

class Renderer3D {
public:
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

private:
    std::array<u32, 256 * 192> framebuffer;
};