#pragma once

#include "Common/Types.h"
#include "Core/ARM/MMIO.h"

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

struct Point {
    int x;
    int y;
};

class Input {
public:
    void reset();
    void build_mmio(MMIO& mmio);

    void HandleInput(int button, bool pressed);
    void SetTouch(bool pressed);
    void SetPoint(int x, int y);
    bool TouchDown();

    u16 keyinput;
    u16 extkeyin;
    Point point;
};