add_executable(test_cpu_compare test_cpu_compare.cpp)
target_link_libraries(test_cpu_compare arm nds common)

find_package(Threads REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(X11 REQUIRED)
endif()

target_link_libraries(test_cpu_compare ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(test_a64_assembler test_a64_assembler.cpp)
target_link_libraries(test_a64_assembler arm common)

find_package(Threads REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(X11 REQUIRED)
endif()

target_link_libraries(test_a64_assembler ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${CMAKE_DL_LIBS})