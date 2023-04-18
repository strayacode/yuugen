add_library(core
    core.h core.cpp
    system.h

    arm/config.h
    arm/coprocessor.h
    arm/fastmem.h arm/virtual_page_table.h
    arm/memory.h
    arm/cpu.h arm/state.h
    arm/interpreter/interpreter.h arm/interpreter/interpreter.cpp

    nds/system.h nds/system.cpp
    nds/config.h

    nds/arm7/arm7.h nds/arm7/arm7.cpp
    nds/arm7/memory.h nds/arm7/memory.cpp

    nds/arm9/arm9.h nds/arm9/arm9.cpp
    nds/arm9/memory.h nds/arm9/memory.cpp

    nds/hardware/cartridge/cartridge.h nds/hardware/cartridge/cartridge.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(core PRIVATE Threads::Threads)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common)
