add_library(core
    core.h core.cpp
    system.h

    nds/system.h nds/system.cpp
    nds/config.h
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(core PRIVATE Threads::Threads)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PUBLIC common)
