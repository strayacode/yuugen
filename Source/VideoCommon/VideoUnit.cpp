#include "Common/Log.h"
#include "Common/Memory.h"
#include "Common/Settings.h"
#include "VideoCommon/VideoUnit.h"
#include "Core/System.h"

VideoUnit::VideoUnit(System& system) : renderer_2d {SoftwareRenderer2D(*this, Engine::A), SoftwareRenderer2D(*this, Engine::B)}, renderer_3d(*this), system(system) {}

VideoUnit::~VideoUnit() {
    stop_render_thread();
}

void VideoUnit::reset() {
    powcnt1 = 0;
    vcount = 0;
    dispstat.fill(0);
    dispcapcnt = 0;
    
    palette_ram.fill(0);
    oam.fill(0);

    renderer_2d[0].reset();
    renderer_2d[1].reset();
    renderer_3d.reset();
    vram.reset();

    scanline_start_event = system.scheduler.RegisterEvent("Scanline Start", [this]() {
        render_scanline_start();
        system.scheduler.AddEvent(524, &scanline_end_event);
    });

    scanline_end_event = system.scheduler.RegisterEvent("Scanline End", [this]() {
        render_scanline_end();
        system.scheduler.AddEvent(1606, &scanline_start_event);
    });

    system.scheduler.AddEvent(1606, &scanline_start_event);

    render_thread_running = false;
    thread_state.store(ThreadState::Idle);
}

void VideoUnit::build_mmio(MMIO& mmio, Arch arch) {
    if (arch == Arch::ARMv5) {
        mmio.register_mmio<u16>(
            0x04000004,
            mmio.direct_read<u16>(&dispstat[1], 0xffbf),
            mmio.direct_write<u16>(&dispstat[1], 0xffbf)
        );

        int masks[9] = {0x9b, 0x9b, 0x9f, 0x9f, 0x87, 0x9f, 0x9f, 0x83, 0x83};

        for (int i = 0; i < 7; i++) {
            mmio.register_mmio<u8>(
                0x04000240 + i,
                mmio.direct_read<u8>(&vram.vramcnt[i], masks[i]),
                mmio.complex_write<u8>([&, this](u32, u8 data) {
                    system.video_unit.vram.update_vram_mapping(static_cast<VRAM::Bank>(i), data);
                })
            );
        }
        
        mmio.register_mmio<u32>(
            0x04000240,
            mmio.complex_read<u32>([this](u32) {
                return ((vram.vramcnt[3] << 24) | (vram.vramcnt[2] << 16) | (vram.vramcnt[1] << 8) | vram.vramcnt[0]);
            }),
            mmio.complex_write<u32>([&, this](u32, u8 data) {
                system.video_unit.vram.update_vram_mapping(VRAM::Bank::A, data);
                system.video_unit.vram.update_vram_mapping(VRAM::Bank::B, (data >> 8) & 0xFF);
                system.video_unit.vram.update_vram_mapping(VRAM::Bank::C, (data >> 16) & 0xFF);
                system.video_unit.vram.update_vram_mapping(VRAM::Bank::D, (data >> 24) & 0xFF);
            })
        );

        mmio.register_mmio<u16>(
            0x04000304,
            mmio.direct_read<u16>(&powcnt1, 0x820f),
            mmio.direct_write<u16>(&powcnt1, 0x820f)
        );

        mmio.register_mmio<u32>(
            0x04000304,
            mmio.invalid_read<u32>(),
            mmio.direct_write<u32, u16>(&powcnt1, 0x820f)
        );

        renderer_2d[0].build_mmio(mmio);
    }
}

const u32* VideoUnit::get_framebuffer(Screen screen) {
    if (((powcnt1 >> 15) & 0x1) == (screen == Screen::Top)) {
        return renderer_2d[0].get_framebuffer();
    } else {
        return renderer_2d[1].get_framebuffer();
    }
}

void VideoUnit::render_scanline_start() {
    if (vcount < 192) {
        if (render_thread_running) {
            // optimisation: get the main thread to render for engine b
            // while rendering is happening for engine a on the main thread
            if (thread_state.load() == ThreadState::DrawingA) {
                thread_state.store(ThreadState::DrawingB);
                renderer_2d[1].render_scanline(vcount);
            }

            // wait for previous scanline to be drawn by render thread
            while (thread_state.load() != ThreadState::Idle) {
                std::this_thread::yield();
            }
        } else {
            renderer_2d[0].render_scanline(vcount);
            renderer_2d[1].render_scanline(vcount);
        }

        // trigger an arm9 dma transfer on hblank (only for visible scanlines)
        system.dma[1].Trigger(2);
    }

    for (int i = 0; i < 2; i++) {
        dispstat[i] |= 1 << 1;

        if (dispstat[i] & (1 << 4)) {
            system.cpu_core[i].SendInterrupt(InterruptType::HBlank);
        }
    }

    if (vcount == 215) {
        renderer_3d.render();
    }

    // ARM9 DMA exclusive
    // check if scanline is between 2 and 193 inclusive
    // if so trigger a start of display dma transfer
    // TODO: on scanline 194 automatically clear the enable bit in DMA
    if ((vcount > 1) && (vcount < 194)) {
        system.dma[1].Trigger(3);
    }
}

void VideoUnit::render_scanline_end() {
    if (++vcount == 263) {
        // enable threaded 2d renderers
        if (Settings::Get().threaded_2d && !render_thread_running) {
            start_render_thread();
        }

        if (!Settings::Get().threaded_2d && render_thread_running) {
            stop_render_thread();
        }

        vcount = 0;
    }

    for (int i = 0; i < 2; i++) {
        dispstat[i] &= ~(1 << 1);

        switch (vcount) {
        case 192:
            // start of vblank
            dispstat[i] |= 1;

            if (dispstat[i] & (1 << 3)) {
                system.cpu_core[i].SendInterrupt(InterruptType::VBlank);
            }

            system.dma[i].Trigger(1);
            break;
        case 262:
            // end of vblank
            dispstat[i] &= ~1;
            break;
        }

        if (((dispstat[i] >> 8) | ((dispstat[i] & (1 << 7)) << 1)) == vcount) {
            dispstat[i] |= (1 << 2);

            if (dispstat[i] & (1 << 5)) {
                system.cpu_core[i].SendInterrupt(InterruptType::VCounter);
            }

        } else if (dispstat[i] & (1 << 2)) {
            dispstat[i] &= ~(1 << 2);
        }
    }

    if (render_thread_running && vcount < 192) {
        thread_state.store(ThreadState::DrawingA);
    }
}

void VideoUnit::start_render_thread() {
    stop_render_thread();

    render_thread_running = true;

    render_thread = std::thread{[this]() {
        while (render_thread_running) {
            while (thread_state.load() == ThreadState::Idle) {
                // early return incase stop_render_thread() is called
                if (!render_thread_running) return;
            }

            renderer_2d[0].render_scanline(vcount);
            
            // only do engine b rendering if the main thread isn't already doing that for us
            if (thread_state.exchange(ThreadState::DrawingB) == ThreadState::DrawingA) {
                renderer_2d[1].render_scanline(vcount);
            }
            
            thread_state.store(ThreadState::Idle);
        }
    }};
}

void VideoUnit::stop_render_thread() {
    if (!render_thread_running) {
        return;
    }

    render_thread_running = false;

    render_thread.join();
}