#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "Common/Log.h"

struct ConfigEntry {
    std::string field;
    std::string string_value;
    int int_value;
};

class Config {
public:
    Config();

    void AddEntry(std::string field, std::string string_value);
    void AddEntry(std::string field, int int_value);
    void SetDefaultConfig();
    void LoadConfig();
    void SaveConfig();

    std::vector<ConfigEntry> entries;
};