add_executable(yuugen main.cpp host_interface.cpp host_interface.h audio_interface.h audio_interface.cpp)

find_package(SDL2 REQUIRED)

include_directories(yuugen ${SDL2_INCLUDE_DIRS})

target_link_libraries(yuugen PRIVATE core common ${SDL2_LIBRARIES})
