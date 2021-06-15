#include "config.h"

Config::Config() {
    LoadConfig();
}

Config::~Config() {
    SaveConfig();
}

void Config::LoadConfig() {
    std::ifstream file("../yuugen.ini", std::ios::in);

    std::string line;
    while (std::getline(file, line)) {
        std::string::iterator end_pos = std::remove(line.begin(), line.end(), ' ');
        line.erase(end_pos, line.end());
        
        // now get the name field and its value
        int delimiter_pos = line.find("=");

        std::string setting_name = line.substr(0, delimiter_pos);
        std::string variable = line.substr(delimiter_pos + 1);

        if (variable == "true" || variable == "false") {
            // boolean
            // not the nicest code but whatever
            *(bool*)config[setting_name] = variable == "true" ? true : false;
        } else {
            // string
            *(std::string*)config[setting_name] = variable;
        }
    }
}

void Config::SaveConfig() {
    std::ofstream file("../yuugen.ini", std::ios::out);

    std::map<std::string, void*>::iterator it;

    for (it = config.begin(); it != config.end(); it++) {
        // not good
        if (it->first == "software_fastmem") {
            file << "software_fastmem = " << (software_fastmem ? "true" : "false") << "\n";
        } else if (it->first == "halt_optimisation") {
            file << "halt_optimisation = " << (halt_optimisation ? "true" : "false") << "\n";
        }
    }
}