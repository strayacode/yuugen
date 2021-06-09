set(FRONTEND_SDL ON)
set(FRONTEND_QT OFF)

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(yuugen_common)

if (FRONTEND_SDL)
    add_subdirectory(yuugen_sdl)
elseif(FRONTEND_QT)
    add_subdirectory(yuugen_qt)
endif()
