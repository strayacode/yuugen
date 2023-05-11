#pragma once

#include <thread>
#include <array>
#include <chrono>
#include <ratio>
#include "core/config.h"
#include "core/arm7/arm7.h"
#include "core/arm9/arm9.h"
#include "core/hardware/cartridge/cartridge.h"
#include "core/video/video_unit.h"
#include "core/hardware/input.h"
#include "core/hardware/spu.h"
#include "core/hardware/dma.h"
#include "core/scheduler.h"

namespace core {

class System {
public:
    System();
    ~System();

    void reset();
    void start();
    void stop();
    void set_game_path(const std::string& game_path);
    void set_boot_mode(BootMode boot_mode);
    void write_wramcnt(u8 data);

    ARM7 arm7;
    ARM9 arm9;
    Cartridge cartridge;
    VideoUnit video_unit;
    Input input;
    SPU spu;
    DMA dma7;
    DMA dma9;
    Scheduler scheduler;
    std::array<u8, 0x400000> main_memory;
    std::array<u8, 0x8000> shared_wram;
    u8 wramcnt;

private:
    void run_thread();
    void run_frame();
    void direct_boot();
    void firmware_boot();

    Config config;
    std::thread thread;

    enum class ThreadState {
        Running,
        Idle,
    };

    ThreadState thread_state = ThreadState::Idle;

    using Frame = std::chrono::duration<int, std::ratio<1, 60>>;

    int frames;
    bool framelimiter = false;

    static constexpr int FPS_UPDATE_INTERVAL = 1000;
};

} // namespace core