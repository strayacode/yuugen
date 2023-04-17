#include "core/core.h"

namespace core {

void Core::set_game_path(const std::string& path) {
    game_path = path;
}

void Core::start() {
    system->load();
}

} // namespace core