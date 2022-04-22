add_library(videocommon
    GPU.h GPU.cpp
    GLWindow.h GLWindow.cpp
    GPUState.h
    Renderer2D.h Renderer3D.h
    VRAMPage.h VRAMRegion.h
)

if (POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif(POLICY CMP0072)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

set_target_properties(videocommon PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(videocommon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/.." ${GLEW_INCLUDE_DIRS})
target_link_libraries(videocommon core common OpenGL::GL GLEW::GLEW)
