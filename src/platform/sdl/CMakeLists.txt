add_executable(ChronoDS main.cpp)

find_package(SDL2 REQUIRED)
include_directories(ChronoGB ${SDL2_INCLUDE_DIRS})
message(${SDL2_INCLUDE_DIRS})

target_link_libraries(ChronoDS PRIVATE emulator ${SDL2_LIBRARIES})