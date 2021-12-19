#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "common/log.h"

class Config {
public:
    Config(const Config& config) = delete;

    static Config& GetInstance() {
        return instance;
    }

    bool nds = true;
    bool direct_boot = true;

private:
    Config() {};

    static Config instance;
};