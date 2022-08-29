#include <chrono>
#include <ctime>
#include <mutex>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include "Common/format.h"
#include "Common/Log.h"
#include "Common/Settings.h"
#include "host_interface.h"

HostInterface::HostInterface() : 
    m_system([this](float fps) {
        if (m_system.state() == State::Running) {
            m_fps = fps;
        }
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
    io.Fonts->AddFontFromFileTTF("../Data/fonts/roboto-regular.ttf", 13.0f);
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2(2500.0f, 1600.0f);
    io.DisplayFramebufferScale = ImVec2(2500.0f, 1600.0f);

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
            case SDLK_SPACE:
                if (key_pressed && m_system.state() != State::Idle) {
                    toggle_pause();
                }

                break;
            case SDLK_d:
                m_system.input.HandleInput(BUTTON_A, key_pressed);
                break;
            case SDLK_s:
                m_system.input.HandleInput(BUTTON_B, key_pressed);
                break;
            case SDLK_RSHIFT:
                m_system.input.HandleInput(BUTTON_SELECT, key_pressed);
                break;
            case SDLK_RETURN:
                m_system.input.HandleInput(BUTTON_START, key_pressed);
                break;
            case SDLK_RIGHT:
                m_system.input.HandleInput(BUTTON_RIGHT, key_pressed);
                break;
            case SDLK_LEFT:
                m_system.input.HandleInput(BUTTON_LEFT, key_pressed);
                break;
            case SDLK_UP:
                m_system.input.HandleInput(BUTTON_UP, key_pressed);
                break;
            case SDLK_DOWN:
                m_system.input.HandleInput(BUTTON_DOWN, key_pressed);
                break;
            case SDLK_e:
                m_system.input.HandleInput(BUTTON_R, key_pressed);
                break;
            case SDLK_w:
                m_system.input.HandleInput(BUTTON_L, key_pressed);
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
                m_system.input.SetTouch(button_pressed);
                m_system.input.SetPoint(x, y);
            }
        }
    }
}

void HostInterface::render_screens() {
    ImGuiIO& io = ImGui::GetIO();
    
    SDL_ShowCursor(Settings::Get().hide_cursor && (fullscreen || (io.MousePos.y > menubar_height)) ? SDL_DISABLE : SDL_ENABLE);

    top_screen.render(m_system.video_unit.get_framebuffer(Screen::Top));
    bottom_screen.render(m_system.video_unit.get_framebuffer(Screen::Bottom));

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

void HostInterface::render_menu_bar() {
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
                m_system.set_state(State::Idle);
            }

            if (ImGui::MenuItem("Quit")) {
                running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulation")) {
            if (ImGui::BeginMenu("Boot Mode")) {
                bool direct_boot = m_system.boot_mode() == BootMode::Direct;
                bool firmware_boot = !direct_boot;

                if (ImGui::MenuItem("Direct", "", &direct_boot)) {
                    m_system.set_boot_mode(BootMode::Direct);
                }

                if (ImGui::MenuItem("Firmware", "", &firmware_boot)) {
                    m_system.set_boot_mode(BootMode::Firmware);
                }

                ImGui::EndMenu();
            }

            bool framelimiter_enabled = m_system.framelimiter_enabled();

            if (ImGui::MenuItem("Toggle Framelimiter", "", &framelimiter_enabled)) {
                if (framelimiter_enabled) {
                    osd.add_message("Framelimiter On");
                } else {
                    osd.add_message("Framelimiter Off");
                }
                
                m_system.toggle_framelimiter();
            }

            if (ImGui::MenuItem(m_system.state() == State::Running ? "Pause" : "Resume", NULL, false, m_system.state() != State::Idle)) {
                toggle_pause();
            }

            if (ImGui::MenuItem("Stop", NULL, false, m_system.state() != State::Idle)) {
                osd.add_message("Emulation Stopped");
                window_type = WindowType::GamesList;
                reset_title();
                m_system.set_state(State::Idle);
            }

            if (ImGui::MenuItem("Restart", NULL, false, m_system.state() != State::Idle)) {
                osd.add_message("Emulation Restarted");
                m_system.set_state(State::Idle);
                m_system.set_state(State::Running);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Settings")) {
                settings_window = !settings_window;
                // window_type = WindowType::Settings;
                // reset_title();

            }

            if (ImGui::MenuItem("Take Screenshot")) {
                take_screenshot();
                osd.add_message("Screenshot Saved!");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::BeginMenu("NDS", true)) {
                ImGui::MenuItem("ARM7", nullptr, &arm7_window);
                ImGui::MenuItem("ARM9", nullptr, &arm9_window);

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Demo Window", nullptr, &demo_window)) {
                ImGui::ShowDemoWindow();
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

        if (m_system.state() == State::Running && m_fps != 0.0f) {
            std::string fps_string = format("%.0f FPS | %.2f ms", m_fps, 1000.0f / m_fps);

            auto pos = window_width - ImGui::CalcTextSize(fps_string.c_str()).x - ImGui::GetStyle().ItemSpacing.x;

            ImGui::SetCursorPosX(pos);
            ImGui::Text("%s", fps_string.c_str());
        } else if (m_system.state() == State::Paused) {
            std::string fps_string = "Paused";

            auto pos = window_width - ImGui::CalcTextSize(fps_string.c_str()).x - ImGui::GetStyle().ItemSpacing.x;

            ImGui::SetCursorPosX(pos);
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", fps_string.c_str());
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

void HostInterface::CartridgeWindow() {
    ImGui::Begin("Cartridge");
    ImGui::Text("%s", m_system.cartridge.loader.header.game_title);
    ImGui::Text("ARM7");
    ImGui::Text("Offset: 0x%08x", m_system.cartridge.loader.GetARM7Offset());
    ImGui::Text("Entrypoint: 0x%08x", m_system.cartridge.loader.GetARM7Entrypoint());
    ImGui::Text("RAM Address: 0x%08x", m_system.cartridge.loader.GetARM7RAMAddress());
    ImGui::Text("Size: 0x%08x", m_system.cartridge.loader.GetARM7Size());
    ImGui::Text("ARM9");
    ImGui::Text("Offset: 0x%08x", m_system.cartridge.loader.GetARM9Offset());
    ImGui::Text("Entrypoint: 0x%08x", m_system.cartridge.loader.GetARM9Entrypoint());
    ImGui::Text("RAM Address: 0x%08x", m_system.cartridge.loader.GetARM9RAMAddress());
    ImGui::Text("Size: 0x%08x", m_system.cartridge.loader.GetARM9Size());
    ImGui::End();
}

void HostInterface::ARMWindow(Arch arch) {
    std::string name = arch == Arch::ARMv5 ? "ARM9" : "ARM7";
    int index = arch == Arch::ARMv5 ? 1 : 0;
    CPUBase& cpu = m_system.cpu(index);

    ImGui::Begin(name.c_str());
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
    if (ImGui::BeginTabBar("ARMTabs", tab_bar_flags)) {
        if (ImGui::BeginTabItem("Registers")) {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Watchpoints")) {
            static u32 watchpoint_addr = 0;
            ImGui::InputScalar("Watchpoint Address", ImGuiDataType_U32, &watchpoint_addr, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);

            if (ImGui::Button("Add")) {
                cpu.m_watchpoints.add(watchpoint_addr);
            }

            for (auto& watchpoint : cpu.m_watchpoints.get()) {
                ImGui::Text("%08x", watchpoint.addr);
            }

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

            if (m_system.state() != State::Idle) {
                int increment = m_system.cpu(index).is_arm() ? 4 : 2;
                u32 pc = m_system.cpu(index).m_gpr[15];
                u32 addr = pc - ((disassembly_size - 1) / 2) * increment;
                
                if (m_system.cpu(index).is_arm()) {
                    for (int i = 0; i < disassembly_size; i++) {
                        u32 instruction = arch == Arch::ARMv5 ? m_system.arm9.memory().read<u32>(addr) : m_system.arm7.memory().read<u32>(addr);
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
                        u32 instruction = arch == Arch::ARMv5 ? m_system.arm9.memory().read<u16>(addr) : m_system.arm7.memory().read<u16>(addr);
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

void HostInterface::DMAWindow() {
    ImGui::Begin("DMA");
    
    for (int i = 0; i < 4; i++) {
        ImGui::Text("DMA7 Channel %d", i);
        ImGui::Text("Source Address: %08x", m_system.dma[0].channel[i].source);
        ImGui::Text("Destination Address: %08x", m_system.dma[0].channel[i].destination);
    }

    for (int i = 0; i < 4; i++) {
        ImGui::Text("DMA9 Channel %d", i);
        ImGui::Text("Source Address: %08x", m_system.dma[1].channel[i].source);
        ImGui::Text("Destination Address: %08x", m_system.dma[1].channel[i].destination);
    }

    ImGui::End();
}

void HostInterface::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (m_system.state() == State::Idle || !fullscreen) {
        render_menu_bar();
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

    if (dma_window) {
        DMAWindow();
    }

    if (arm7_window) {
        ARMWindow(Arch::ARMv4);
    }

    if (arm9_window) {
        ARMWindow(Arch::ARMv5);
    }

    if (demo_window) {
        ImGui::ShowDemoWindow();
    }

    if (settings_window) {
        render_settings_window();
    }

    osd.render_messages();

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

    if (ImGui::BeginTable("Games List", 4, flags)) {
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
    ImGui::Separator();
    ImGui::Text("Video Settings");
    ImGui::Checkbox("Fullscreen on Game Launch", &Settings::Get().fullscreen_on_game_launch);
    ImGui::Checkbox("Hide Cursor in Game", &Settings::Get().hide_cursor);
    ImGui::Checkbox("Do 2D Rendering on Separate Thread (Experimental)", &Settings::Get().threaded_2d);

    const char* items[] = {"Nearest Neighbour", "Linear"};
    static int item_current = 0;
    int item_prev = item_current;
    ImGui::Combo("Texture Filtering", &item_current, items, IM_ARRAYSIZE(items));

    if (item_prev != item_current) {
        bool nearest_filtering = item_current == 0;

        top_screen.initialise(256, 192, nearest_filtering);
        bottom_screen.initialise(256, 192, nearest_filtering);
    }
    
    ImGui::Separator();
    ImGui::Text("Audio Settings");
    ImGui::SliderInt("Volume", &Settings::Get().volume, 0, 100);
    ImGui::End();
}

void HostInterface::reset_title() {
    SDL_SetWindowTitle(window, "yuugen");
}

void HostInterface::boot_game(std::string path) {
    window_type = WindowType::Game;
    m_system.boot(path);
    set_fullscreen(Settings::Get().fullscreen_on_game_launch);
}

void HostInterface::boot_firmware() {
    window_type = WindowType::Game;
    BootMode old_boot_mode = m_system.boot_mode();

    m_system.set_boot_mode(BootMode::Firmware);
    m_system.boot();
    m_system.set_boot_mode(old_boot_mode);

    set_fullscreen(Settings::Get().fullscreen_on_game_launch);
}

void HostInterface::set_fullscreen(bool value) {
    fullscreen = value;
    SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void HostInterface::take_screenshot() {
    // first combine the top and bottom framebuffer into a std::array
    const u32* top_framebuffer = m_system.video_unit.get_framebuffer(Screen::Top);
    const u32* bottom_framebuffer = m_system.video_unit.get_framebuffer(Screen::Bottom);
    std::array<u32, 256 * 192 * 2> screenshot_data;

    std::mutex screenshot_copy_mutex;

    screenshot_copy_mutex.lock();

    for (int i = 0; i < 256 * 192; i++) {
        screenshot_data[i] = top_framebuffer[i];
        screenshot_data[(256 * 192) + i] = bottom_framebuffer[i];
    }

    screenshot_copy_mutex.unlock();

    // next construct the path
    std::string path = Settings::Get().get_screenshots_path();
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string filename(40, '\0');

    std::strftime(&filename[0], filename.size(), "yuugen-%Y-%m-%d-%H-%M-%S.png", std::localtime(&now));
    
    path += filename;
    
    stbi_write_png(path.c_str(), 256, 384, 4, screenshot_data.data(), 256 * 4);
}

void HostInterface::toggle_pause() {
    if (m_system.state() == State::Running) {
        m_system.set_state(State::Paused);
        osd.add_message("Emulation Paused");
    } else {
        m_system.set_state(State::Running);
        osd.add_message("Emulation Resumed");
    }
}