add_library(emulator
    emulator.cpp emulator.h
    core/arm7.cpp core/arm7.h
    core/arm9.cpp core/arm9.h
    core/arm7_arm_instrs.cpp core/arm7_thumb_instrs.cpp
    core/arm9_arm_instrs.cpp core/arm9_thumb_instrs.cpp
    core/memory.cpp core/memory.h
    core/cp15.cpp core/cp15.h
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