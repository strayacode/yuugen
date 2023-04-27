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
        logger.debug("run arm7");
        arm7.run(1);
        logger.debug("run arm9");
        arm9.run(2);
        scheduler.tick(1);
        scheduler.run_events();
    }
}

void System::reset() {
    scheduler.reset();
    arm7.reset();
    arm9.reset();

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

    // switch (wramcnt) {
    // case 0x0:
    //     arm7.get_memory().map<arm::Bus::All>(0x03000000, 0x03800000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
    //     arm7.get_memory().map<arm::Bus::All>(0x03000000, 0x03800000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
    //     break;
    // case 0x1:
    //     arm7.get_memory().map<arm::Bus::All>(0x03000000, 0x03800000, shared_wram.data(), 0x3fff, arm::RegionAttributes::ReadWrite);
    //     arm7.get_memory().map<arm::Bus::All>(0x03000000, 0x03800000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
    //     break;
    // case 0x2:
    //     arm7.get_memory().map<arm::Bus::All>(0x03000000, 0x03800000, arm7_wram.data() + 0x4000, 0x3fff, arm::RegionAttributes::ReadWrite);
    //     arm7.get_memory().map<arm::Bus::All>(0x03000000, 0x03800000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
    //     break;
    // case 0x3:
    //     arm7.get_memory().map<arm::Bus::All>(0x03000000, 0x03800000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
    //     arm7.get_memory().map<arm::Bus::All>(0x03000000, 0x03800000, arm7_wram.data(), 0xffff, arm::RegionAttributes::ReadWrite);
    //     break;
    // }
    // arm7.memory().update_memory_map(0x03000000, 0x04000000);
    // arm9.memory().update_memory_map(0x03000000, 0x04000000);
}

} // namespace core::nds