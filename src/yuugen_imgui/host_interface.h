#pragma once

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Keyboard.hpp>
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

    bool show_cartridge_window = false;
    bool show_interrupts_window = false;
    bool show_graphics_window = false;
    bool show_console_window = false;

    char prompt[15];

    sf::Uint8* framebuffer;

    sf::View view;

    void Loop();
    void UpdateTextures();
    void HandleInput();

    void SetupStyle();

    void SetToContentSize();

    sf::Texture texture;
};