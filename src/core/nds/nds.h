#pragma once

#include "core/system.h"

class NDS : public System {
public:
    NDS();

    std::string GetSystem() override {
        return "Nintendo DS";
    }

    void Reset() override;

    // TODO: use a struct called BootParameters instead when we decide to add more
    // parameters
    void Boot(bool direct) override;
    void RunFrame() override;
    const u32* GetFramebuffer(int screen) override;
};
