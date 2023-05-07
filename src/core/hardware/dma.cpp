#include "core/hardware/dma.h"

namespace core {

void DMA::reset() {
    channels.fill(Channel{});
}

void DMA::write_length(int index, u16 value) {
    channels[index].length = value;
}

} // namespace core