#include "core/nds/system.h"

namespace core::nds {

System::System(Config config) {
    cartridge.load(config.game_path);
}

void System::run_frame() {

}

void System::reset() {

}

} // namespace core::nds