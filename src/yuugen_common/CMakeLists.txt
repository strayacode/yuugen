add_library(yuugen_common
    emu_thread.h emu_thread.cpp
)

set_target_properties(yuugen_common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(yuugen_common PUBLIC core common "${CMAKE_CURRENT_SOURCE_DIR}/..")
