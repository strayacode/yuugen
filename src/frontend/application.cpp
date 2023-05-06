#include "common/logger.h"
#include "frontend/application.h"

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

    u32 window_flags =  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    window = SDL_CreateWindow("yuugen", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    gl_context = SDL_GL_CreateContext(window);

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../Data/fonts/roboto-regular.ttf", 13.0f);

    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    setup_style();
    SDL_GetWindowSize(window, &window_width, &window_height);

    top_screen.initialise(256, 192);
    bottom_screen.initialise(256, 192);

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
        } else if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
            bool key_pressed = event.type == SDL_KEYDOWN;
        }
    }
}

void Application::render_screens() {
    top_screen.render(system.video_unit.get_framebuffer(core::Screen::Top));
    bottom_screen.render(system.video_unit.get_framebuffer(core::Screen::Bottom));

    
    const u32* top = system.video_unit.get_framebuffer(core::Screen::Top);
    logger.warn("render screens %08x", top[0]);

    const double scale_x = static_cast<f64>(window_width) / 256;
    const double scale_y = static_cast<f64>(window_height) / 384;
    const double scale = scale_x < scale_y ? scale_x : scale_y;

    scaled_dimensions = ImVec2(256 * scale, 192 * scale);

    center_pos = (static_cast<f64>(window_width) - scaled_dimensions.x) / 2;
    
    ImGui::GetBackgroundDrawList()->AddImage(
        (void*)(intptr_t)top_screen.get_texture(),
        ImVec2(center_pos, menubar_height),
        ImVec2(center_pos + scaled_dimensions.x, scaled_dimensions.y),
        ImVec2(0, 0),
        ImVec2(1, 1),
        IM_COL32_WHITE
    );
    
    ImGui::GetBackgroundDrawList()->AddImage(
        (void*)(intptr_t)bottom_screen.get_texture(),
        ImVec2(center_pos, scaled_dimensions.y),
        ImVec2(center_pos + scaled_dimensions.x, scaled_dimensions.y * 2),
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

void Application::setup_style() {
    ImGui::GetStyle().WindowBorderSize = 1.0f;
    ImGui::GetStyle().PopupBorderSize = 0.0f;
    ImGui::GetStyle().ChildBorderSize = 0.0f;
    ImGui::GetStyle().GrabMinSize = 7.0f;
    ImGui::GetStyle().WindowRounding = 5.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;
    ImGui::GetStyle().ScrollbarSize = 10.0f;
    ImGui::GetStyle().ScrollbarRounding = 12.0f;
    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.50f, 0.50f);
    ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;
    ImGui::GetStyle().FramePadding = ImVec2(4.0f, 2.0f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.109f, 0.109f, 0.109f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.109f, 0.109f, 0.109f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGrip] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.349f, 0.500f, 0.910f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_Tab] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImVec4(0.186f, 0.186f, 0.188f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImVec4(0.231f, 0.231f, 0.251f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg] = ImVec4(0.210f, 0.210f, 0.210f, 1.000f);
}

void Application::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    render_menubar();
    render_screens();
    
    if (demo_window) {
        ImGui::ShowDemoWindow();
    }
    
    ImGui::Render();
    glViewport(0, 0, 1280, 720);
    glClearColor(0, 0, 0, 1);
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

void Application::boot_game(const std::string& path) {
    system.set_game_path(path);
    system.set_boot_mode(core::BootMode::Direct);
    system.start();
}

void Application::boot_firmware() {
    // window_type = WindowType::Game;
    // BootMode old_boot_mode = m_system.boot_mode();

    // m_system.set_boot_mode(BootMode::Firmware);
    // m_system.boot();
    // m_system.set_boot_mode(old_boot_mode);

    // set_fullscreen(Settings::Get().fullscreen_on_game_launch);
}