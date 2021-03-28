#include "host_interface.h"

HostInterface::HostInterface() {
    core = std::make_unique<Core>();
    window.create(sf::VideoMode(800, 1000), "yuugen");
    texture.create(256, 384);
    framebuffer = new sf::Uint8[256 * 384 * 4];
}

void HostInterface::Loop() {
    file_dialog.SetTitle("Select ROM");
    file_dialog.SetTypeFilters({".nds"});
    file_dialog.SetWindowSize(480, 400);

    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    SetupStyle();

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoMove            |
                    ImGuiWindowFlags_NoResize          |
                    ImGuiWindowFlags_NoScrollbar       |
                    ImGuiWindowFlags_NoScrollWithMouse |
                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoSavedSettings |
                    // ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_AlwaysUseWindowPadding |
                    ImGuiWindowFlags_NoBackground;

    sf::Clock deltaClock;

    while (window.isOpen()) {
        if (core_running) {
            core->RunFrame();
            UpdateTextures();
        }

        HandleInput();

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
        
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load ROM")) {
                    file_dialog.Open();
                }

                if (ImGui::MenuItem("Quit")) {
                    window.close();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Fit to DS Screen Size")) {
                    SetToContentSize();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("Cartridge", nullptr, show_cartridge_window)) { 
                    show_cartridge_window = !show_cartridge_window; 
                }

                ImGui::EndMenu();
            }

            // TODO: add settings, debug and help
                
            ImGui::EndMainMenuBar();
        }

        sf::Vector2f window_dimensions(window.getSize());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("MainWindow", 0, window_flags);
        ImGui::Image(texture, ImVec2(512, 768));
        ImGui::SetWindowPos(ImVec2((window_dimensions.x - 514) / 2, (window_dimensions.y - 768 + 30) / 2), true);
        ImGui::End();
        ImGui::PopStyleVar();

        file_dialog.Display();
        if (file_dialog.HasSelected()) {
            // TODO: add member variable for the rom path
            core->SetRomPath(file_dialog.GetSelected().string().c_str());
            core->Reset();
            core->DirectBoot();
            core_running = true;
            file_dialog.ClearSelected();
        }
        

        if (show_cartridge_window) {
            ImGui::Begin("Cartridge");
            ImGui::Text("ARM9 rom_offset: %08x", core->cartridge.header.arm9_rom_offset);
            ImGui::Text("ARM9 rom_entrypoint: %08x", core->cartridge.header.arm9_entrypoint);
            ImGui::Text("ARM9 rom_ram_address: %08x", core->cartridge.header.arm9_ram_address);
            ImGui::Text("ARM9 rom_size: %08x", core->cartridge.header.arm9_size);
            ImGui::Text("ARM7 rom_offset: %08x", core->cartridge.header.arm7_rom_offset);
            ImGui::Text("ARM7 rom_entrypoint: %08x", core->cartridge.header.arm7_entrypoint);
            ImGui::Text("ARM7 rom_ram_address: %08x", core->cartridge.header.arm7_ram_address);
            ImGui::Text("ARM7 rom_size: %08x", core->cartridge.header.arm7_size);
            ImGui::End();
        }

        ImGui::Begin("Hello, world!");
        
        ImGui::Button("Look at this pretty button");
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Text("Welcome to this Dear ImGui & SFML boilerplate.");

        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}

void HostInterface::UpdateTextures() {
    // copy framebuffer data from core into the 2 textures
    // TODO: don't use 2 extra textures
    for (int i = 0; i < 192; i++) {
        for (int j = 0; j < 256; j++) {
            u32 pixel = (256 * i) + j;
            u32 data_a = core->gpu.engine_a.framebuffer[pixel];
            framebuffer[pixel * 4] = (data_a >> 16) & 0xFF;
            framebuffer[pixel * 4 + 1] = (data_a >> 8) & 0xFF;
            framebuffer[pixel * 4 + 2] = data_a & 0xFF;
            framebuffer[pixel * 4 + 3] = 0xFF;

            u32 data_b = core->gpu.engine_b.framebuffer[pixel];
            framebuffer[(256 * 192 + pixel) * 4] = (data_b >> 16) & 0xFF;
            framebuffer[(256 * 192 + pixel) * 4 + 1] = (data_b >> 8) & 0xFF;
            framebuffer[(256 * 192 + pixel) * 4 + 2] = data_b & 0xFF;
            framebuffer[(256 * 192 + pixel) * 4 + 3] = 0xFF;
        }
    }

    texture.update(framebuffer);
}

void HostInterface::HandleInput() {
    sf::Event event;
    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        switch (event.type) {
        case sf::Event::Closed:
            window.close();
            break;
        // case sf::Event::Resized:
        //     break;
        case sf::Event::KeyPressed: case sf::Event::KeyReleased:
            bool key_pressed = event.type == sf::Event::KeyPressed;
            switch (event.key.code) {
            case sf::Keyboard::D:
                // A
                core->input.HandleInput(BUTTON_A, key_pressed);
                break;
            case sf::Keyboard::S:
                // B
                core->input.HandleInput(BUTTON_B, key_pressed);
                break;
            // should handle X and Y later (not in keyinput)
            case sf::Keyboard::RShift:
                // select
                core->input.HandleInput(BUTTON_SELECT, key_pressed);
                break;
            case sf::Keyboard::Enter:
                // start
                core->input.HandleInput(BUTTON_START, key_pressed);
                break;
            case sf::Keyboard::Right:
                // right
                core->input.HandleInput(BUTTON_RIGHT, key_pressed);
                break;
            case sf::Keyboard::Left:
                // left 
                core->input.HandleInput(BUTTON_LEFT, key_pressed);
                break;
            case sf::Keyboard::Up:
                // up
                core->input.HandleInput(BUTTON_UP, key_pressed);
                break;
            case sf::Keyboard::Down:
                // down
                core->input.HandleInput(BUTTON_DOWN, key_pressed);
                break;
            case sf::Keyboard::E:
                // Button R
                core->input.HandleInput(BUTTON_R, key_pressed);
                break;
            case sf::Keyboard::W:
                // Button L
                core->input.HandleInput(BUTTON_L, key_pressed);
                break;
            }
            break;
        }
    }
}

void HostInterface::SetupStyle() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
}

void HostInterface::SetToContentSize() {
    window.setSize(sf::Vector2u(509, 768));
}