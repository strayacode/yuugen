#include "common/logger.h"
#include "core/nds/system.h"

namespace core::nds {

System::System(Config config) : arm7(*this), arm9(*this), cartridge(*this) {
    arm7.select_backend(arm::Backend::Interpreter);
    arm9.select_backend(arm::Backend::Interpreter);
    reset();

    cartridge.load(config.game_path);

    if (config.boot_mode == BootMode::Direct) {
        direct_boot();
    }
}

void System::run_frame() {
    auto frame_end = scheduler.get_current_time() + 560190;
    while (scheduler.get_current_time() < frame_end) {
        arm7.run(1);
        arm9.run(2);
        scheduler.tick(1);
        scheduler.run_events();
    }
}

void System::reset() {
    scheduler.reset();
    arm7.reset();
    arm9.reset();
    video_unit.reset();
    input.reset();
    spu.reset();

    main_memory.fill(0);
    shared_wram.fill(0);
    wramcnt = 0;
}

void System::direct_boot() {
    cartridge.direct_boot();
    arm7.direct_boot();
    arm9.direct_boot();
    logger.debug("System: direct booted successfully");
}

void System::write_wramcnt(u8 data) {
    wramcnt = data & 0x3;
    arm7.get_memory().update_wram_mapping();
    arm9.get_memory().update_wram_mapping();
}

} // namespace core::nds