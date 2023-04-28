#pragma once

#include <memory>
#include <thread>
#include <string>
#include "core/system.h"

namespace core {

class Core {
public:
    ~Core();

    void set_game_path(const std::string& path);
    void start();
    void stop();

private:
    enum class State {
        Running,
        Idle,
        Paused,
    };

    enum class ThreadState {
        Running,
        Idle,
    };

    // place generic input, audio and video devices here alongside the emulator
    std::unique_ptr<System> system;
    std::string game_path;
    std::thread emulator_thread;
    State state = State::Idle;
    ThreadState thread_state = ThreadState::Idle;
};

} // namespace core