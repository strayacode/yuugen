#pragma once

#include <memory>
#include <core/core.h>
#include <common/log.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// provides an interface for the sdl frontend and the yuugen core

struct HostInterface {
    HostInterface();
    bool Initialise();
    void Run(const char* path);
    void Cleanup();

    std::unique_ptr<Core> core;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* top_texture = nullptr;
    SDL_Texture* bottom_texture = nullptr;

    SDL_Rect top_texture_area;
    SDL_Rect bottom_texture_area;

    SDL_Event event;

    int frames = 0;
    char window_title[30];
};
