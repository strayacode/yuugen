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
#include "common/games_list.h"
#include "arm/cpu.h"
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
    double center_pos_x{0};
    int window_width = 0;
    int window_height = 0;

    FontDatabase font_database;

private:
    void begin_fullscreen_window(const char *name, float padding_x = 0.0f, float padding_y = 0.0f);
    void end_fullscreen_window();
    
    void render_gba();
    void render_nds();
    void render_menubar();
    void render_library_screen();
    void render_settings_screen();

    void handle_input_gba(SDL_Event& event);
    void handle_input_nds(SDL_Event& event);
    
    void boot_game(const std::string& path);
    void boot_firmware();

    enum class ScreenType {
        Library,
        Settings,
        Game,
    };

    void switch_screen(ScreenType screen_type);
    void switch_to_previous();

    bool demo_window{false};
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
    ImVec4 light_grey = ImVec4(0.650f, 0.650f, 0.650f, 1.000f);
    ImVec4 yellow = ImVec4(1.0000f, 0.941f, 0.335f, 1.000f);
    ImVec4 black = ImVec4(0.000f, 0.000f, 0.000f, 1.000f);
    ImVec4 blue = ImVec4(0.218f, 0.359f, 0.832f, 1.000f);
    ImVec4 window_bg = ImVec4(0.100f, 0.100f, 0.100f, 0.980f);

    f32 fps{0.0f};
    std::shared_ptr<common::AudioDevice> audio_device;

    ScreenType previous_screen_type{ScreenType::Library};
    ScreenType screen_type{ScreenType::Library};
    common::GamesList games_list;
    arm::BackendType backend_type{arm::BackendType::Jit};
    arm::BackendType new_backend_type{arm::BackendType::Jit};

    // TODO: create an SDLInputDevice to abstract away input
};