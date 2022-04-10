add_library(common
    types.h log.h
    memory_helpers.h matrix.h
    matrix_stack.h
    polygon.h vertex.h
    log_file.h log_file.cpp
    lut_helpers.h
    arithmetic.h
    circular_buffer.h
    slope.h memory_mapped_file.h
    format.h
    FileSystem.h FileSystem.cpp
    GamesList.h GamesList.cpp
    Memory.h
)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
