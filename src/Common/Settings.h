#pragma once

class Settings {
public:
    Settings(const Settings& log_file) = delete;

    static Settings& Get() {
        return instance;
    }

    bool fullscreen_on_game_launch = false;
    bool hide_cursor = false;
    bool threaded_2d = false;

private:
    Settings() {};

    static Settings instance;
};