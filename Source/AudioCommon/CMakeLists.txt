find_package(SDL2 REQUIRED)

add_library(audiocommon
    AudioInterface.h
    SDLAudioInterface.h SDLAudioInterface.cpp
)

message(STATUS ${SDL2_INCLUDE_DIRS})
message(STATUS ${SDL2_LIBRARIES})

# TODO: don't hardcode this
include_directories("/opt/homebrew/include")
target_link_libraries(audiocommon PRIVATE ${SDL2_LIBRARIES})

set_target_properties(audiocommon PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(audiocommon PUBLIC common "${CMAKE_CURRENT_SOURCE_DIR}/..")