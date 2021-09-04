#pragma once

// #include <audio_common/sdl/audio_interface.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <common/types.h>
#include <common/log.h>
#include <core/core.h>
// #include "addons/imfilebrowser.h"
#include <memory>
#include <vector>
#include <array>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include <SDL.h>
#include <SDL_opengl.h>

class HostInterface {
public:
    HostInterface();

    auto Initialise() -> bool;
    void Run();
    void Shutdown();
    void HandleInput();
    void UpdateTitle(float fps);

    bool running = true;

    Core core;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    SDL_Window* window;
    SDL_GLContext gl_context;
    const char* glsl_version = "#version 330";
};