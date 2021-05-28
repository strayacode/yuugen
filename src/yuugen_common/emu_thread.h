#pragma once

#include <core/core.h>
#include <thread>

struct EmuThread {
    EmuThread(Core& core);
    void Start();
    void Pause();
    void Stop();
    void Run();

    Core& core;

    bool running = false;

    std::thread thread;
};