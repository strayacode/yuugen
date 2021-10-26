#pragma once

#include <audio_common/sdl/audio_interface.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <common/types.h>
#include <common/log.h>
#include <core/core.h>
#include <memory>
#include <vector>
#include <array>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imfilebrowser.h"
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
    void DrawMenubar();
    void DrawScreen();
    void SetupStyle();
    void BootFirmware();

    void CartridgeWindow();
    void ARMWindow();
    void GPUWindow();
    void GPU2DWindow();
    void SchedulerWindow();

    bool running = true;
    bool fullscreen = false;

    Core core;

    SDL_Window* window;
    SDL_GLContext gl_context;
    const char* glsl_version = "#version 330";

    ImGui::FileBrowser file_dialog;

    SDLAudioInterface audio_interface;

    // TODO: separate framebuffer display into its own class
    GLuint textures[2];

    bool cartridge_window = false;
    bool arm_window = false;
    bool gpu_window = false;
    bool gpu_2d_window = false;
    bool scheduler_window = false;
};