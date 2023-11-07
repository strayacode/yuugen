cmake_minimum_required(VERSION 3.10.1)

project(yuugen VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(LTO "Enable link time optimisations" ON)
option(CPU_DEBUG "Enable CPU debugging" OFF)
option(BUILD_TESTS "Build tests" ON)

add_compile_options(
    -Wall
    -Wextra
    -Wno-missing-braces
)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified, defaulting to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-D_DEBUG)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "Using -Ofast")
    add_compile_options(-Ofast)
endif()

if(LTO)
    message(STATUS "Using link time optimisations...")
    add_compile_options(-flto)
endif()

if(CPU_DEBUG)
    message(STATUS "CPU debugging enabled...")
    add_definitions(-DCPU_DEBUG)
endif()

add_subdirectory(src)

if(BUILD_TESTS)
    message(STATUS "Building tests...")
    add_subdirectory(tests)
endif()