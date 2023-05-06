#pragma once

#include <thread>
#include <array>
#include "core/config.h"
#include "arm7/arm7.h"
#include "arm9/arm9.h"
#include "core/hardware/cartridge/cartridge.h"
#include "core/video/video_unit.h"
#include "core/hardware/input.h"
#include "core/hardware/spu.h"
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
    Scheduler scheduler;
    std::array<u8, 0x400000> main_memory;
    std::array<u8, 0x8000> shared_wram;
    u8 wramcnt;

private:
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
};

} // namespace core