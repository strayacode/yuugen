add_library(core
    core.h core.cpp
    arm.h arm.cpp
    memory.h memory.cpp
    cartridge.h cartridge.cpp
    gpu.h gpu.cpp
    gpu_2d.h gpu_2d.cpp
    dma.h dma.cpp
    cp15.h cp15.cpp
    input.h input.cpp
    interrupt.h interrupt.cpp
    ipc.h ipc.cpp
    timers.h timers.cpp
    rtc.h rtc.cpp
    spu.h spu.cpp
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_link_libraries(core PRIVATE common)
