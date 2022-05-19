#pragma once

#include <string>
#include <vector>
#include <chrono>

// a helper class for displaying on screen messages
class OnScreenDisplay {
public:
    OnScreenDisplay();

    void render_messages();
    void add_message(std::string text, int display_duration = 2000);

private:
    struct Message {
        std::string text;
        std::chrono::time_point<std::chrono::system_clock> timestamp;
        int display_duration = 0;
        int fade_duration = 1000;
    };

    void render_message(int index, Message& message, int x, int y, int time_passed);

    int padding_left = 10;
    int padding_top = 29;
    std::vector<Message> messages;
};