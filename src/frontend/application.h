#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imfilebrowser.h"
#include "common/types.h"
#include "core/system.h"
#include "frontend/gl_video_device.h"

class Application {
public:
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
    
    void render_display_window();
    void render_screens();
    void render_menubar();
    
    void boot_game(const std::string& path);
    void boot_firmware();

    bool demo_window = true;
    core::System system;

    GLVideoDevice top_screen;
    GLVideoDevice bottom_screen;
};