set(SOURCES
    main.cpp
    application.h application.cpp
    imgui_video_device.h imgui_video_device.cpp
    sdl_audio_device.h sdl_audio_device.cpp
    widgets/cpu_debugger.h widgets/cpu_debugger.cpp
    font_database.h
)

set(IMGUI_SOURCES
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_opengl3.h
    imgui/imgui_impl_sdl.cpp
    imgui/imgui_impl_sdl.h
    imgui/imgui_memory_editor.h
)

add_executable(yuugen ${SOURCES} ${IMGUI_SOURCES})

find_package(SDL2 REQUIRED)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
target_link_libraries(yuugen ${OPENGL_LIBRARIES})

include_directories(imgui ${SDL2_INCLUDE_DIRS})
target_link_libraries(yuugen arm nds common ${SDL2_LIBRARIES})

find_package(Threads REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(X11 REQUIRED)
endif()

target_link_libraries(yuugen ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${CMAKE_DL_LIBS})