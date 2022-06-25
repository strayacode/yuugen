#pragma once

#include <string>
#include "Common/FileSystem.h"

class Settings {
public:
    Settings(const Settings& settings) = delete;

    static Settings& Get() {
        return instance;
    }

    std::string get_screenshots_path();
    std::string get_user_path();

    bool fullscreen_on_game_launch = false;
    bool hide_cursor = false;
    bool threaded_2d = false;
    int volume = 100;

private:
    Settings() {
        home_path = FileSystem::get_home_path();
        FileSystem::create_directory_if_not_exists(get_user_path());
        FileSystem::create_directory_if_not_exists(get_screenshots_path());
    };

    std::string home_path = "";
    static Settings instance;
};