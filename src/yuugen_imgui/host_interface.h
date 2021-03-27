#pragma once

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
#include <string>
#include <util/types.h>


#include <core/core.h>
#include "addons/imfilebrowser.h"
#include <memory>
#include <array>

struct HostInterface {
    HostInterface();

    std::unique_ptr<Core> core;

    ImGui::FileBrowser file_dialog;
    sf::RenderWindow window;

    bool core_running = false;

    u8* top_framebuffer = new u8[256 * 192 * 4];
    u8* bottom_framebuffer = new u8[256 * 192 * 4];

    void Loop();
    void UpdateTextures();

    sf::Texture top_texture;
    sf::Texture bottom_texture;
    
    
};