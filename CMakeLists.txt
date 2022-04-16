cmake_minimum_required(VERSION 3.9.4)

project(yuugen VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_compile_options(
    -Wall
    -Wextra
    -pedantic
    -pedantic-errors
    -Wno-missing-braces
)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified, defaulting to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-D_DEBUG)
endif()

add_subdirectory(src)
