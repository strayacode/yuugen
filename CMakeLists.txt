cmake_minimum_required(VERSION 3.2)

project(ChronoDS)

set(GPROF_COMPILE OFF)



set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_FLAGS "-Wall -std=c++17 -Ofast")
add_subdirectory(src)