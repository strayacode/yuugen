find_package(SDL2 REQUIRED)

add_library(audio_common
    audio_interface.h
    sdl/audio_interface.h sdl/audio_interface.cpp
)

include_directories(audio_common ${SDL2_INCLUDE_DIRS})
target_link_libraries(audio_common PRIVATE ${SDL2_LIBRARIES})

set_target_properties(audio_common PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(audio_common PUBLIC common "${CMAKE_CURRENT_SOURCE_DIR}/..")