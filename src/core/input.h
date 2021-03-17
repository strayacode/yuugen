#pragma once

#include <util/types.h>

enum ButtonInput {
    BUTTON_A,
    BUTTON_B,
    BUTTON_SELECT,
    BUTTON_START,
    BUTTON_RIGHT,
    BUTTON_LEFT,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_R,
    BUTTON_L,
};

struct Input {
    u16 KEYINPUT;
    u16 EXTKEYIN;

    void Reset();
    void HandleInput(int button, bool pressed);
};