#include "common/logger.h"
#include "common/system.h"

namespace common {

void System::start() {
    // stop the previous thread if one was running
    stop();
    reset();

    audio_device->set_state(common::AudioState::Playing);

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

    audio_device->set_state(common::AudioState::Idle);

    thread_state = ThreadState::Idle;
    thread.join();
}

void System::set_game_path(const std::string& game_path) {
    config.game_path = game_path;
}

void System::set_boot_mode(BootMode boot_mode) {
    config.boot_mode = boot_mode;
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

} // namespace common