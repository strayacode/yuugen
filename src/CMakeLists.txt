set(PLATFORM_SDL ON)

add_subdirectory(emulator)


if(PLATFORM_SDL)
    add_subdirectory(platform/sdl)
endif()