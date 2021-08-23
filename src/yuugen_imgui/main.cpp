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

#include <memory>
#include "host_interface.h"

int main() {
    HostInterface host_interface;
    host_interface.Loop();

    return 0;
}