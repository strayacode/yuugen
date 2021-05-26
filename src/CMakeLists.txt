set(FRONTEND_SDL ON)
set(FRONTEND_QT OFF)

add_subdirectory(common)
add_subdirectory(core)

if (FRONTEND_SDL)
    add_subdirectory(yuugen_sdl)
elseif(FRONTEND_QT)
    add_subdirectory(yuugen_qt)
endif()
