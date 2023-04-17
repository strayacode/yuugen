#pragma once

#include <memory>
#include <string>
#include "core/system.h"

namespace core {

class Core {
public:
    void set_game_path(const std::string& path);
    void start();

private:
    // place generic input, audio and video devices here alongside the emulator
    std::unique_ptr<System> system;
    std::string game_path;
};

} // namespace core