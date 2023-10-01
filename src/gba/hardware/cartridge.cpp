#include "common/logger.h"
#include "gba/hardware/cartridge.h"

namespace gba {

void Cartridge::reset() {

}

void Cartridge::load(const std::string& path) {
    memory_mapped_file.load(path);
}

} // namespace gba