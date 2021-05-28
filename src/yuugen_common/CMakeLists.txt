add_library(yuugen_common
    emu_thread.h emu_thread.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(yuugen_common PRIVATE Threads::Threads)

set_target_properties(yuugen_common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(yuugen_common PUBLIC core common "${CMAKE_CURRENT_SOURCE_DIR}/..")
