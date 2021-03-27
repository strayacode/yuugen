#include "host_interface.h"

HostInterface::HostInterface() {
    core = std::make_unique<Core>();
    window.create(sf::VideoMode(640, 480), "yuugen");
    top_texture.create(256, 192);
    bottom_texture.create(256, 192);
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
    memset(top_framebuffer, 0xFF, 256 * 192 * 4);
    top_texture.update(top_framebuffer);
    

    while (window.isOpen()) {
        if (core_running) {
            core->RunFrame();
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
    // for (int i = 0; i < 192; i++) {
    //     for (int j = 0; j < 256; j++) {
    //         u32 data = core->gpu.engine_a.framebuffer[(256 * i) + j];
    //         for (int k = 3; k >= 0; k--) {
    //             // top_framebuffer[((256 * i) + j) * 4 + k] = 0x20;
    //             top_framebuffer[((256 * i) + j) * 4 + k] = data & 0xFF;
    //             data >>= 8;
    //         }
    //     }
    // }

    // top_texture.update(core->gpu.engine_a.framebuffer);
    // top_texture.update(core->gpu.engine_b.framebuffer);
}