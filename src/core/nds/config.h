#pragma once

#include <string>

namespace core::nds {

enum class BootMode {
    Firmware,
    Direct,
};

struct Config {
    const std::string& game_path;
    BootMode boot_mode;
};

} // namespace core::nds