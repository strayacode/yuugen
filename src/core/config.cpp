#include <core/config.h>

Config::Config() {
    LoadConfig();
}

void Config::AddEntry(std::string field, std::string string_value) {
    entries.push_back({field, string_value, 0});
}

void Config::AddEntry(std::string field, int int_value) {
    entries.push_back({field, "", int_value});
}

void Config::SetDefaultConfig() {
    AddEntry("arm9_bios_path", "../bios/bios9.bin");
    AddEntry("arm7_bios_path", "../bios/bios7.bin");
    AddEntry("firmware_path", "../firmware/firmware.bin");
}

void Config::LoadConfig() {
    // check if the config file has been created yet and if not use
    // the default settings
    std::fstream file("../yuugen.ini", std::ios::in | std::ios::out);

    if (!file) {
        SetDefaultConfig();
        return;
    }
}

void Config::SaveConfig() {

}