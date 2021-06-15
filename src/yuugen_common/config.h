#pragma once

#include <string>
#include <fstream>
#include <algorithm>
#include <map>
#include <typeinfo>

// for our config file we will use a .ini file
struct Config {
    Config();
    ~Config();
    
    void LoadConfig();
    void SaveConfig();

    bool software_fastmem = false;
    bool halt_optimisation = false;

    std::map<std::string, void*> config {{"software_fastmem", &software_fastmem}, {"halt_optimisation", &halt_optimisation}};
};