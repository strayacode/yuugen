set(SOFTWARE_FASTMEM ON)

add_library(core
    core.h core.cpp
    arm/cpu_base.h
    arm/memory_base.h arm/memory_base.cpp
    arm/cpu_registers.h
    arm/interpreter/interpreter.h
    arm/interpreter/interpreter.cpp
    arm/arm7/memory.h arm/arm7/memory.cpp
    arm/arm9/memory.h arm/arm9/memory.cpp
    
    hw/hw.h hw/hw.cpp
    
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PRIVATE common yuugen_common)
