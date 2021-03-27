#pragma once

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window.hpp>
// #include <SFML/Keyboard.hpp>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <util/types.h>


#include <core/core.h>
#include "addons/imfilebrowser.h"
#include <memory>
#include <vector>
#include <array>

struct HostInterface {
    HostInterface();

    std::unique_ptr<Core> core;

    ImGui::FileBrowser file_dialog;
    sf::RenderWindow window;

    bool core_running = false;

    sf::Uint8* top_framebuffer;
    sf::Uint8* bottom_framebuffer;

    void Loop();
    void UpdateTextures();
    void HandleInput();

    sf::Texture top_texture;
    sf::Texture bottom_texture;
    
    
};