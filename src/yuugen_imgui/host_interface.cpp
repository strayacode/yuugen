#include "host_interface.h"
#include <iostream>

HostInterface::HostInterface() : 
    core([this](float fps) {
        UpdateTitle(fps);
    }) {
    
}

auto HostInterface::Initialise() -> bool {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        log_warn("error initialising SDL!");
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
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
    io.Fonts->AddFontFromFileTTF("../data/fonts/roboto-regular.ttf", 15.0f);

    clear_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

    core.SetAudioInterface(audio_interface);

    return true;
}

void HostInterface::Run() {
    while (running) {
        HandleInput();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        DrawMenubar();

        ImGui::Begin("Hello, world!");
        ImGui::End();

        ImGui::Render();

        glViewport(0, 0, 1280, 720);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
}

void HostInterface::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void HostInterface::HandleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            running = false;
        }
    }
}

void HostInterface::UpdateTitle(float fps) {
    char window_title[40];
    snprintf(window_title, 40, "yuugen [%0.2f FPS | %0.2f ms]", fps, 1000.0 / fps);
    SDL_SetWindowTitle(window, window_title);
}

 void HostInterface::DrawMenubar() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load ROM")) {
                file_dialog.Open();
            }

            if (ImGui::MenuItem("Quit")) {
                running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulator")) {
            if (ImGui::MenuItem("Toggle Framelimiter")) {
                core.SetState(State::Paused);
                core.ToggleFramelimiter();
                core.SetState(State::Running);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Fit to DS Screen Size")) {
                // SetToContentSize();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            // if (ImGui::MenuItem("Cartridge", nullptr, show_cartridge_window)) { 
            //     show_cartridge_window = !show_cartridge_window; 
            // }
            // if (ImGui::MenuItem("Interrupts", nullptr, show_interrupts_window)) { 
            //     show_interrupts_window = !show_interrupts_window; 
            // }

            ImGui::EndMenu();
        }

        // TODO: add settings, debug and help
        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    file_dialog.Display();
    if (file_dialog.HasSelected()) {
        audio_interface.SetState(AudioState::Paused);
        core.SetState(State::Idle);
        core.SetRomPath(file_dialog.GetSelected().string());
        core.SetBootMode(BootMode::Direct);
        core.SetState(State::Running);
        audio_interface.SetState(AudioState::Playing);
        file_dialog.ClearSelected();
    }
 }