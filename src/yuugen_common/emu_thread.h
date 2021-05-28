#pragma once

#include <thread>
#include <chrono>
#include <ratio>
#include <stdio.h>
#include <core/core.h>

struct EmuThread {
    EmuThread(Core& core);
    ~EmuThread();
    void Start();
    void Run();
    void Stop();
    auto IsActive() -> bool;

    Core& core;

    bool running = false;
    bool framelimiter = false;

    int frames = 0;

    std::thread thread;

    using frame = std::chrono::duration<int, std::ratio<1, 60>>;
};