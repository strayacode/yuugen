#include "VideoCommon/Renderer2D.h"
#include "VideoCommon/GPU.h"

Renderer2D::Renderer2D(GPU& gpu, Engine engine) : gpu(gpu), engine(engine) {
    if (engine == Engine::A) {
        palette_ram = gpu.get_palette_ram();
        oam = gpu.get_oam();
        vram_addr = 0x06000000;
        obj_addr = 0x06400000;
    } else {
        palette_ram = gpu.get_palette_ram() + 0x400;
        oam = gpu.get_oam() + 0x400;
        vram_addr = 0x06200000;
        obj_addr = 0x06600000;
    }
}

void Renderer2D::reset() {
    dispcnt = 0;
    framebuffer.fill(0);
}

u8 Renderer2D::read_byte(u32 addr) {
    switch (addr) {
    default:
        log_fatal("handle %08x", addr);
    }

    return 0;
}

u16 Renderer2D::read_half(u32 addr) {
    switch (addr) {
    default:
        log_fatal("handle %08x", addr);
    }

    return 0;
}

u32 Renderer2D::read_word(u32 addr) {
    switch (addr) {
    default:
        log_fatal("handle %08x", addr);
    }

    return 0;
}

void Renderer2D::write_byte(u32 addr, u8 data) {
    switch (addr) {
    default:
        log_fatal("handle %08x", addr);
    }
}

void Renderer2D::write_half(u32 addr, u16 data) {
    switch (addr) {
    default:
        log_fatal("handle %08x", addr);
    }
}

void Renderer2D::write_word(u32 addr, u32 data) {
    switch (addr) {
    default:
        log_fatal("handle %08x", addr);
    }
}