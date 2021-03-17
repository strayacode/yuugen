# Turn on automatic invocation of the MOC, UIC & RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


# Find the QtWidgets library
find_package(Qt5 COMPONENTS Core Widgets)

# Tell CMake to create the helloworld executable
add_executable(yuugen main.cpp mainwindow.h mainwindow.cpp)

# Add the Qt5 Widgets for linking
target_link_libraries(yuugen PRIVATE core yuugen_common util Qt5::Widgets)