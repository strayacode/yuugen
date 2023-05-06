#pragma once

#include <string>

namespace core {

enum class BootMode {
    Firmware,
    Direct,
};

struct Config {
    std::string game_path;
    BootMode boot_mode;
};

} // namespace core