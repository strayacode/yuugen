set(PLATFORM_SDL ON)

add_subdirectory(dmg)
add_subdirectory(nds)

if(PLATFORM_SDL)
    add_subdirectory(platform/sdl)
endif()