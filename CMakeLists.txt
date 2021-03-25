cmake_minimum_required(VERSION 3.9.4)

project(yuugen VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -std=c++17 -Ofast -flto -no-pie")
add_subdirectory(src)
