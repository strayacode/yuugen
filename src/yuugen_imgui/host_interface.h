#pragma once

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

    bool Initialise();
    void Run();
    void Shutdown();
    void HandleInput();
    void UpdateTitle(float fps);
    void DrawMenubar();
    void DrawScreen();
    void SetupStyle();
    void CartridgeWindow();
    
    // the arch argument specifies whether to render window for arm7 or arm9
    void ARMWindow(CPUArch arch);
    void GPUWindow();
    void GPU2DWindow();
    void SchedulerWindow();
    void DMAWindow();
    void InputSettingsWindow();

    // TODO: sort of memory possible leaking
    void UpdateControllerList();

    bool running = true;
    bool fullscreen = false;

    Core core;

    SDL_Window* window;
    SDL_GLContext gl_context;
    const char* glsl_version = "#version 330";

    ImGui::FileBrowser file_dialog;

    // TODO: separate framebuffer display into its own class
    GLuint textures[2];

    bool cartridge_window = false;
    bool arm7_window = false;
    bool arm9_window = false;
    bool gpu_window = false;
    bool gpu_2d_window = false;
    bool scheduler_window = false;
    bool dma_window = false;
    bool input_settings_window = false;
    bool demo_window = false;

    int disassembly_size = 15;

    ImVec2 scaled_dimensions;
    static constexpr float menubar_height = 19;
    double center_pos = 0;

    std::vector<SDL_GameController*> controller_list;
    SDL_GameController* current_controller = nullptr;

    ImFont* regular_font = nullptr;
    ImFont* monospace_font = nullptr;
};