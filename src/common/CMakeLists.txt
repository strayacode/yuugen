add_library(common
    types.h
    logger.h logger.cpp
    bits.h
    ring_buffer.h
    memory_mapped_file.h
    memory.h
    string.h string.cpp
    callback.h
    video_device.h input_device.h
)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
