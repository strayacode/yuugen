set(SOURCES
    main.cpp
)

add_executable(yuugen ${SOURCES})

target_link_libraries(yuugen core common)

find_package(Threads REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(X11 REQUIRED)
endif()

target_link_libraries(yuugen ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${CMAKE_DL_LIBS})