cmake_minimum_required(VERSION 3.1.0)

project(ChronoDS VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_FLAGS "-Wall -std=c++17 -Ofast")
add_subdirectory(src)