add_executable(ChronoDS main.cpp host_interface.cpp)

find_package(SDL2 REQUIRED)
include_directories(ChronoDS ${SDL2_INCLUDE_DIRS})
message(${SDL2_INCLUDE_DIRS})

target_link_libraries(ChronoDS PRIVATE nds gba common ${SDL2_LIBRARIES})