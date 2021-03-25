set(PLATFORM_SDL ON)
set(PLATFORM_QT OFF)

add_subdirectory(util)
add_subdirectory(core)
add_subdirectory(yuugen_common)

if (PLATFORM_SDL)
    add_subdirectory(yuugen_sdl)
elseif(PLATFORM_QT)
    add_subdirectory(yuugen_qt)
endif()
