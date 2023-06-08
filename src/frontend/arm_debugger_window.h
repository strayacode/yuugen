#pragma once

#include <memory>
#include "arm/cpu.h"
#include "arm/arch.h"
#include "frontend/font_database.h"

class ARMDebuggerWindow {
public:
    ARMDebuggerWindow(arm::CPU& cpu, arm::Arch arch, FontDatabase& font_database);

    void render();
    void set_visible(bool visible) { this->visible = visible; }

    bool* get_visible_pointer() { return &visible; }

private:
    bool visible = true;
    arm::CPU& cpu;
    arm::Arch arch;
    FontDatabase& font_database;
};