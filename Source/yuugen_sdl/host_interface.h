#pragma once


#include <string>
#include <chrono>
#include <SDL2/SDL.h>
#include "Common/Log.h"
#include "Common/Types.h"
#include "Core/System.h"

class HostInterface {
public:
    HostInterface();
    bool Initialise();
    void Run(std::string path);
    void Cleanup();
    void UpdateTitle(float fps);

private:
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