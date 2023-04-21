#pragma once

#include <memory>
#include <thread>
#include <string>
#include "core/system.h"

namespace core {

class Core {
public:
    void set_game_path(const std::string& path);
    void start();

private:
    enum State {
        Running,
        Idle,
        Paused,
    };

    // place generic input, audio and video devices here alongside the emulator
    std::unique_ptr<System> system;
    std::string game_path;
    std::thread emulator_thread;
    State state = State::Idle;
};

} // namespace core