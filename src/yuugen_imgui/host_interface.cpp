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

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoMove            |
                    ImGuiWindowFlags_NoResize          |
                    ImGuiWindowFlags_NoScrollbar       |
                    ImGuiWindowFlags_NoScrollWithMouse |
                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoBackground;

    sf::Clock deltaClock;
    
    
    // view.setSize(480, 640);
    // view.setCenter(view.getSize().x / 2, view.getSize().y / 2);
    // GetLetterboxView(480, 640);

    while (window.isOpen()) {
        if (core_running) {
            core->RunFrame();
            UpdateTextures();
            // const float lowest_scale_factor = GetLowestScaleFactor();

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
                    printf("resize window to fit ds screen\n");
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
        ImGui::Begin("MainWindow", 0, window_flags);
        ImGui::Image(texture, ImVec2(512, 768));
        ImGui::SetWindowPos(ImVec2((window_dimensions.x - 512) / 2, (window_dimensions.y - 768) / 2), true);
        ImGui::End();

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
            ImGui::End();
        }

        ImGui::Begin("Hello, world!");
        
        ImGui::Button("Look at this pretty button");
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Text("Welcome to this Dear ImGui & SFML boilerplate.");

        ImGui::End();


        
        // sprite.setScale(sf::Vector2f(1.0f, 1.0f)); // absolute scale factor
        // sprite.scale(sf::Vector2f(1.0f, 1.0f));
        window.clear();

        // window.draw(sprite);
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

        //     GetLetterboxView(event.size.width, event.size.height);
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

// void HostInterface::ScaleViewport() {
//     const sf::Vector2f window_dimensions(window.getSize());
//     printf("x: %f y: %f\n", window_dimensions.x, window_dimensions.y);
//     const float lowest_scale_factor;
//     if ((window_dimensions.x / 256) >= (window_dimensions.y / 384)) {
//         lowest_scale_factor = window_dimensions.x / 256;
//     } else {
//         lowest_scale_factor = window_dimensions.y / 384;
//     }
// }

void HostInterface::GetLetterboxView(int windowWidth, int windowHeight) {

    // Compares the aspect ratio of the window to the aspect ratio of the view,
    // and sets the view's viewport accordingly in order to archieve a letterbox effect.
    // A new view (with a new viewport set) is returned.

    float windowRatio = windowWidth / (float) windowHeight;
    float viewRatio = view.getSize().x / (float) view.getSize().y;
    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    bool horizontalSpacing = true;
    if (windowRatio < viewRatio)
        horizontalSpacing = false;

    // If horizontalSpacing is true, the black bars will appear on the left and right side.
    // Otherwise, the black bars will appear on the top and bottom.

    if (horizontalSpacing) {
        sizeX = viewRatio / windowRatio;
        posX = (1 - sizeX) / 2.f;
    }

    else {
        sizeY = windowRatio / viewRatio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport( sf::FloatRect(posX, posY, sizeX, sizeY) );
}