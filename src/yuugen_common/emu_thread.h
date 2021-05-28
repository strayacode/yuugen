#pragma once

#include <thread>
#include <mutex>
#include <stdio.h>
#include <core/core.h>

struct EmuThread {
    EmuThread(Core& core);

    Core& core;

    bool running = false;

    std::mutex run_mutex;

    // std::thread thread;
};