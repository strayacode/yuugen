# Turn on automatic invocation of the MOC, UIC & RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(SOURCES
    main.cpp
    mainwindow.cpp
    configwindow.cpp
)

find_package(Qt5 COMPONENTS Core Widgets)

add_executable(yuugen ${SOURCES})
target_link_libraries(yuugen PRIVATE core common yuugen_common Qt5::Widgets)
