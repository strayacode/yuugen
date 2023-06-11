#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imfilebrowser.h"
#include "common/types.h"
#include "core/system.h"
#include "frontend/imgui_video_device.h"
#include "frontend/arm_debugger_window.h"
#include "frontend/font_database.h"

class Application {
public:
    Application();

    bool initialise();
    void start();
    void stop();
    void handle_input();
    void setup_style();
    void render();

    bool running = true;

    SDL_Window* window;
    SDL_GLContext gl_context;
    const char* glsl_version = "#version 330";

    ImGui::FileBrowser file_dialog;

    ImVec2 scaled_dimensions;
    static constexpr float menubar_height = 18;
    double center_pos = 0;
    int window_width = 0;
    int window_height = 0;

private:
    void begin_fullscreen_window(const char *name, float padding = 0.0f);
    void end_fullscreen_window();
    
    void render_screens();
    void render_menubar();
    void render_performance_overlay();
    
    void boot_game(const std::string& path);
    void boot_firmware();

    bool demo_window = true;
    core::System system;

    // TODO: combine top and bottom screen into single VideoDevice interface
    ImGuiVideoDevice top_screen;
    ImGuiVideoDevice bottom_screen;

    f32 fps;
    FontDatabase font_database;

    ARMDebuggerWindow arm7_debugger_window;
    ARMDebuggerWindow arm9_debugger_window;
};