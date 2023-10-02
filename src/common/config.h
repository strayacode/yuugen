#pragma once

#include <string>

namespace common {

enum class BootMode {
    Fast,
    Regular,
};

struct Config {
    std::string game_path;
    BootMode boot_mode;
};

} // namespace common