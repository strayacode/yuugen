add_executable(yuugen_tests main.cpp)
target_link_libraries(yuugen_tests arm nds common)

find_package(Threads REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(X11 REQUIRED)
endif()

target_link_libraries(yuugen_tests ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${CMAKE_DL_LIBS})