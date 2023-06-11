#pragma once

#include <memory>
#include "arm/cpu.h"
#include "arm/arch.h"
#include "core/hardware/irq.h"
#include "frontend/font_database.h"

namespace core {
    class System;
};

class ARMDebuggerWindow {
public:
    ARMDebuggerWindow(core::System& system, arm::CPU& cpu, core::IRQ& irq, arm::Arch arch, FontDatabase& font_database);

    void render();
    void set_visible(bool visible) { this->visible = visible; }

    bool* get_visible_pointer() { return &visible; }

private:
    bool visible = false;
    int disassembly_size = 25;
    core::System& system;
    arm::CPU& cpu;
    core::IRQ& irq;
    arm::Arch arch;
    FontDatabase& font_database;
};