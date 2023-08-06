#include <algorithm>
#include "common/logger.h"
#include "core/system.h"

namespace core {

System::System() :
    arm7(*this),
    arm9(*this),
    cartridge(*this),
    video_unit(*this),
    dma7(scheduler, arm7.get_memory(), arm7.get_irq(), arm::Arch::ARMv4),
    dma9(scheduler, arm9.get_memory(), arm9.get_irq(), arm::Arch::ARMv5),
    ipc(arm7.get_irq(), arm9.get_irq()),
    spi(*this),
    timers7(scheduler, arm7.get_irq()),
    timers9(scheduler, arm9.get_irq())
{
    arm7.select_backend(arm::Backend::Interpreter);
    arm9.select_backend(arm::Backend::Interpreter);
}

System::~System() {
    stop();
}

void System::reset() {
    scheduler.reset();
    arm7.reset();
    arm9.reset();
    cartridge.reset();
    cartridge.load(config.game_path);
    video_unit.reset();
    input.reset();
    spu.reset();
    dma7.reset();
    dma9.reset();
    ipc.reset();
    maths_unit.reset();
    rtc.reset();
    spi.reset();
    timers7.reset();
    timers9.reset();

    main_memory.fill(0);
    shared_wram.fill(0);
    wramcnt = 0;
    haltcnt = 0;
    exmemcnt = 0;

    frames = 0;
}

void System::start() {
    // stop the previous thread if one was running
    stop();
    reset();

    if (config.boot_mode == BootMode::Direct) {
        direct_boot();
    } else {
        firmware_boot();
    }

    thread_state = ThreadState::Running;
    state = State::Running;
    thread = std::thread{[this]() {
        run_thread();
    }};
}

void System::stop() {
    if (thread_state == ThreadState::Idle) {
        return;
    }

    thread_state = ThreadState::Idle;
    thread.join();
}

void System::set_game_path(const std::string& game_path) {
    config.game_path = game_path;
}

void System::set_boot_mode(BootMode boot_mode) {
    config.boot_mode = boot_mode;
}

void System::write_wramcnt(u8 value) {
    wramcnt = value & 0x3;
    arm7.get_memory().update_wram_mapping();
    arm9.get_memory().update_wram_mapping();
}

void System::write_haltcnt(u8 value) {
    haltcnt = value & 0xc0;
    switch ((haltcnt >> 6) & 0x3) {
    case 0x2:
        arm7.get_cpu().update_halted(true);
        break;
    case 0x3:
        logger.todo("System: handle sleep mode");
        break;
    default:
        logger.todo("System: unimplemented power down mode");
    }
}

void System::write_exmemcnt(u16 value, u32 mask) {
    exmemcnt = (exmemcnt & ~mask) | (value & mask);
}

void System::run_thread() {
    auto frame_end = std::chrono::system_clock::now() + Frame{1};
    auto fps_update = std::chrono::system_clock::now();
    while (thread_state == ThreadState::Running) {
        run_frame();
        frames++;

        if (std::chrono::system_clock::now() - fps_update >= std::chrono::milliseconds(FPS_UPDATE_INTERVAL)) {
            update_callback(frames * (1000.0f / FPS_UPDATE_INTERVAL));
            frames = 0;
            fps_update = std::chrono::system_clock::now();
        }

        if (framelimiter) {
            std::this_thread::sleep_until(frame_end);
            frame_end += Frame{1};
        } else {
            frame_end = std::chrono::system_clock::now() + Frame{1};
        }
    }
}

void System::run_frame() {
    auto frame_end = scheduler.get_current_time() + 560190;
    while (scheduler.get_current_time() < frame_end) {
        arm7.run(1);
        arm9.run(2);
        scheduler.tick(1);
        scheduler.run();
    }

    // auto frame_end = scheduler.get_current_time() + 560190;
    // while (scheduler.get_current_time() < frame_end) {
    //     u64 cycles = std::min(static_cast<u64>(16), scheduler.get_event_time() - scheduler.get_current_time());
    //     arm7.run(cycles);
    //     arm9.run(cycles * 2);
    //     scheduler.tick(cycles);
    //     scheduler.run();
    // }
}

void System::direct_boot() {
    write_wramcnt(0x03);

    cartridge.direct_boot();
    arm7.direct_boot();
    arm9.direct_boot();
    spi.direct_boot();
    logger.debug("System: direct booted successfully");
}

void System::firmware_boot() {
    logger.error("System: handle firmware boot");
}

} // namespace core