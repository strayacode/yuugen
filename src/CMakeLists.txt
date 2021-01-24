set(PLATFORM_SDL ON)
set(PLATFORM_QT OFF)

add_subdirectory(common)
add_subdirectory(nds)

if(PLATFORM_SDL)
    add_subdirectory(platform_sdl)
elseif(PLATFORM_QT)
    add_subdirectory(platform_qt)
endif()