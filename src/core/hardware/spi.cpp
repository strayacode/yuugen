#include "core/hardware/spi.h"

namespace core {

void SPI::reset() {
    spicnt = 0;
    spidata = 0;
}

} // namespace core