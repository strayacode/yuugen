#pragma once

#include <core/core.h>
#include <memory>
#include <string>
#include <SDL2/SDL.h>
#include <common/log.h>
#include <common/types.h>

struct HostInterface {
    HostInterface();
    bool Initialise();
    void Run(std::string path);
    void Cleanup();

    std::unique_ptr<Core> core;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* top_texture = nullptr;
    SDL_Texture* bottom_texture = nullptr;

    SDL_Rect top_texture_area;
    SDL_Rect bottom_texture_area;

    SDL_Event event;
};