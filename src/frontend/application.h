#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <memory>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imfilebrowser.h"
#include "common/types.h"
#include "common/system.h"
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

    template <typename... Args>
    void monospace_text(const char* fmt, Args... args) {
        font_database.push_style(FontDatabase::Style::Monospace);
        ImGui::Text(fmt, args...);

        font_database.pop_style();
    }

    bool running = true;

    SDL_Window* window;
    SDL_GLContext gl_context;
    const char* glsl_version = "#version 330";

    ImGui::FileBrowser file_dialog;

    ImVec2 scaled_dimensions;
    static constexpr float menubar_height = 21;
    int window_width = 0;
    int window_height = 0;

    FontDatabase font_database;

private:
    void begin_fullscreen_window(const char *name, float padding = 0.0f);
    void end_fullscreen_window();
    
    void render_gba();
    void render_nds();
    void render_menubar();
    void render_performance_overlay();
    
    void boot_game(const std::string& path);
    void boot_firmware();

    bool demo_window = false;
    std::unique_ptr<common::System> system;

    enum class SystemType {
        None,
        GBA,
        NDS,
    };

    SystemType system_type = SystemType::None;
    ImVec4 grey0 = ImVec4(0.143f, 0.150f, 0.153f, 1.000f);
    ImVec4 grey1 = ImVec4(0.100f, 0.100f, 0.100f, 1.000f);
    ImVec4 grey2 = ImVec4(0.090f, 0.090f, 0.090f, 1.000f);
    ImVec4 black = ImVec4(0.000f, 0.000f, 0.000f, 1.000f);
    ImVec4 blue = ImVec4(0.218f, 0.359f, 0.832f, 1.000f);
    ImVec4 window_bg = ImVec4(0.100f, 0.100f, 0.100f, 0.980f);

    f32 fps;
    std::shared_ptr<common::AudioDevice> audio_device;
};