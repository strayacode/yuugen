set(PLATFORM_SDL OFF)
set(PLATFORM_QT ON)

add_subdirectory(common)
add_subdirectory(nds)

if(PLATFORM_SDL)
    add_subdirectory(platform_sdl)
elseif(PLATFORM_QT)
    add_subdirectory(platform_qt)
endif()