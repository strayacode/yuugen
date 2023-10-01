add_library(common
    types.h
    logger.h logger.cpp
    bits.h
    ring_buffer.h
    memory_mapped_file.h
    regular_file.h regular_file.cpp
    memory.h
    string.h string.cpp
    callback.h
    video_device.h
    audio_device.h
    page_table.h
    config.h
    system.h system.cpp
)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
