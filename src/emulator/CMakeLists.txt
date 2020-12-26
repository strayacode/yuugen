add_library(emulator
    emulator.cpp emulator.h
    core/arm.cpp core/arm.h
    core/arm_instrs.cpp core/thumb_instrs.cpp
    core/memory.cpp core/memory.h
    core/cp15.cpp core/cp15.h
    core/cartridge.cpp core/cartridge.h
    core/disassembler.cpp core/barrel_shifter.cpp
    common/types.h common/arithmetic.h
    core/gpu.cpp core/gpu.h
    core/gpu_2d.cpp core/gpu_2d.h
    core/dma.cpp core/dma.h
    common/arithmetic.cpp
)

set (DIRECT_BOOT ON)
set (FIRMWARE_BOOT OFF)
 
if (DIRECT_BOOT)
    target_compile_definitions(emulator PRIVATE DIRECT_BOOT)
endif()

if (FIRMWARE_BOOT)
    target_compile_definitions(emulator PRIVATE FIRMWARE_BOOT)
endif()



target_include_directories(emulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")