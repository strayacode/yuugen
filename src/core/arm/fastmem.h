#pragma once

namespace core::arm {

enum class Bus {
    // for instructions
    Code,

    // for regular data that gets read by instructions
    Data,

    // for components other than the cpu (e.g. DMA)
    System,
};

class Fastmem {
public:

private:
};

} // namespace core::arm