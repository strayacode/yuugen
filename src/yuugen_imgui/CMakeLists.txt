include_directories("${CMAKE_CURRENT_SOURCE_DIR}/addons" "${CMAKE_CURRENT_SOURCE_DIR}/imgui" "${CMAKE_CURRENT_SOURCE_DIR}/imgui-sfml")

set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_tables.cpp
    imgui-sfml/imgui-SFML.cpp
)

add_executable(yuugen main.cpp host_interface.cpp ${IMGUI_SOURCES})

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
target_link_libraries(yuugen ${OPENGL_LIBRARIES})

find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)
if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(yuugen core util ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} sfml-graphics sfml-audio)
endif()