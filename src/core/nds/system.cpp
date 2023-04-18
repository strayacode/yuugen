#include "common/logger.h"
#include "core/nds/system.h"

namespace core::nds {

System::System(Config config) : arm7(*this), arm9(*this), cartridge(*this) {
    arm7.select_backend(arm::Backend::Interpreter);
    arm9.select_backend(arm::Backend::Interpreter);
    cartridge.load(config.game_path);

    if (config.boot_mode == BootMode::Direct) {
        direct_boot();
    }
}

void System::run_frame() {

}

void System::reset() {

}

void System::direct_boot() {
    cartridge.direct_boot();
    arm7.direct_boot();
    arm9.direct_boot();
    logger.debug("System: direct booted successfully");
}

} // namespace core::nds