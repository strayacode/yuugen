#include "Common/Settings.h"
#include "host_interface.h"

HostInterface::HostInterface() : 
    core([this](float fps) {
        UpdateTitle(fps);
    }) {
    
}

bool HostInterface::initialise() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) > 0) {
        log_warn("error initialising SDL!");
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
    io.Fonts->AddFontFromFileTTF("../data/fonts/roboto-regular.ttf", 13.0f);
    SetupStyle();
    SDL_GetWindowSize(window, &window_width, &window_height);

    top_screen.initialise(256, 192);
    bottom_screen.initialise(256, 192);
    games_list.initialise();

    return true;
}

void HostInterface::Run() {
    while (running) {
        HandleInput();
        render();
    }
}

void HostInterface::Shutdown() {
    top_screen.destroy();
    bottom_screen.destroy();

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
        } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            window_width = event.window.data1;
            window_height = event.window.data2;
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
            case SDLK_ESCAPE:
                set_fullscreen(false);
                break;
            }
        } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
            int x = ((event.button.x - center_pos) / scaled_dimensions.x) * 256;
            int y = ((event.button.y - scaled_dimensions.y) / scaled_dimensions.y) * 192;

            if ((x >= 0 && x < 256) && (y >= 0 && y < 192) && event.button.button == SDL_BUTTON_LEFT) {
                // only do a touchscreen event if it occurs in the bottom screen
                bool button_pressed = event.type == SDL_MOUSEBUTTONDOWN;
                core.system.input.SetTouch(button_pressed);
                core.system.input.SetPoint(x, y);
            }
        }
    }
}

void HostInterface::UpdateTitle(float fps) {
    char window_title[60];
    float percent_usage = (fps / 60.0f) * 100;
    
    snprintf(window_title, 60, "yuugen | %s | %0.2f FPS | %0.2f%s", core.system.GetCPUCoreType().c_str(), fps, percent_usage, "%");
    SDL_SetWindowTitle(window, window_title);
}

void HostInterface::render_screens() {
    ImGuiIO& io = ImGui::GetIO();
    
    SDL_ShowCursor(Settings::Get().hide_cursor && (fullscreen || (io.MousePos.y > menubar_height)) ? SDL_DISABLE : SDL_ENABLE);

    top_screen.render(core.system.gpu.get_framebuffer(Screen::Top));
    bottom_screen.render(core.system.gpu.get_framebuffer(Screen::Bottom));

    const double scale_x = (double)window_width / 256;
    const double scale_y = (double)window_height / 384;
    const double scale = scale_x < scale_y ? scale_x : scale_y;

    scaled_dimensions = ImVec2(256 * scale, 192 * scale);

    center_pos = ((double)window_width - scaled_dimensions.x) / 2;
    
    ImGui::GetBackgroundDrawList()->AddImage(
        (void*)(intptr_t)top_screen.get_texture(),
        ImVec2(center_pos, fullscreen ? 0 : menubar_height),
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

void HostInterface::render_menubar() {
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
                window_type = WindowType::GamesList;
                reset_title();
                core.SetState(State::Idle);
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
                window_type = WindowType::GamesList;
                reset_title();
                core.SetState(State::Idle);
            }

            if (ImGui::MenuItem("Restart")) {
                core.SetState(State::Idle);
                core.SetState(State::Running);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Settings")) {
                settings_window = !settings_window;
                // window_type = WindowType::Settings;
                // reset_title();

            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Cartridge", nullptr, cartridge_window)) { 
                cartridge_window = !cartridge_window; 
            }

            if (ImGui::MenuItem("ARM7", nullptr, arm7_window)) { 
                arm7_window = !arm7_window; 
            }

            if (ImGui::MenuItem("ARM9", nullptr, arm9_window)) { 
                arm9_window = !arm9_window; 
            }

            if (ImGui::MenuItem("Scheduler", nullptr, scheduler_window)) { 
                scheduler_window = !scheduler_window; 
            }

            if (ImGui::MenuItem("DMA", nullptr, dma_window)) { 
                dma_window = !dma_window; 
            }

            if (ImGui::MenuItem("Demo Window", nullptr, demo_window)) { 
                demo_window = !demo_window; 
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Toggle Fullscreen", nullptr, fullscreen)) {
                set_fullscreen(!fullscreen);
            }

            if (ImGui::MenuItem("Set To DS Screen Size")) {
                SDL_SetWindowSize(window, 512, 768);
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

void HostInterface::SetupStyle() {
    ImGui::GetStyle().WindowBorderSize = 0.0f;
    ImGui::GetStyle().PopupBorderSize = 0.0f;
    ImGui::GetStyle().ChildBorderSize = 0.0f;
    ImGui::GetStyle().WindowRounding = 5.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().ScrollbarSize = 10.0f;
    ImGui::GetStyle().ScrollbarRounding = 12.0f;
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
    ImGui::GetStyle().Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
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

void HostInterface::ARMWindow(CPUArch arch) {
    std::string name = arch == CPUArch::ARMv5 ? "ARM9" : "ARM7";
    int index = arch == CPUArch::ARMv5 ? 1 : 0;

    ImGui::Begin(name.c_str());
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("ARMTabs", tab_bar_flags)) {
        if (ImGui::BeginTabItem("Registers")) {
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Disassembly")) {
            if (ImGui::Button("+")) {
                disassembly_size++;
            }

            ImGui::SameLine();

            if (ImGui::Button("-")) {
                disassembly_size--;
            }

            ImGui::SameLine();

            if (disassembly_size < 0) {
                disassembly_size = 0;
            }

            ImGui::Text("Number of Instructions: %d", disassembly_size);

            if (core.GetState() != State::Idle) {
                int increment = core.system.cpu_core[index].IsARM() ? 4 : 2;
                u32 pc = core.system.cpu_core[index].regs.r[15];
                u32 addr = pc - ((disassembly_size - 1) / 2) * increment;
                
                if (core.system.cpu_core[index].IsARM()) {
                    for (int i = 0; i < disassembly_size; i++) {
                        u32 instruction = core.system.cpu_core[index].ReadWord(addr);
                        if (addr == pc) {
                            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08X:", addr);
                            ImGui::SameLine(67);
                            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08X", instruction);
                            ImGui::SameLine(125);
                            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", disassembler.disassemble_arm(instruction).c_str());
                        } else {
                            ImGui::Text("%08X:", addr);
                            ImGui::SameLine(67);
                            ImGui::Text("%08X", instruction);
                            ImGui::SameLine(125);
                            ImGui::Text("%s", disassembler.disassemble_arm(instruction).c_str());
                        }
                        
                        addr += increment;
                    }
                } else {
                    for (int i = 0; i < disassembly_size; i++) {
                        u16 instruction = core.system.cpu_core[index].ReadHalf(addr);
                        if (addr == pc) {
                            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08X:", addr);
                            ImGui::SameLine(67);
                            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08X", instruction);
                            ImGui::SameLine(125);
                            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", disassembler.disassemble_thumb(instruction).c_str());
                        } else {
                            ImGui::Text("%08X:", addr);
                            ImGui::SameLine(67);
                            ImGui::Text("%08X", instruction);
                            ImGui::SameLine(125);
                            ImGui::Text("%s", disassembler.disassemble_thumb(instruction).c_str());
                        }

                        addr += increment;
                    }
                }
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
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

void HostInterface::DMAWindow() {
    ImGui::Begin("DMA");

    for (int i = 0; i < 4; i++) {
        ImGui::Text("DMA7 Channel %d", i);
        ImGui::Text("Source Address: %08x", core.system.dma[0].channel[i].source);
        ImGui::Text("Destination Address: %08x", core.system.dma[0].channel[i].destination);
    }

    for (int i = 0; i < 4; i++) {
        ImGui::Text("DMA9 Channel %d", i);
        ImGui::Text("Source Address: %08x", core.system.dma[1].channel[i].source);
        ImGui::Text("Destination Address: %08x", core.system.dma[1].channel[i].destination);
    }

    ImGui::End();
}

void HostInterface::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (core.GetState() == State::Idle || !fullscreen) {
        render_menubar();
    }

    switch (window_type) {
    case WindowType::GamesList:
        render_games_list_window();
        break;
    case WindowType::Game:
        render_screens();
        break;
    // case WindowType::Settings:
    //     render_settings_window();
    //     break;
    }

    if (cartridge_window) {
        CartridgeWindow();
    }

    if (arm7_window) {
        ARMWindow(CPUArch::ARMv4);
    }

    if (arm9_window) {
        ARMWindow(CPUArch::ARMv5);
    }

    if (scheduler_window) {
        SchedulerWindow();
    }

    if (dma_window) {
        DMAWindow();
    }

    if (demo_window) {
        ImGui::ShowDemoWindow();
    }

    if (settings_window) {
        render_settings_window();
    }

    ImGui::Render();
    glViewport(0, 0, 1280, 720);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

void HostInterface::begin_fullscreen_window(const char *name, float padding) {
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

void HostInterface::end_fullscreen_window() {
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

void HostInterface::render_games_list_window() {
    begin_fullscreen_window("Games List");
    
    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_BordersOuterV
        | ImGuiTableFlags_SizingStretchProp;

    float min_row_height = 20.0f;

    if (ImGui::BeginTable("table_advanced", 4, flags)) {
        ImGui::TableSetupColumn("Title");
        ImGui::TableSetupColumn("Region");
        ImGui::TableSetupColumn("Gamecode");
        ImGui::TableSetupColumn("Size");
        ImGui::TableHeadersRow();

        for (int row = 0; row < games_list.get_num_entries(); row++) {
            Common::GamesList::Entry entry = games_list.get_entry(row);

            ImGui::TableNextRow(ImGuiTableRowFlags_None);
            ImGui::PushID(row);
            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
            ImGui::TableSetColumnIndex(0);

            if (ImGui::Selectable(entry.file_name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, min_row_height))) {
                boot_game(entry.path.c_str());
            }

            ImGui::TableSetColumnIndex(1);

            if (ImGui::Selectable(entry.region.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, min_row_height))) {
                boot_game(entry.path.c_str());
            }

            ImGui::TableSetColumnIndex(2);

            if (ImGui::Selectable(entry.game_code.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, min_row_height))) {
                boot_game(entry.path.c_str());
            }

            ImGui::TableSetColumnIndex(3);

            if (ImGui::Selectable(entry.size.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, min_row_height))) {
                boot_game(entry.path.c_str());
            }
            
            ImGui::PopStyleVar();
            ImGui::PopID();
        }
        
        ImGui::EndTable();
    }
    
    end_fullscreen_window();
}

void HostInterface::render_settings_window() {
    ImGui::Begin("Settings");
    ImGui::Text("Video Settings");
    ImGui::Checkbox("Fullscreen on Game Launch", &Settings::Get().fullscreen_on_game_launch);
    ImGui::Checkbox("Hide Cursor in Game", &Settings::Get().hide_cursor);
    ImGui::Checkbox("Do 2D Rendering on Separate Thread (Experimental)", &Settings::Get().threaded_2d);
    ImGui::Separator();
    ImGui::End();
}

void HostInterface::reset_title() {
    SDL_SetWindowTitle(window, "yuugen");
}

void HostInterface::boot_game(std::string path) {
    window_type = WindowType::Game;
    core.BootGame(path);
    set_fullscreen(Settings::Get().fullscreen_on_game_launch);
}

void HostInterface::boot_firmware() {
    window_type = WindowType::Game;
    core.BootFirmware();
    set_fullscreen(Settings::Get().fullscreen_on_game_launch);
}

void HostInterface::set_fullscreen(bool value) {
    fullscreen = value;
    SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}