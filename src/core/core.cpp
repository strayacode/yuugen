#include "common/logger.h"
#include "core/core.h"
#include "core/nds/system.h"

namespace core {

void Core::set_game_path(const std::string& path) {
    game_path = path;
}

void Core::start() {
    nds::Config config;
    config.game_path = game_path;
    config.boot_mode = nds::BootMode::Direct;
    system = std::make_unique<nds::System>(config);
    state = State::Running;

    emulator_thread = std::thread{[this]() {
        system->run_frame();
    }};
}

} // namespace core