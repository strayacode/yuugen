add_library(common
    Types.h Log.h
    log_file.h log_file.cpp
    lut_helpers.h
    arithmetic.h
    circular_buffer.h
    memory_mapped_file.h
    format.h
    FileSystem.h FileSystem.cpp
    GamesList.h GamesList.cpp
    Memory.h
    GPUTypes.h
    StringHelpers.h StringHelpers.cpp
    Settings.h Settings.cpp
)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
