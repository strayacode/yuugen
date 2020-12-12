set(PLATFORM_SDL ON)

add_subdirectory(nds)

if(PLATFORM_SDL)
    add_subdirectory(platform/sdl)
endif()