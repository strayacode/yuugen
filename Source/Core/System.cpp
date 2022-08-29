#include <algorithm>
#include "Common/Settings.h"
#include "Core/System.h"

System::System(UpdateFunction update_fps) 
    :video_unit(*this), cartridge(*this),
    spi(*this),
    dma {DMA(*this, 0), DMA(*this, 1)}, ipc(*this), 
    timers {Timers(*this, 0), Timers(*this, 1)},
    spu(*this),
    arm7(*this), arm9(*this),
    m_emu_thread([this]() {
        if (m_state == State::Running) {
            run_frame();
        }
    }, update_fps) {
    arm7.select_backend(CPUBackend::Interpreter);
    arm9.select_backend(CPUBackend::Interpreter);

    arm7.memory().build_mmio();
    arm9.memory().build_mmio();

    m_audio_interface = std::make_shared<SDLAudioInterface>();
    spu.set_audio_interface(m_audio_interface);
}

void System::reset() {
    arm7.memory().reset();
    arm9.memory().reset();
    scheduler.reset();
    cartridge.reset();
    cartridge.load(m_game_path);
    video_unit.reset();
    dma[0].reset();
    dma[1].reset();
    timers[0].reset();
    timers[1].reset();
    arm9.coprocessor().reset();
    spi.reset();
    input.reset();
    ipc.reset();
    maths_unit.reset();
    spu.reset();
    rtc.reset();

    main_memory.fill(0);
    shared_wram.fill(0);
    
    wramcnt = 0;
    powcnt2 = 0;
    rcnt = 0;
    HALTCNT = 0;
    exmemcnt = 0;
    postflg7 = 0;
    postflg9 = 0;
    biosprot = 0;
    siocnt = 0;

    arm7.cpu().reset();
    arm9.cpu().reset();
}

void System::start() {
    reset();

    if (m_boot_mode == BootMode::Firmware) {
        firmware_boot();
    } else {
        direct_boot();
    }
}

void System::shutdown() {
    m_emu_thread.Stop();
}

void System::direct_boot() {
    arm9.coprocessor().direct_boot();

    // TODO: make a direct_boot function for ARM7Memory and ARM9Memory
    arm7.memory().write<u16>(0x04000134, 0x8000); // rcnt
    arm9.memory().write<u8>(0x04000247, 0x03); // wramcnt
    arm9.memory().write<u8>(0x04000300, 0x01); // POSTFLG (ARM9)
    arm7.memory().write<u8>(0x04000300, 0x01); // POSTFLG (ARM7)
    arm9.memory().write<u16>(0x04000304, 0x0001); // POWCNT1
    arm7.memory().write<u16>(0x04000504, 0x0200); // SOUNDBIAS
    arm9.memory().write<u32>(0x027FF800, 0x00001FC2); // Chip ID 1
    arm9.memory().write<u32>(0x027FF804, 0x00001FC2); // Chip ID 2
    arm9.memory().write<u16>(0x027FF850, 0x5835); // ARM7 BIOS CRC
    arm9.memory().write<u16>(0x027FF880, 0x0007); // Message from ARM9 to ARM7
    arm9.memory().write<u16>(0x027FF884, 0x0006); // ARM7 boot task
    arm9.memory().write<u32>(0x027FFC00, 0x00001FC2); // Copy of chip ID 1
    arm9.memory().write<u32>(0x027FFC04, 0x00001FC2); // Copy of chip ID 2
    arm9.memory().write<u16>(0x027FFC10, 0x5835); // Copy of ARM7 BIOS CRC
    arm9.memory().write<u16>(0x027FFC40, 0x0001); // Boot indicator

    cartridge.DirectBoot();
    arm7.cpu().direct_boot(cartridge.loader.GetARM7Entrypoint());
    arm9.cpu().direct_boot(cartridge.loader.GetARM9Entrypoint());    
    spi.DirectBoot();
}

void System::firmware_boot() {
    arm7.cpu().firmware_boot();
    arm9.cpu().firmware_boot();
    cartridge.FirmwareBoot();
}

void System::run_frame() {
    u64 frame_end_time = scheduler.GetCurrentTime() + 560190;

    while (scheduler.GetCurrentTime() < frame_end_time) {
        if (!arm7.cpu().is_halted() || !arm9.cpu().is_halted()) {
            u64 cycles = std::min(static_cast<u64>(16), scheduler.GetEventTime() - scheduler.GetCurrentTime());
            u64 arm7_target = scheduler.GetCurrentTime() + cycles;
            u64 arm9_target = scheduler.GetCurrentTime() + (cycles << 1);
            
            // run the arm9 until the next scheduled event
            if (!arm9.cpu().run(arm9_target)) {
                set_state(State::Paused);
                return;
            }

            // let the arm7 catch up
            if (!arm7.cpu().run(arm7_target)) {
                set_state(State::Paused);
                return;
            }

            // advance the scheduler
            scheduler.Tick(cycles);
        } else {
            // if both cpus are halted we can just advance to the next event
            scheduler.set_current_time(scheduler.GetEventTime());
        }

        scheduler.RunEvents();
    }
}

void System::set_state(State state) {
    State old_state = m_state;
    m_state = state;

    switch (state) {
    case State::Running:
        if (old_state == State::Idle) {
            start();
            m_emu_thread.Start();
        }

        m_audio_interface->SetState(AudioState::Playing);

        if (Settings::Get().threaded_2d) {
            video_unit.start_render_thread();
        }

        break;
    case State::Paused:
        m_audio_interface->SetState(AudioState::Paused);

        if (Settings::Get().threaded_2d) {
            video_unit.stop_render_thread();
        }

        break;
    case State::Idle:
        m_audio_interface->SetState(AudioState::Idle);
        m_emu_thread.Stop();

        if (Settings::Get().threaded_2d) {
            video_unit.stop_render_thread();
        }

        break;
    }
}

void System::set_boot_mode(BootMode boot_mode) {
    m_boot_mode = boot_mode;
}

void System::boot(std::string game_path) {
    set_state(State::Idle);
    m_game_path = game_path;
    set_state(State::Running);
}

void System::toggle_framelimiter() {
    if (m_state == State::Running) {
        set_state(State::Paused);
        m_emu_thread.toggle_framelimiter();
        set_state(State::Running);
    } else {
        m_emu_thread.toggle_framelimiter();
    }
}

void System::write_haltcnt(u8 data) {
    HALTCNT = data & 0xC0;

    u8 power_down_mode = (HALTCNT >> 6) & 0x3;

    // check bits 6..7 to see what to do
    switch (power_down_mode) {
    case 2:
        arm7.cpu().halt();
        break;
    case 3:
        log_warn("unhandled request for sleep mode");
        break;
    default:
        log_fatal("power down mode %d is not implemented!", power_down_mode);
        break;
    }
}

void System::write_wramcnt(u8 data) {
    wramcnt = data & 0x3;

    // now we must update the memory map for the shared wram space specifically
    arm7.memory().update_memory_map(0x03000000, 0x04000000);
    arm9.memory().update_memory_map(0x03000000, 0x04000000);
}

bool System::cartridge_access_rights() {
    // check which cpu has access to the nds cartridge
    if (exmemcnt & (1 << 11)) {
        return false; // 0 = ARMv4
    } else {
        return true; // 1 = ARMv5
    }
}
