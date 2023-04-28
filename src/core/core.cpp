#include "common/logger.h"
#include "core/core.h"
#include "core/nds/system.h"

namespace core {

Core::~Core() {
    stop();
}

void Core::set_game_path(const std::string& path) {
    game_path = path;
}

void Core::start() {
    nds::Config config;
    config.game_path = game_path;
    config.boot_mode = nds::BootMode::Direct;
    system = std::make_unique<nds::System>(config);
    state = State::Running;
    thread_state = ThreadState::Running;

    emulator_thread = std::thread{[this]() {
        while (thread_state == ThreadState::Running) {
            system->run_frame();
        }
    }};
}

void Core::stop() {
    if (thread_state == ThreadState::Idle) {
        return;
    }

    thread_state = ThreadState::Idle;
    emulator_thread.join();
}

} // namespace core