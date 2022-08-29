#pragma once

#include <stdlib.h>
#include <string.h>
#include <string>
#include "Common/Types.h"
#include "Common/Log.h"
#include "Common/GamesList.h"
#include "VideoCommon/GLWindow.h"
#include "Core/System.h"
#include "Core/ARM/Disassembler/Disassembler.h"
#include <memory>
#include <vector>
#include <array>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imfilebrowser.h"
#include "yuugen_imgui/OnScreenDisplay.h"
#include <SDL.h>
#include <SDL_opengl.h>

class HostInterface {
public:
    HostInterface();

    bool initialise();
    void Run();
    void Shutdown();
    void HandleInput();
    void SetupStyle();
    void CartridgeWindow();
    void ARMWindow(Arch arch);
    void DMAWindow();

    void render();

    bool running = true;

    SDL_Window* window;
    SDL_GLContext gl_context;
    const char* glsl_version = "#version 330";

    ImGui::FileBrowser file_dialog;

    // TODO: separate framebuffer display into its own class
    GLuint textures[2];

    bool cartridge_window = false;
    bool arm7_window = false;
    bool arm9_window = false;
    bool scheduler_window = false;
    bool dma_window = false;
    bool demo_window = false;

    int disassembly_size = 15;

    ImVec2 scaled_dimensions;
    static constexpr float menubar_height = 18;
    double center_pos = 0;
    int window_width = 0;
    int window_height = 0;

    ImFont* regular_font = nullptr;
    ImFont* monospace_font = nullptr;

private:
    void begin_fullscreen_window(const char *name, float padding = 0.0f);
    void end_fullscreen_window();
    
    void render_games_list_window();
    void render_settings_window();
    void render_screens();
    void render_menu_bar();
    void reset_title();

    void boot_game(std::string path);
    void boot_firmware();

    void set_fullscreen(bool value);

    void take_screenshot();

    void toggle_pause();

    Disassembler disassembler;

    enum class WindowType {
        GamesList,
        Game,
    };

    WindowType window_type = WindowType::GamesList;

    Common::GamesList games_list;

    GLWindow top_screen;
    GLWindow bottom_screen;

    bool fullscreen = false;
    bool settings_window = false;

    OnScreenDisplay osd;

    float m_fps;

    System m_system;
};