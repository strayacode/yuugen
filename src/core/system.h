#pragma once

#include <string>

enum class CPUCoreType {
    Interpreter,
};

// a base class which has virtual methods that will be overriden by the gba and nds classes
class System {
public:
    virtual ~System() = default;
    virtual std::string GetSystem() = 0;
    virtual void Reset() = 0;
    
    // TODO: use a struct called BootParameters instead when we decide to add more
    // parameters
    virtual void Boot(bool direct) = 0;
    virtual void RunFrame() = 0;

    std::string GetGame() {
        return game_path;
    }

    void SetGamePath(std::string path) {
        game_path = path;
    }

    std::string game_path;
};