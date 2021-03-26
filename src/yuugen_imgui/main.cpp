#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window.hpp>
#include <stdlib.h>
#include <string.h>

int main() {
    sf::RenderWindow window(sf::VideoMode(640, 480), "yuugen");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_MenuBar           |
                    ImGuiWindowFlags_NoMove            |
                    // ImGuiWindowFlags_NoResize          |
                    ImGuiWindowFlags_NoScrollbar       |
                    ImGuiWindowFlags_NoScrollWithMouse |
                    ImGuiWindowFlags_NoBackground;

    sf::Clock deltaClock;

    sf::Uint8* pixels = new sf::Uint8[256 * 192 * 4];
    memset(pixels, 0xFF, 256 * 192 * 4);
    sf::Texture texture;
    texture.create(256, 192);
    texture.update(pixels);
    sf::Sprite sprite(texture);

    while (window.isOpen()) {
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
                        printf("open rom\n");
                    }

                    if (ImGui::MenuItem("Quit")) {
                        window.close();
                    }
                    ImGui::EndMenu();
                }
                
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }
        


        

        // ImGui::Begin("Hello, world!");
        // ImGui::Button("Look at this pretty button");
        // ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        // ImGui::Text("Welcome to this Dear ImGui & SFML boilerplate.");
        // ImGui::End();

        window.clear();
        window.draw(sprite);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}