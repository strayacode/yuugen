find_package(SDL2 REQUIRED)

add_library(audiocommon
    AudioInterface.h
    SDLAudioInterface.h SDLAudioInterface.cpp
)

include_directories(audiocommon ${SDL2_INCLUDE_DIRS})
target_link_libraries(audiocommon PRIVATE ${SDL2_LIBRARIES})

set_target_properties(audiocommon PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(audiocommon PUBLIC common "${CMAKE_CURRENT_SOURCE_DIR}/..")