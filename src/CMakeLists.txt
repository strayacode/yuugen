set(FRONTEND_SDL OFF)
set(FRONTEND_QT ON)

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(yuugen_common)

if (FRONTEND_SDL)
    add_subdirectory(yuugen_sdl)
elseif(FRONTEND_QT)
    add_subdirectory(yuugen_qt)
endif()
