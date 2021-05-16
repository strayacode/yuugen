add_library(core
    core.h core.cpp
    arm/arm7/bus.cpp arm/arm7/mmio.cpp
    arm/arm9/bus.cpp arm/arm9/mmio.cpp
    arm/arm.h arm/arm.cpp
    arm/memory.h arm/memory.cpp
    
    hw/cartridge/cartridge.h hw/cartridge/cartridge.cpp
    hw/spi/spi.h hw/spi/spi.cpp
    scheduler/scheduler.h scheduler/scheduler.cpp
    hw/cp15/cp15.h hw/cp15/cp15.cpp
    hw/gpu/gpu.h hw/gpu/gpu.cpp
    hw/gpu/gpu_2d.h hw/gpu/gpu_2d.cpp
    hw/gpu/gpu_3d.h hw/gpu/gpu_3d.cpp
    hw/gpu/render/text.cpp hw/gpu/render/extended.cpp
    hw/gpu/compose.cpp hw/gpu/render/object.cpp
    hw/dma/dma.h hw/dma/dma.cpp
    hw/input/input.h hw/input/input.cpp
    hw/ipc/ipc.h hw/ipc/ipc.cpp
    hw/interrupt/interrupt.h hw/interrupt/interrupt.cpp
    hw/timers/timers.h hw/timers/timers.cpp
    hw/spu/spu.h hw/spu/spu.cpp
    hw/rtc/rtc.h hw/rtc/rtc.cpp
    hw/maths_unit/maths_unit.h hw/maths_unit/maths_unit.cpp
    hw/wifi/wifi.h hw/wifi/wifi.cpp
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PRIVATE common)
