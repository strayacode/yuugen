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
    SetupStyle();

    // initialise texture stuff
    glGenTextures(2, &textures[0]);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return true;
}

void HostInterface::Run() {
    while (running) {
        HandleInput();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        DrawMenubar();
        DrawScreen();

        if (cartridge_window) {
            CartridgeWindow();
        }

        if (arm_window) {
            ARMWindow();
        }

        if (gpu_window) {
            GPUWindow();
        }

        if (gpu_2d_window) {
            GPU2DWindow();
        }

        if (scheduler_window) {
            SchedulerWindow();
        }

        ImGui::Render();
        glViewport(0, 0, 1280, 720);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
}

void HostInterface::Shutdown() {
    glDeleteTextures(2, &textures[0]);

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
        } else if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
            bool key_pressed = event.type == SDL_KEYDOWN;
            switch (event.key.keysym.sym) {
            case SDLK_d:
                core.system.input.HandleInput(BUTTON_A, key_pressed);
                break;
            case SDLK_s:
                core.system.input.HandleInput(BUTTON_B, key_pressed);
                break;
            case SDLK_RSHIFT:
                core.system.input.HandleInput(BUTTON_SELECT, key_pressed);
                break;
            case SDLK_RETURN:
                core.system.input.HandleInput(BUTTON_START, key_pressed);
                break;
            case SDLK_RIGHT:
                core.system.input.HandleInput(BUTTON_RIGHT, key_pressed);
                break;
            case SDLK_LEFT:
                core.system.input.HandleInput(BUTTON_LEFT, key_pressed);
                break;
            case SDLK_UP:
                core.system.input.HandleInput(BUTTON_UP, key_pressed);
                break;
            case SDLK_DOWN:
                core.system.input.HandleInput(BUTTON_DOWN, key_pressed);
                break;
            case SDLK_e:
                core.system.input.HandleInput(BUTTON_R, key_pressed);
                break;
            case SDLK_w:
                core.system.input.HandleInput(BUTTON_L, key_pressed);
                break;
            }
        } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
            int x = event.button.x;
            int y = event.button.y - 192;
            
            if ((y >= 0) && event.button.button == SDL_BUTTON_LEFT) {
                // only do a touchscreen event if it occurs in the bottom screen
                bool button_pressed = event.type == SDL_MOUSEBUTTONDOWN;
                core.system.input.SetTouch(button_pressed);
                core.system.input.SetPoint(x, y);
            }
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

            if (ImGui::MenuItem("Boot Firmware")) {
                core.BootFirmware();
            }

            if (ImGui::MenuItem("Quit")) {
                running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulator")) {
            if (ImGui::BeginMenu("Boot Mode")) {
                bool direct_boot = core.GetBootMode() == BootMode::Direct;
                bool firmware_boot = !direct_boot;

                if (ImGui::MenuItem("Direct", "", &direct_boot)) {
                    core.SetBootMode(BootMode::Direct);
                }

                if (ImGui::MenuItem("Firmware", "", &firmware_boot)) {
                    core.SetBootMode(BootMode::Firmware);
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Toggle Framelimiter")) {
                core.SetState(State::Paused);
                core.ToggleFramelimiter();
                core.SetState(State::Running);
            }

            if (ImGui::MenuItem("Pause")) {
                if (core.GetState() == State::Running) {
                    core.SetState(State::Paused);
                } else {
                    core.SetState(State::Running);
                }
            }

            if (ImGui::MenuItem("Stop")) {
                core.SetState(State::Idle);
            }

            if (ImGui::MenuItem("Restart")) {
                core.SetState(State::Idle);
                core.SetState(State::Running);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Cartridge", nullptr, cartridge_window)) { 
                cartridge_window = !cartridge_window; 
            }

            if (ImGui::MenuItem("ARM", nullptr, arm_window)) { 
                arm_window = !arm_window; 
            }

            if (ImGui::MenuItem("GPU", nullptr, gpu_window)) { 
                gpu_window = !gpu_window; 
            }

            if (ImGui::MenuItem("GPU2D", nullptr, gpu_2d_window)) { 
                gpu_2d_window = !gpu_2d_window; 
            }

            if (ImGui::MenuItem("Scheduler", nullptr, scheduler_window)) { 
                scheduler_window = !scheduler_window; 
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Toggle Fullscreen", nullptr, fullscreen)) {
                fullscreen = !fullscreen;
                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    file_dialog.Display();
    if (file_dialog.HasSelected()) {
        core.BootGame(file_dialog.GetSelected().string());
        file_dialog.ClearSelected();
    }
}

void HostInterface::DrawScreen() {
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_BGRA, GL_UNSIGNED_BYTE, core.system.gpu.GetFramebuffer(Screen::Top));

    glBindTexture(GL_TEXTURE_2D, textures[1]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_BGRA, GL_UNSIGNED_BYTE, core.system.gpu.GetFramebuffer(Screen::Bottom));

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
    ImGui::Begin("Screen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    ImVec2 window_size = ImGui::GetContentRegionAvail();

    const double scale_x = window_size.x / 256;
    const double scale_y = window_size.y / 192;
    const double scale = scale_x < scale_y ? scale_x : scale_y;
    ImVec2 scaled_dimensions = ImVec2(256 * scale, 192 * scale);

    ImGui::Image((void*)(intptr_t)textures[0], scaled_dimensions);
    ImGui::Image((void*)(intptr_t)textures[1], scaled_dimensions);

    ImGui::End();
    ImGui::PopStyleVar();
}

void HostInterface::SetupStyle() {
    ImGui::GetStyle().WindowBorderSize = 0.0f;
    ImGui::GetStyle().PopupBorderSize = 0.0f;
    ImGui::GetStyle().ChildBorderSize = 0.0f;
    ImGui::GetStyle().WindowRounding = 8.0f;
    ImGui::GetStyle().FrameRounding = 4.0f;
    ImGui::GetStyle().PopupRounding = 6.0f;
    ImGui::GetStyle().ChildRounding = 6.0f;
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
    ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(0.140f, 0.140f, 0.140f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = ImVec4(0.160f, 0.273f, 0.632f, 1.000f);
}

void HostInterface::CartridgeWindow() {
    ImGui::Begin("Cartridge");
    ImGui::Text("%s", core.system.cartridge.loader.header.game_title);
    ImGui::Text("ARM7");
    ImGui::Text("Offset: 0x%08x", core.system.cartridge.loader.GetARM7Offset());
    ImGui::Text("Entrypoint: 0x%08x", core.system.cartridge.loader.GetARM7Entrypoint());
    ImGui::Text("RAM Address: 0x%08x", core.system.cartridge.loader.GetARM7RAMAddress());
    ImGui::Text("Size: 0x%08x", core.system.cartridge.loader.GetARM7Size());
    ImGui::Text("ARM9");
    ImGui::Text("Offset: 0x%08x", core.system.cartridge.loader.GetARM9Offset());
    ImGui::Text("Entrypoint: 0x%08x", core.system.cartridge.loader.GetARM9Entrypoint());
    ImGui::Text("RAM Address: 0x%08x", core.system.cartridge.loader.GetARM9RAMAddress());
    ImGui::Text("Size: 0x%08x", core.system.cartridge.loader.GetARM9Size());
    ImGui::End();
}

void HostInterface::ARMWindow() {
    ImGui::Begin("ARM");
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("ARMTabs", tab_bar_flags)) {
        if (ImGui::BeginTabItem("ARM7")) {
            if (ImGui::BeginTable("registers", 4)) {
                for (int i = 0; i < 4; i++) {
                    ImGui::TableNextRow();
                    for (int j = 0; j < 4; j++) {
                        ImGui::TableSetColumnIndex(j);
                        ImGui::Text("r%d: %08x", (i * 4) + j, core.system.cpu_core[0]->regs.r[(i * 4) + j]);
                    }
                }
                ImGui::EndTable();
            }

            ImGui::Text("cpsr: %08x", core.system.cpu_core[0]->regs.cpsr);
            ImGui::Text("State: %s", core.system.cpu_core[0]->halted ? "Halted" : "Running");
            ImGui::Text("T Bit: %s", (!(core.system.cpu_core[0]->regs.cpsr & (1 << 5))) ? "ARM" : "Thumb");
            ImGui::Text("Mode: %02x", core.system.cpu_core[0]->regs.cpsr & 0x1F);
            
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("ARM9")) {
            if (ImGui::BeginTable("registers", 4)) {
                for (int i = 0; i < 4; i++) {
                    ImGui::TableNextRow();
                    for (int j = 0; j < 4; j++) {
                        ImGui::TableSetColumnIndex(j);
                        ImGui::Text("r%d: %08x", (i * 4) + j, core.system.cpu_core[1]->regs.r[(i * 4) + j]);
                    }
                }
                ImGui::EndTable();
            }

            ImGui::Text("cpsr: %08x", core.system.cpu_core[1]->regs.cpsr);
            ImGui::Text("State: %s", core.system.cpu_core[1]->halted ? "Halted" : "Running");
            ImGui::Text("T Bit: %s", (!(core.system.cpu_core[1]->regs.cpsr & (1 << 5))) ? "ARM" : "Thumb");
            ImGui::Text("Mode: %02x", core.system.cpu_core[1]->regs.cpsr & 0x1F);
            
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void HostInterface::GPU2DWindow() {
    ImGui::Begin("GPU2D");
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

    const char* bg_modes[7] = {
        "Text/3D Text Text Text", "Text/3D Text Text Affine", "Text/3D Text Affine Affine", 
        "Text/3D Text Text Extended", "Text/3D Text Affine Extended", "Text/3D Text Extended Extended", 
        "3D - Large -",
    };

    const char* display_modes[4] = {
        "Display Off", "Graphics Display", "VRAM Display", "Main Memory Display"
    };

    if (ImGui::BeginTabBar("GPU2DTabs", tab_bar_flags)) {
        if (ImGui::BeginTabItem("Engine A")) {
            if (ImGui::CollapsingHeader("dispcnt", ImGuiTreeNodeFlags_None)) {
                ImGui::Text("dispcnt: %08x", core.system.gpu.engine_a.DISPCNT);
                ImGui::Text("BG Mode %s", bg_modes[core.system.gpu.engine_a.DISPCNT & 0x7]);
                ImGui::Text("BG0 2D/3D Selection %s", (core.system.gpu.engine_a.DISPCNT >> 3) & 0x1 ? "3D": "2D");
                ImGui::Text("Tile OBJ Mapping %s", (core.system.gpu.engine_a.DISPCNT >> 4) & 0x1 ? "2D": "1D");
                ImGui::Text("Bitmap OBJ 2D-Dimension %s", (core.system.gpu.engine_a.DISPCNT >> 5) & 0x1 ? "128x512": "256x256");
                ImGui::Text("Bitmap OBJ Mapping %s", (core.system.gpu.engine_a.DISPCNT >> 6) & 0x1 ? "2D": "1D");
                ImGui::Text("Forced Blank %s", (core.system.gpu.engine_a.DISPCNT >> 7) & 0x1 ? "true": "false");
                ImGui::Text("Display BG0 %s", (core.system.gpu.engine_a.DISPCNT >> 8) & 0x1 ? "On": "Off");
                ImGui::Text("Display BG1 %s", (core.system.gpu.engine_a.DISPCNT >> 9) & 0x1 ? "On": "Off");
                ImGui::Text("Display BG2 %s", (core.system.gpu.engine_a.DISPCNT >> 10) & 0x1 ? "On": "Off");
                ImGui::Text("Display BG3 %s", (core.system.gpu.engine_a.DISPCNT >> 11) & 0x1 ? "On": "Off");
                ImGui::Text("Display OBJ %s", (core.system.gpu.engine_a.DISPCNT >> 12) & 0x1 ? "On": "Off");
                ImGui::Text("Display Window 0 %s", (core.system.gpu.engine_a.DISPCNT >> 13) & 0x1 ? "On": "Off");
                ImGui::Text("Display Window 1 %s", (core.system.gpu.engine_a.DISPCNT >> 14) & 0x1 ? "On": "Off");
                ImGui::Text("Display OBJ Window %s", (core.system.gpu.engine_a.DISPCNT >> 15) & 0x1 ? "On": "Off");
                ImGui::Text("Display Mode %s", display_modes[(core.system.gpu.engine_a.DISPCNT >> 16) & 0x3]);
                ImGui::Text("VRAM Block %c", 'A' + ((core.system.gpu.engine_a.DISPCNT >> 18) & 0x3));
                ImGui::Text("Tile OBJ 1D-Boundary %d", (core.system.gpu.engine_a.DISPCNT >> 20) & 0x3);
                ImGui::Text("Bitmap OBJ 1D-Boundary %d", (core.system.gpu.engine_a.DISPCNT >> 22) & 0x1);
                ImGui::Text("OBJ Processing during H-Blank %s", (core.system.gpu.engine_a.DISPCNT >> 23) & 0x1 ? "true" : "false");
                ImGui::Text("Character Base %08x", ((core.system.gpu.engine_a.DISPCNT >> 24) & 0x7) * 0x10000);
                ImGui::Text("Screen Base %08x", ((core.system.gpu.engine_a.DISPCNT >> 27) & 0x7) * 0x10000);
                ImGui::Text("BG Extended Palettes %s", (core.system.gpu.engine_a.DISPCNT >> 30) & 0x1 ? "true" : "false");
                ImGui::Text("OBJ Extended Palettes %s", (core.system.gpu.engine_a.DISPCNT >> 31) & 0x1 ? "true" : "false");
            }

            switch (core.system.gpu.engine_a.DISPCNT & 0x7) {
            case 0x3: case 0x4:
                if (((core.system.gpu.engine_a.BGCNT[3] >> 7) & 0x1) == 0) {
                    ImGui::Text("BG3 Extended Mode rot/scal 16-bit bg map entries");
                } else {
                    if ((core.system.gpu.engine_a.BGCNT[3] >> 2) & 0x1) {
                        ImGui::Text("BG3 Extended Mode rot/scal 256 colour bitmap");
                    } else {
                        ImGui::Text("BG3 Extended Mode rot/scal direct colour bitmap");
                    }
                }
                
                break;
            case 0x5:
                if (((core.system.gpu.engine_a.BGCNT[2] >> 7) & 0x1) == 0) {
                    ImGui::Text("BG2 Extended Mode rot/scal 16-bit bg map entries");
                } else {
                    if ((core.system.gpu.engine_a.BGCNT[2] >> 2) & 0x1) {
                        ImGui::Text("BG2 Extended Mode rot/scal 256 colour bitmap");
                    } else {
                        ImGui::Text("BG2 Extended Mode rot/scal direct colour bitmap");
                    }
                }

                if (((core.system.gpu.engine_a.BGCNT[3] >> 7) & 0x1) == 0) {
                    ImGui::Text("BG3 Extended Mode rot/scal 16-bit bg map entries");
                } else {
                    if ((core.system.gpu.engine_a.BGCNT[3] >> 2) & 0x1) {
                        ImGui::Text("BG3 Extended Mode rot/scal 256 colour bitmap");
                    } else {
                        ImGui::Text("BG3 Extended Mode rot/scal direct colour bitmap");
                    }
                }
                
                break;
            }
            
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Engine B")) {
            if (ImGui::CollapsingHeader("dispcnt", ImGuiTreeNodeFlags_None)) {
                ImGui::Text("dispcnt: %08x", core.system.gpu.engine_b.DISPCNT);
                ImGui::Text("BG Mode %s", bg_modes[core.system.gpu.engine_b.DISPCNT & 0x7]);
                ImGui::Text("Tile OBJ Mapping %s", (core.system.gpu.engine_b.DISPCNT >> 4) & 0x1 ? "2D": "1D");
                ImGui::Text("Bitmap OBJ 2D-Dimension %s", (core.system.gpu.engine_b.DISPCNT >> 5) & 0x1 ? "128x512": "256x256");
                ImGui::Text("Bitmap OBJ Mapping %s", (core.system.gpu.engine_b.DISPCNT >> 6) & 0x1 ? "2D": "1D");
                ImGui::Text("Forced Blank %s", (core.system.gpu.engine_b.DISPCNT >> 7) & 0x1 ? "true": "false");
                ImGui::Text("Display BG0 %s", (core.system.gpu.engine_b.DISPCNT >> 8) & 0x1 ? "On": "Off");
                ImGui::Text("Display BG1 %s", (core.system.gpu.engine_b.DISPCNT >> 9) & 0x1 ? "On": "Off");
                ImGui::Text("Display BG2 %s", (core.system.gpu.engine_b.DISPCNT >> 10) & 0x1 ? "On": "Off");
                ImGui::Text("Display BG3 %s", (core.system.gpu.engine_b.DISPCNT >> 11) & 0x1 ? "On": "Off");
                ImGui::Text("Display OBJ %s", (core.system.gpu.engine_b.DISPCNT >> 12) & 0x1 ? "On": "Off");
                ImGui::Text("Display Window 0 %s", (core.system.gpu.engine_b.DISPCNT >> 13) & 0x1 ? "On": "Off");
                ImGui::Text("Display Window 1 %s", (core.system.gpu.engine_b.DISPCNT >> 14) & 0x1 ? "On": "Off");
                ImGui::Text("Display OBJ Window %s", (core.system.gpu.engine_b.DISPCNT >> 15) & 0x1 ? "On": "Off");
                ImGui::Text("Display Mode %s", display_modes[(core.system.gpu.engine_a.DISPCNT >> 16) & 0x3]);
                ImGui::Text("Tile OBJ 1D-Boundary %d", (core.system.gpu.engine_a.DISPCNT >> 20) & 0x3);
                ImGui::Text("OBJ Processing during H-Blank %s", (core.system.gpu.engine_a.DISPCNT >> 23) & 0x1 ? "true" : "false");

                // TODO: should these be here?
                ImGui::Text("Character Base %08x", ((core.system.gpu.engine_a.DISPCNT >> 24) & 0x7) * 0x10000);
                ImGui::Text("Screen Base %08x", ((core.system.gpu.engine_a.DISPCNT >> 27) & 0x7) * 0x10000);
                ImGui::Text("BG Extended Palettes %s", (core.system.gpu.engine_a.DISPCNT >> 30) & 0x1 ? "true" : "false");
                ImGui::Text("OBJ Extended Palettes %s", (core.system.gpu.engine_a.DISPCNT >> 31) & 0x1 ? "true" : "false");
            }

            switch (core.system.gpu.engine_b.DISPCNT & 0x7) {
            case 0x3: case 0x4:
                if (((core.system.gpu.engine_b.BGCNT[3] >> 7) & 0x1) == 0) {
                    ImGui::Text("BG3 Extended Mode rot/scal 16-bit bg map entries");
                } else {
                    if ((core.system.gpu.engine_b.BGCNT[3] >> 2) & 0x1) {
                        ImGui::Text("BG3 Extended Mode rot/scal 256 colour bitmap");
                    } else {
                        ImGui::Text("BG3 Extended Mode rot/scal direct colour bitmap");
                    }
                }
                
                break;
            case 0x5:
                if (((core.system.gpu.engine_b.BGCNT[2] >> 7) & 0x1) == 0) {
                    ImGui::Text("BG2 Extended Mode rot/scal 16-bit bg map entries");
                } else {
                    if ((core.system.gpu.engine_b.BGCNT[2] >> 2) & 0x1) {
                        ImGui::Text("BG2 Extended Mode rot/scal 256 colour bitmap");
                    } else {
                        ImGui::Text("BG2 Extended Mode rot/scal direct colour bitmap");
                    }
                }

                if (((core.system.gpu.engine_b.BGCNT[3] >> 7) & 0x1) == 0) {
                    ImGui::Text("BG3 Extended Mode rot/scal 16-bit bg map entries");
                } else {
                    if ((core.system.gpu.engine_b.BGCNT[3] >> 2) & 0x1) {
                        ImGui::Text("BG3 Extended Mode rot/scal 256 colour bitmap");
                    } else {
                        ImGui::Text("BG3 Extended Mode rot/scal direct colour bitmap");
                    }
                }
                
                break;
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void HostInterface::GPUWindow() {
    ImGui::Begin("GPU");
    if (ImGui::CollapsingHeader("powcnt1", ImGuiTreeNodeFlags_None)) {
        ImGui::Text("powcnt1: %04x", core.system.gpu.POWCNT1);
        ImGui::Text("Enable Both LCDs %s", core.system.gpu.POWCNT1 & 0x1 ? "true" : "false");
        ImGui::Text("Enable 2D Engine A %s", (core.system.gpu.POWCNT1 >> 1) & 0x1 ? "true" : "false");
        ImGui::Text("Enable 3D Rendering Engine %s", (core.system.gpu.POWCNT1 >> 2) & 0x1 ? "true" : "false");
        ImGui::Text("Enable 3D Geometry Engine %s", (core.system.gpu.POWCNT1 >> 3) & 0x1 ? "true" : "false");
        ImGui::Text("Enable 2D Engine B %s", (core.system.gpu.POWCNT1 >> 9) & 0x1 ? "true" : "false");
        ImGui::Text("Display Swap %s", (core.system.gpu.POWCNT1 >> 15) & 0x1 ? "Engine A Top / Engine B Bottom" : "Engine B Top / Engine A Bottom");
    }

    ImGui::End();
}

void HostInterface::SchedulerWindow() {
    ImGui::Begin("Scheduler");
    ImGui::Text("Current Time: %ld", core.system.scheduler.GetCurrentTime());
    for (Event event : core.system.scheduler.GetEvents()) {
        ImGui::Text("%s +%ld", event.type->name.c_str(), event.time - core.system.scheduler.GetCurrentTime());
    }
    ImGui::End();
}