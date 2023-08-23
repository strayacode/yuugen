#pragma once

#include "arm/cpu.h"
#include "core/hardware/irq.h"

class Application;

class CPUDebugger {
public:
    void render(Application& application, arm::CPU& cpu, core::IRQ& irq);
    void set_visible(bool visible) { this->visible = visible; }

    bool* get_visible_pointer() { return &visible; }

private:
    bool visible = false;
    int disassembly_size = 25;
};