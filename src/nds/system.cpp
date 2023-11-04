#include <algorithm>
#include "common/logger.h"
#include "nds/system.h"

namespace nds {

System::System() :
    arm7(*this),
    arm9(*this),
    cartridge(*this),
    video_unit(*this),
    spu(scheduler, arm7.get_memory()),
    dma7(scheduler, arm7.get_memory(), arm7.get_irq(), arm::Arch::ARMv4),
    dma9(scheduler, arm9.get_memory(), arm9.get_irq(), arm::Arch::ARMv5),
    ipc(arm7.get_irq(), arm9.get_irq()),
    spi(*this),
    timers7(scheduler, arm7.get_irq()),
    timers9(scheduler, arm9.get_irq())
{
    main_memory = std::make_unique<std::array<u8, 0x400000>>();
    select_cpu_backend(arm::BackendType::Interpreter, false);
}

System::~System() {
    stop();
}

void System::reset() {
    scheduler.reset();
    arm7.reset();
    arm9.reset();
    cartridge.reset();

    if (config.game_path != "") {
        cartridge.load(config.game_path);
    }
    
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
    wifi.reset();

    main_memory->fill(0);
    shared_wram.fill(0);
    wramcnt = 0;
    haltcnt = 0;
    exmemcnt = 0;
    exmemstat = 0;
    rcnt = 0;
    
    if (config.boot_mode == common::BootMode::Fast) {
        direct_boot();
    } else {
        firmware_boot();
    }
}

void System::run_frame() {
    // auto frame_end = scheduler.get_current_time() + 560190;
    // while (scheduler.get_current_time() < frame_end) {
    //     // initially assume that both cpus are halted, meaning we can safely skip until the next
    //     // scheduler event
    //     auto cycles = scheduler.get_event_time() - scheduler.get_current_time();

    //     // if either cpu is not halted then we must enforce a max timeslice of 32 cycles (later on
    //     // this will be increased when instructions aren't 1 cpi anymore)
    //     if (!arm7.is_halted() || !arm9.is_halted()) {
    //         cycles = std::min(static_cast<u64>(32), cycles);
    //     }
        
    //     arm9.run(2 * cycles);
    //     arm7.run(cycles);
    //     scheduler.tick(cycles);
    //     scheduler.run();
    // }

    auto frame_end = scheduler.get_current_time() + 560190;
    while (scheduler.get_current_time() < frame_end) {
        auto cycles = scheduler.get_event_time() - scheduler.get_current_time();

        if (!arm7.is_halted() || !arm9.is_halted()) {
            cycles = std::min(static_cast<u64>(16), cycles);
        }

        arm9.run(2 * cycles);
        arm7.run(cycles);
        scheduler.tick(cycles);
        scheduler.run();
    }

    // TODO: move this to VideoUnit when hblank or end of frame occurs
    video_unit.ppu_a.on_finish_frame();
    video_unit.ppu_b.on_finish_frame();
}

void System::set_audio_device(std::shared_ptr<common::AudioDevice> audio_device) {
    this->audio_device = audio_device;
    spu.set_audio_device(audio_device);
}

std::vector<u32*> System::fetch_framebuffers() {
    return {video_unit.fetch_framebuffer(Screen::Top), video_unit.fetch_framebuffer(Screen::Bottom)};
}

void System::select_cpu_backend(arm::BackendType backend_type, bool optimise) {
    arm7.select_backend(backend_type, optimise);
    arm9.select_backend(backend_type, optimise);
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
        logger.warn("System: handle sleep mode");
        break;
    default:
        logger.todo("System: unimplemented power down mode");
    }
}

void System::write_exmemcnt(u16 value, u32 mask) {
    exmemcnt = (exmemcnt & ~mask) | (value & mask);
}

void System::write_exmemstat(u16 value, u32 mask) {
    exmemstat = (exmemstat & ~mask) | (value & mask);
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
    cartridge.firmware_boot();
    arm7.firmware_boot();
    arm9.firmware_boot();
}

} // namespace nds