add_library(core
    core.h core.cpp
    system.h

    arm/config.h
    arm/coprocessor.h
    arm/fastmem.h arm/virtual_page_table.h
    arm/memory.h

    nds/system.h nds/system.cpp
    nds/config.h
    nds/hardware/cartridge/cartridge.h nds/hardware/cartridge/cartridge.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(core PRIVATE Threads::Threads)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common)
