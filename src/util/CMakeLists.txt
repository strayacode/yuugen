add_library(util
    types.h log.h
    memory_helpers.h
)

set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
