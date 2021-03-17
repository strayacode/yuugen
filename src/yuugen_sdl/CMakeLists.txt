add_executable(yuugen main.cpp host_interface.cpp)

find_package(SDL2 REQUIRED)

include_directories(yuugen ${SDL2_INCLUDE_DIRS})

target_link_libraries(yuugen PRIVATE core util ${SDL2_LIBRARIES})
