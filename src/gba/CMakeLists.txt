add_library(gba
    system.h system.cpp
    memory.h memory.cpp
    mmio.cpp

    hardware/cartridge.h hardware/cartridge.cpp
    hardware/irq.h hardware/irq.cpp
    hardware/input.h hardware/input.cpp
    hardware/timers.h hardware/timers.cpp
    hardware/dma.h hardware/dma.cpp
    hardware/apu.h hardware/apu.cpp

    video/ppu.h video/ppu.cpp
    video/background.cpp video/bitmap.cpp
    video/composer.cpp video/tile_decoder.cpp
    video/object.cpp video/affine.cpp
)

include_directories(gba PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(gba PUBLIC arm common)
