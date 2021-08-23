#include "host_interface.h"
#include <iostream>


HostInterface::HostInterface() {
    window.create(sf::VideoMode(800, 1000), "yuugen");
    texture.create(256, 384);
    framebuffer = new sf::Uint8[256 * 384 * 4];

    core = std::make_unique<Core>([this](float fps) {
        UpdateTitle(fps);
    });
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
        if (core->GetState() == State::Running) {
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
                if (ImGui::MenuItem("Interrupts", nullptr, show_interrupts_window)) { 
                    show_interrupts_window = !show_interrupts_window; 
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
            core->SetRomPath(file_dialog.GetSelected().string());
            core->SetBootMode(BootMode::Direct);
            core->SetState(State::Running);
            file_dialog.ClearSelected();
        }
        

        if (show_cartridge_window) {
            ImGui::Begin("Cartridge");
            ImGui::Text("ARM9 rom_offset: %08x", core->hw.cartridge.header.arm9_rom_offset);
            ImGui::Text("ARM9 rom_entrypoint: %08x", core->hw.cartridge.header.arm9_entrypoint);
            ImGui::Text("ARM9 rom_ram_address: %08x", core->hw.cartridge.header.arm9_ram_address);
            ImGui::Text("ARM9 rom_size: %08x", core->hw.cartridge.header.arm9_size);
            ImGui::Text("ARM7 rom_offset: %08x", core->hw.cartridge.header.arm7_rom_offset);
            ImGui::Text("ARM7 rom_entrypoint: %08x", core->hw.cartridge.header.arm7_entrypoint);
            ImGui::Text("ARM7 rom_ram_address: %08x", core->hw.cartridge.header.arm7_ram_address);
            ImGui::Text("ARM7 rom_size: %08x", core->hw.cartridge.header.arm7_size);
            ImGui::End();
        }

        if (show_interrupts_window) {
            ImGui::Begin("Interrupts");
            
            // if (ImGui::CollapsingHeader("ARM9 Interrupts")) {
            //     bool arm9_ime_set = core->interrupt[1].IME & 0x1;
            //     bool arm9_vblank_irq_enable = core->interrupt[1].IE & 0x1;
            //     bool arm9_vblank_irq_request = core->interrupt[1].IF & 0x1;
            //     bool arm9_hblank_irq_enable = core->interrupt[1].IE & (1 << 1);
            //     bool arm9_hblank_irq_request = core->interrupt[1].IF & (1 << 1);
            //     bool arm9_lyc_irq_enable = core->interrupt[1].IE & (1 << 2);
            //     bool arm9_lyc_irq_request = core->interrupt[1].IF & (1 << 2);
            //     bool arm9_timer0_irq_enable = core->interrupt[1].IE & (1 << 3);
            //     bool arm9_timer0_irq_request = core->interrupt[1].IF & (1 << 3);
            //     bool arm9_timer1_irq_enable = core->interrupt[1].IE & (1 << 4);
            //     bool arm9_timer1_irq_request = core->interrupt[1].IF & (1 << 4);
            //     bool arm9_timer2_irq_enable = core->interrupt[1].IE & (1 << 5);
            //     bool arm9_timer2_irq_request = core->interrupt[1].IF & (1 << 5);
            //     bool arm9_timer3_irq_enable = core->interrupt[1].IE & (1 << 6);
            //     bool arm9_timer3_irq_request = core->interrupt[1].IF & (1 << 6);
            //     bool arm9_dma0_irq_enable = core->interrupt[1].IE & (1 << 7);
            //     bool arm9_dma0_irq_request = core->interrupt[1].IF & (1 << 7);
            //     bool arm9_dma1_irq_enable = core->interrupt[1].IE & (1 << 8);
            //     bool arm9_dma1_irq_request = core->interrupt[1].IF & (1 << 8);
            //     bool arm9_dma2_irq_enable = core->interrupt[1].IE & (1 << 9);
            //     bool arm9_dma2_irq_request = core->interrupt[1].IF & (1 << 9);
            //     bool arm9_dma3_irq_enable = core->interrupt[1].IE & (1 << 10);
            //     bool arm9_dma3_irq_request = core->interrupt[1].IF & (1 << 10);
            //     ImGui::Checkbox("IME", &arm9_ime_set);
            //     ImGui::Columns(2, nullptr, false);
            //     ImGui::SetColumnOffset(1, 28);
            //     ImGui::Text("IE");
            //     ImGui::Checkbox("", &arm9_vblank_irq_enable);
            //     ImGui::Checkbox("", &arm9_hblank_irq_enable);
            //     ImGui::Checkbox("", &arm9_lyc_irq_enable);
            //     ImGui::Checkbox("", &arm9_timer0_irq_enable);
            //     ImGui::Checkbox("", &arm9_timer1_irq_enable);
            //     ImGui::Checkbox("", &arm9_timer2_irq_enable);
            //     ImGui::Checkbox("", &arm9_timer3_irq_enable);
            //     ImGui::Checkbox("", &arm9_dma0_irq_enable);
            //     ImGui::Checkbox("", &arm9_dma1_irq_enable);
            //     ImGui::Checkbox("", &arm9_dma2_irq_enable);
            //     ImGui::Checkbox("", &arm9_dma3_irq_enable);
            //     ImGui::NextColumn();
            //     ImGui::Text("IF");
            //     ImGui::Checkbox("VBlank", &arm9_vblank_irq_request);
            //     ImGui::Checkbox("HBlank", &arm9_hblank_irq_request);
            //     ImGui::Checkbox("LYC", &arm9_lyc_irq_request);
            //     ImGui::Checkbox("Timer 0", &arm9_timer0_irq_request);
            //     ImGui::Checkbox("Timer 1", &arm9_timer1_irq_request);
            //     ImGui::Checkbox("Timer 2", &arm9_timer2_irq_request);
            //     ImGui::Checkbox("Timer 3", &arm9_timer3_irq_request);
            //     ImGui::Checkbox("DMA 0", &arm9_dma0_irq_request);
            //     ImGui::Checkbox("DMA 1", &arm9_dma1_irq_request);
            //     ImGui::Checkbox("DMA 2", &arm9_dma2_irq_request);
            //     ImGui::Checkbox("DMA 3", &arm9_dma3_irq_request);
            // }

            // if (ImGui::CollapsingHeader("ARM7 Interrupts")) {
            //     bool arm7_ime_set = core->hw.cpu_core[0].ime & 0x1;
            //     bool arm7_vblank_irq_enable = core->hw.cpu_core[0].ie & 0x1;
            //     bool arm7_vblank_irq_request = core->hw.cpu_core[0].irf & 0x1;
            //     bool arm7_hblank_irq_enable = core->hw.cpu_core[0].ie & (1 << 1);
            //     bool arm7_hblank_irq_request = core->hw.cpu_core[0].irf & (1 << 1);
            //     bool arm7_lyc_irq_enable = core->hw.cpu_core[0].ie & (1 << 2);
            //     bool arm7_lyc_irq_request = core->hw.cpu_core[0].irf & (1 << 2);
            //     bool arm7_timer0_irq_enable = core->hw.cpu_core[0].ie & (1 << 3);
            //     bool arm7_timer0_irq_request = core->hw.cpu_core[0].irf & (1 << 3);
            //     bool arm7_timer1_irq_enable = core->hw.cpu_core[0].ie & (1 << 4);
            //     bool arm7_timer1_irq_request = core->hw.cpu_core[0].irf & (1 << 4);
            //     bool arm7_timer2_irq_enable = core->hw.cpu_core[0].ie & (1 << 5);
            //     bool arm7_timer2_irq_request = core->hw.cpu_core[0].irf & (1 << 5);
            //     bool arm7_timer3_irq_enable = core->hw.cpu_core[0].ie & (1 << 6);
            //     bool arm7_timer3_irq_request = core->hw.cpu_core[0].irf & (1 << 6);
            //     bool arm7_dma0_irq_enable = core->hw.cpu_core[0].ie & (1 << 7);
            //     bool arm7_dma0_irq_request = core->hw.cpu_core[0].irf & (1 << 7);
            //     bool arm7_dma1_irq_enable = core->hw.cpu_core[0].ie & (1 << 8);
            //     bool arm7_dma1_irq_request = core->hw.cpu_core[0].irf & (1 << 8);
            //     bool arm7_dma2_irq_enable = core->hw.cpu_core[0].ie & (1 << 9);
            //     bool arm7_dma2_irq_request = core->hw.cpu_core[0].irf & (1 << 9);
            //     bool arm7_dma3_irq_enable = core->hw.cpu_core[0].ie & (1 << 10);
            //     bool arm7_dma3_irq_request = core->hw.cpu_core[0].irf & (1 << 10);
            //     ImGui::Checkbox("IME", &arm7_ime_set);
            //     ImGui::Columns(2, nullptr, false);
            //     ImGui::SetColumnOffset(1, 28);
            //     ImGui::Text("IE");
            //     ImGui::Checkbox("", &arm7_vblank_irq_enable);
            //     ImGui::Checkbox("", &arm7_hblank_irq_enable);
            //     ImGui::Checkbox("", &arm7_lyc_irq_enable);
            //     ImGui::Checkbox("", &arm7_timer0_irq_enable);
            //     ImGui::Checkbox("", &arm7_timer1_irq_enable);
            //     ImGui::Checkbox("", &arm7_timer2_irq_enable);
            //     ImGui::Checkbox("", &arm7_timer3_irq_enable);
            //     ImGui::Checkbox("", &arm7_dma0_irq_enable);
            //     ImGui::Checkbox("", &arm7_dma1_irq_enable);
            //     ImGui::Checkbox("", &arm7_dma2_irq_enable);
            //     ImGui::Checkbox("", &arm7_dma3_irq_enable);
            //     ImGui::NextColumn();
            //     ImGui::Text("IF");
            //     ImGui::Checkbox("VBlank", &arm7_vblank_irq_request);
            //     ImGui::Checkbox("HBlank", &arm7_hblank_irq_request);
            //     ImGui::Checkbox("LYC", &arm7_lyc_irq_request);
            //     ImGui::Checkbox("Timer 0", &arm7_timer0_irq_request);
            //     ImGui::Checkbox("Timer 1", &arm7_timer1_irq_request);
            //     ImGui::Checkbox("Timer 2", &arm7_timer2_irq_request);
            //     ImGui::Checkbox("Timer 3", &arm7_timer3_irq_request);
            //     ImGui::Checkbox("DMA 0", &arm7_dma0_irq_request);
            //     ImGui::Checkbox("DMA 1", &arm7_dma1_irq_request);
            //     ImGui::Checkbox("DMA 2", &arm7_dma2_irq_request);
            //     ImGui::Checkbox("DMA 3", &arm7_dma3_irq_request);
            // }

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
            u32 data_a = core->hw.gpu.engine_a.framebuffer[pixel];
            framebuffer[pixel * 4] = (data_a >> 16) & 0xFF;
            framebuffer[pixel * 4 + 1] = (data_a >> 8) & 0xFF;
            framebuffer[pixel * 4 + 2] = data_a & 0xFF;
            framebuffer[pixel * 4 + 3] = 0xFF;

            u32 data_b = core->hw.gpu.engine_b.framebuffer[pixel];
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
        case sf::Event::KeyPressed: case sf::Event::KeyReleased:
            bool key_pressed = event.type == sf::Event::KeyPressed;
            switch (event.key.code) {
            case sf::Keyboard::D:
                // A
                core->hw.input.HandleInput(BUTTON_A, key_pressed);
                break;
            case sf::Keyboard::S:
                // B
                core->hw.input.HandleInput(BUTTON_B, key_pressed);
                break;
            // should handle X and Y later (not in keyinput)
            case sf::Keyboard::RShift:
                // select
                core->hw.input.HandleInput(BUTTON_SELECT, key_pressed);
                break;
            case sf::Keyboard::Enter:
                // start
                core->hw.input.HandleInput(BUTTON_START, key_pressed);
                break;
            case sf::Keyboard::Right:
                // right
                core->hw.input.HandleInput(BUTTON_RIGHT, key_pressed);
                break;
            case sf::Keyboard::Left:
                // left 
                core->hw.input.HandleInput(BUTTON_LEFT, key_pressed);
                break;
            case sf::Keyboard::Up:
                // up
                core->hw.input.HandleInput(BUTTON_UP, key_pressed);
                break;
            case sf::Keyboard::Down:
                // down
                core->hw.input.HandleInput(BUTTON_DOWN, key_pressed);
                break;
            case sf::Keyboard::E:
                // Button R
                core->hw.input.HandleInput(BUTTON_R, key_pressed);
                break;
            case sf::Keyboard::W:
                // Button L
                core->hw.input.HandleInput(BUTTON_L, key_pressed);
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

void HostInterface::UpdateTitle(float fps) {
    window.setTitle("hi");
}