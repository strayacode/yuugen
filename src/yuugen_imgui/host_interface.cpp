#include "host_interface.h"

HostInterface::HostInterface() {
    core = std::make_unique<Core>();
    window.create(sf::VideoMode(640, 480), "yuugen");
    top_texture.create(256, 192);
    bottom_texture.create(256, 192);
    top_framebuffer = new sf::Uint8[256 * 192 * 4];
    bottom_framebuffer = new sf::Uint8[256 * 192 * 4];
}

void HostInterface::Loop() {
    file_dialog.SetTitle("Select ROM");
    file_dialog.SetTypeFilters({".nds"});
    file_dialog.SetWindowSize(480, 400);

    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_MenuBar           |
                    ImGuiWindowFlags_NoMove            |
                    ImGuiWindowFlags_NoResize          |
                    ImGuiWindowFlags_NoScrollbar       |
                    ImGuiWindowFlags_NoScrollWithMouse |
                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoBackground;

    sf::Clock deltaClock;
    
    
    

    while (window.isOpen()) {
        if (core_running) {
            core->RunFrame();
            UpdateTextures();
            // // memset(top_framebuffer, 0x50, 256 * 192 * 4);
            // top_texture.update(top_framebuffer);
            // bottom_texture.update(bottom_framebuffer);
            // for (int i = 3980; i < 4000; i++) {
            //     printf("%02x ", top_framebuffer[i]);
            // }
        }

        
        

        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
        
        if (ImGui::Begin("MainWindow", 0, window_flags)) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Load ROM")) {
                        file_dialog.Open();
                    }

                    if (ImGui::MenuItem("Quit")) {
                        window.close();
                    }
                    ImGui::EndMenu();
                }
                
                ImGui::EndMenuBar();
            }
            
            ImGui::Image(top_texture);
            ImGui::Image(bottom_texture);
            ImGui::End();
        }
        

        file_dialog.Display();
        if (file_dialog.HasSelected()) {
            // TODO: add member variable for the rom path
            core->SetRomPath(file_dialog.GetSelected().string().c_str());
            core->Reset();
            core->DirectBoot();
            core_running = true;
            file_dialog.ClearSelected();
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
            top_framebuffer[pixel * 4] = (data_a >> 16) & 0xFF;
            top_framebuffer[pixel * 4 + 1] = (data_a >> 8) & 0xFF;
            top_framebuffer[pixel * 4 + 2] = data_a & 0xFF;
            top_framebuffer[pixel * 4 + 3] = 0xFF;

            u32 data_b = core->gpu.engine_b.framebuffer[pixel];
            bottom_framebuffer[pixel * 4] = (data_b >> 16) & 0xFF;
            bottom_framebuffer[pixel * 4 + 1] = (data_b >> 8) & 0xFF;
            bottom_framebuffer[pixel * 4 + 2] = data_b & 0xFF;
            bottom_framebuffer[pixel * 4 + 3] = 0xFF;
        }
    }

    top_texture.update(top_framebuffer);
    bottom_texture.update(bottom_framebuffer);
}

void HostInterface::HandleInput() {
    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        
    // }
}