#pragma once

#include <stdlib.h>
#include <string.h>
#include <string>
#include "Common/Types.h"
#include "Common/Log.h"
#include "Common/GamesList.h"
#include "core/core.h"
#include "core/arm/Disassembler/Disassembler.h"
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

    bool initialise();
    void Run();
    void Shutdown();
    void HandleInput();
    void UpdateTitle(float fps);
    void DrawMenubar();
    void SetupStyle();
    void CartridgeWindow();
    
    // the arch argument specifies whether to render window for arm7 or arm9
    void ARMWindow(CPUArch arch);
    void GPUWindow();
    void GPU2DWindow();
    void SchedulerWindow();
    void DMAWindow();
    void InputSettingsWindow();

    void render();

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
    int window_width = 0;
    int window_height = 0;

    ImFont* regular_font = nullptr;
    ImFont* monospace_font = nullptr;

    

private:
    void begin_fullscreen_window(const char *name, float padding = 0.0f);
    void end_fullscreen_window();
    
    void render_games_list_window();
    void render_screen();
    void reset_title();

    Disassembler disassembler;

    enum class WindowType {
        GamesList,
        Game,
    };

    WindowType window_type = WindowType::GamesList;

    Common::GamesList games_list;
};