add_library(gba
    system.h system.cpp
    cp14.h cp14.cpp
    memory.h memory.cpp
    mmio.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(gba PRIVATE Threads::Threads)

include_directories(gba PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(gba PUBLIC arm common)
