add_library(yuugen_common
    emuthread.h emuthread.cpp
)
set_target_properties(yuugen_common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(yuugen_common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_link_libraries(yuugen_common PRIVATE core)