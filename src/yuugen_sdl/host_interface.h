#pragma once

#include <core/core.h>
#include <string>
#include <chrono>
#include <SDL2/SDL.h>
#include <common/log.h>
#include <common/types.h>
#include <yuugen_common/emu_thread.h>

class HostInterface {
    HostInterface();
    auto Initialise() -> bool;
    void Run(std::string path);
    void Cleanup();
    void UpdateTitle(int fps);

    Core core;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* top_texture = nullptr;
    SDL_Texture* bottom_texture = nullptr;

    SDL_Rect top_texture_area;
    SDL_Rect bottom_texture_area;

    SDL_Event event;

    int window_size = 1;
};