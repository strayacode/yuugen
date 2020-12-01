add_library(emulator
    emulator.cpp emulator.h
    core/arm.cpp core/arm.h
    core/memory.cpp core/memory.h
    




)

target_include_directories(emulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")