#include "Common/Settings.h"

Settings Settings::instance;

std::string Settings::get_screenshots_path() {
    return home_path + "/.config/yuugen/Screenshots/";
}

std::string Settings::get_user_path() {
    return home_path + "/.config/yuugen/";
}