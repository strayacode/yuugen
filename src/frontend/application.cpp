#include <cassert>
#include "common/logger.h"
#include "common/string.h"
#include "gba/system.h"
#include "nds/system.h"
#include "frontend/application.h"
#include "frontend/sdl_audio_device.h"
#include "frontend/imgui_video_device.h"

Application::Application() {}

bool Application::initialise() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) > 0) {
        logger.warn("error initialising SDL!");
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    u32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    window = SDL_CreateWindow("yuugen", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    gl_context = SDL_GL_CreateContext(window);

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiIO& io = ImGui::GetIO();

    font_database.add_font(FontDatabase::Style::Regular, io.Fonts->AddFontFromFileTTF("../data/fonts/roboto-light.ttf", 16.0f));
    font_database.add_font(FontDatabase::Style::Large, io.Fonts->AddFontFromFileTTF("../data/fonts/roboto-light.ttf", 19.0f));
    font_database.add_font(FontDatabase::Style::Monospace, io.Fonts->AddFontFromFileTTF("../data/fonts/sf-mono-regular.otf", 15.0f));

    setup_style();
    SDL_GetWindowSize(window, &window_width, &window_height);

    audio_device = std::make_shared<SDLAudioDevice>();
    return true;
}

void Application::start() {
    if (!initialise()) {
        stop();
        return;
    }
        
    while (running) {
        handle_input();
        render();
    }

    stop();
}

void Application::stop() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::handle_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            window_width = event.window.data1;
            window_height = event.window.data2;
        } else {
            switch (system_type) {
            case SystemType::GBA:
                handle_input_gba(event);
                break;
            case SystemType::NDS:
                handle_input_nds(event);
                break;
            case SystemType::None:
                break;
            }
        }
    }
}

void Application::render_gba() {
    static ImGuiVideoDevice main_screen;
    main_screen.configure(240, 160, ImGuiVideoDevice::Filter::Nearest);
    
    auto framebuffers = system->fetch_framebuffers();
    assert(framebuffers.size() == 1);

    main_screen.update_texture(framebuffers[0]);
    
    const f64 scale_x = static_cast<f64>(window_width) / 240;
    const f64 scale_y = static_cast<f64>(window_height) / 160;
    const f64 scale = scale_x < scale_y ? scale_x : scale_y;

    scaled_dimensions = ImVec2(240 * scale, 160 * scale);
    ImVec2 center_pos = ImVec2((static_cast<f64>(window_width) - scaled_dimensions.x) / 2, (static_cast<f64>(window_height) - scaled_dimensions.y) / 2);

    ImGui::GetBackgroundDrawList()->AddImage(
        (void*)(intptr_t)main_screen.get_texture(),
        ImVec2(center_pos.x, menubar_height + center_pos.y),
        ImVec2(center_pos.x + scaled_dimensions.x, center_pos.y + scaled_dimensions.y),
        ImVec2(0, 0),
        ImVec2(1, 1),
        IM_COL32_WHITE
    );
}

void Application::render_nds() {
    static ImGuiVideoDevice top_screen;
    static ImGuiVideoDevice bottom_screen;
    top_screen.configure(256, 192, ImGuiVideoDevice::Filter::Nearest);
    bottom_screen.configure(256, 192, ImGuiVideoDevice::Filter::Nearest);

    auto framebuffers = system->fetch_framebuffers();
    assert(framebuffers.size() == 2);

    top_screen.update_texture(framebuffers[0]);
    bottom_screen.update_texture(framebuffers[1]);

    const f64 scale_x = static_cast<f64>(window_width) / 256;
    const f64 scale_y = static_cast<f64>(window_height) / 384;
    const f64 scale = scale_x < scale_y ? scale_x : scale_y;

    scaled_dimensions = ImVec2(256 * scale, 192 * scale);
    center_pos_x = (static_cast<f64>(window_width) - scaled_dimensions.x) / 2;

    ImGui::GetBackgroundDrawList()->AddImage(
        (void*)(intptr_t)top_screen.get_texture(),
        ImVec2(center_pos_x, menubar_height),
        ImVec2(center_pos_x + scaled_dimensions.x, scaled_dimensions.y),
        ImVec2(0, 0),
        ImVec2(1, 1),
        IM_COL32_WHITE
    );

    ImGui::GetBackgroundDrawList()->AddImage(
        (void*)(intptr_t)bottom_screen.get_texture(),
        ImVec2(center_pos_x, scaled_dimensions.y),
        ImVec2(center_pos_x + scaled_dimensions.x, scaled_dimensions.y * 2),
        ImVec2(0, 0),
        ImVec2(1, 1),
        IM_COL32_WHITE
    );
}

void Application::render_menubar() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load ROM")) {
                file_dialog.Open();
            }

            if (ImGui::MenuItem("Boot Firmware")) {
                boot_firmware();
            }

            if (ImGui::MenuItem("Power Off")) {
                // window_type = WindowType::GamesList;
                // m_system.set_state(State::Idle);
            }

            if (ImGui::MenuItem("Quit")) {
                running = false;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    file_dialog.Display();
    if (file_dialog.HasSelected()) {
        boot_game(file_dialog.GetSelected().string());
        file_dialog.ClearSelected();
    }
}

void Application::render_performance_overlay() {
    font_database.push_style(FontDatabase::Style::Large);
    if (system->get_state() == common::System::State::Running && static_cast<int>(fps) != 0) {
        auto fps_string = common::format("%.0f FPS | %.2f ms", fps, 1000.0f / fps);
        auto pos = ImVec2(window_width - ImGui::CalcTextSize(fps_string.c_str()).x - ImGui::GetStyle().ItemSpacing.x, menubar_height + ImGui::GetStyle().ItemSpacing.y);
        ImGui::GetBackgroundDrawList()->AddText(pos, IM_COL32_WHITE, fps_string.c_str());
    } else if (system->get_state() == common::System::State::Paused) {
        auto fps_string = "Paused";
        auto pos = ImVec2(window_width - ImGui::CalcTextSize(fps_string).x - ImGui::GetStyle().ItemSpacing.x, menubar_height + ImGui::GetStyle().ItemSpacing.y);
        ImGui::GetBackgroundDrawList()->AddText(pos, IM_COL32_WHITE, fps_string);
    }
    font_database.pop_style();
}

void Application::setup_style() {
    ImGui::GetStyle().WindowBorderSize = 1.0f;
    ImGui::GetStyle().PopupBorderSize = 0.0f;
    ImGui::GetStyle().ChildBorderSize = 0.0f;
    ImGui::GetStyle().GrabMinSize = 7.0f;
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;
    ImGui::GetStyle().TabRounding = 0.0f;
    ImGui::GetStyle().ScrollbarSize = 10.0f;
    ImGui::GetStyle().ScrollbarRounding = 12.0f;
    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.50f, 0.50f);
    ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;
    ImGui::GetStyle().FramePadding = ImVec2(4.0f, 2.0f);
    ImGui::GetStyle().ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.109f, 0.109f, 0.109f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.109f, 0.109f, 0.109f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = blue;
    ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = blue;
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGrip] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGripHovered] = blue;
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGripActive] = blue;
    ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = blue;
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.349f, 0.500f, 0.910f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_Tab] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImVec4(0.186f, 0.186f, 0.188f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImVec4(0.231f, 0.231f, 0.251f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = blue;
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = blue;
    ImGui::GetStyle().Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg] = ImVec4(0.210f, 0.210f, 0.210f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = window_bg;
    ImGui::GetStyle().Colors[ImGuiCol_Border] = black;
}

void Application::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    render_menubar();

    switch (system_type) {
    case SystemType::GBA:
        render_gba();
        render_performance_overlay();
        break;
    case SystemType::NDS:
        render_nds();
        render_performance_overlay();
        break;
    case SystemType::None:
        break;
    }
    
    if (demo_window) {
        ImGui::ShowDemoWindow();
    }

    ImGui::Render();
    glViewport(0, 0, 1280, 720);
    glClearColor(grey2.x, grey2.y, grey2.z, grey2.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

void Application::begin_fullscreen_window(const char *name, float padding) {
    ImGui::SetNextWindowPos(ImVec2(0, menubar_height));
    ImGui::SetNextWindowSize(ImVec2(window_width, window_height - menubar_height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin(
        name,
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus
    );
}

void Application::end_fullscreen_window() {
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

void Application::handle_input_gba(SDL_Event& event) {
    auto& gba_system = reinterpret_cast<gba::System&>(*system);
    if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
        bool pressed = event.type == SDL_KEYDOWN;
        switch (event.key.keysym.sym) {
        case SDLK_d:
            gba_system.input.handle_input(gba::InputEvent::A, pressed);
            break;
        case SDLK_s:
            gba_system.input.handle_input(gba::InputEvent::B, pressed);
            break;
        case SDLK_RSHIFT:
            gba_system.input.handle_input(gba::InputEvent::Select, pressed);
            break;
        case SDLK_RETURN:
            gba_system.input.handle_input(gba::InputEvent::Start, pressed);
            break;
        case SDLK_RIGHT:
            gba_system.input.handle_input(gba::InputEvent::Right, pressed);
            break;
        case SDLK_LEFT:
            gba_system.input.handle_input(gba::InputEvent::Left, pressed);
            break;
        case SDLK_UP:
            gba_system.input.handle_input(gba::InputEvent::Up, pressed);
            break;
        case SDLK_DOWN:
            gba_system.input.handle_input(gba::InputEvent::Down, pressed);
            break;
        case SDLK_e:
            gba_system.input.handle_input(gba::InputEvent::R, pressed);
            break;
        case SDLK_w:
            gba_system.input.handle_input(gba::InputEvent::L, pressed);
            break;
        }
    }
}

void Application::handle_input_nds(SDL_Event& event) {
    auto& nds_system = reinterpret_cast<nds::System&>(*system);
    if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
        bool pressed = event.type == SDL_KEYDOWN;
        switch (event.key.keysym.sym) {
        case SDLK_d:
            nds_system.input.handle_input(nds::InputEvent::A, pressed);
            break;
        case SDLK_s:
            nds_system.input.handle_input(nds::InputEvent::B, pressed);
            break;
        case SDLK_RSHIFT:
            nds_system.input.handle_input(nds::InputEvent::Select, pressed);
            break;
        case SDLK_RETURN:
            nds_system.input.handle_input(nds::InputEvent::Start, pressed);
            break;
        case SDLK_RIGHT:
            nds_system.input.handle_input(nds::InputEvent::Right, pressed);
            break;
        case SDLK_LEFT:
            nds_system.input.handle_input(nds::InputEvent::Left, pressed);
            break;
        case SDLK_UP:
            nds_system.input.handle_input(nds::InputEvent::Up, pressed);
            break;
        case SDLK_DOWN:
            nds_system.input.handle_input(nds::InputEvent::Down, pressed);
            break;
        case SDLK_e:
            nds_system.input.handle_input(nds::InputEvent::R, pressed);
            break;
        case SDLK_w:
            nds_system.input.handle_input(nds::InputEvent::L, pressed);
            break;
        }
    } else if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        int x = ((event.button.x - center_pos_x) / scaled_dimensions.x) * 256;
        int y = ((event.button.y - scaled_dimensions.y) / scaled_dimensions.y) * 192;
        
        if ((x >= 0 && x < 256) && (y >= 0 && y < 192)) {
            bool pressed = reinterpret_cast<SDL_MouseMotionEvent*>(&event)->state & SDL_BUTTON_LMASK;
            nds_system.input.set_touch(pressed);
            nds_system.input.set_point(x, y);
        }
    }
}

void Application::boot_game(const std::string& path) {
    auto extension = path.substr(path.find_last_of(".") + 1, path.size());
    if (extension == "gba") {
        system = std::make_unique<gba::System>();
        system_type = SystemType::GBA;
    } else if (extension == "nds") {
        system = std::make_unique<nds::System>();
        system_type = SystemType::NDS;
    } else {
        logger.todo("unhandled game extension %s", extension.c_str());
    }

    system->set_update_callback([this](f32 fps) {
        this->fps = fps;
    });

    system->set_audio_device(audio_device);
    system->set_game_path(path);
    system->set_boot_mode(common::BootMode::Fast);
    system->start();
}

void Application::boot_firmware() {
    // window_type = WindowType::Game;
    // BootMode old_boot_mode = m_system.boot_mode();

    // m_system.set_boot_mode(BootMode::Firmware);
    // m_system.boot();
    // m_system.set_boot_mode(old_boot_mode);

    // set_fullscreen(Settings::Get().fullscreen_on_game_launch);
}