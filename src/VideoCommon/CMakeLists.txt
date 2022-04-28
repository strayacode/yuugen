add_library(videocommon
    GPU.h GPU.cpp
    GLWindow.h GLWindow.cpp
    Renderer2D.h Renderer2D.cpp
    Renderer3D.h Renderer3D.cpp
    VRAMPage.h VRAMRegion.h
    Colour.h Colour.cpp
    GeometryCommands.cpp
)

if (POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif(POLICY CMP0072)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

target_include_directories(videocommon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/.." ${GLEW_INCLUDE_DIRS})
target_link_libraries(videocommon core common videobackends2d videobackends3d OpenGL::GL GLEW::GLEW)
