add_library(core
    core.h core.cpp
    config.h config.cpp
    system.h

    gba/gba.h gba/gba.cpp
    gba/memory/memory.h gba/memory/memory.cpp
    gba/cartridge/cartridge.h gba/cartridge/cartridge.cpp
    
    nds/nds.h nds/nds.cpp
    nds/cp15/cp15.h nds/cp15/cp15.cpp

    arm/memory_base.h arm/cpu_registers.h
    arm/cpu_core.h arm/cpu_core.cpp
    arm/interpreter/lookup_table.cpp
    arm/disassembler/disassembler.h

    scheduler/scheduler.h scheduler/scheduler.cpp
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common yuugen_common audio_common)
