#pragma once
#include <dmg/core/sm83.h>


class DMG {
public:
    DMG();
    void run();
    bool running;
private:
    SM83 sm83;
};