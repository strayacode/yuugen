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
#include "core/hardware/ipc.h"
#include "core/hardware/maths_unit.h"
#include "core/hardware/rtc.h"
#include "core/hardware/spi.h"
#include "core/hardware/timers.h"
#include "core/hardware/wifi.h"
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

    u8 read_wramcnt() { return wramcnt; }
    void write_wramcnt(u8 value);
    void write_haltcnt(u8 value);

    u16 read_exmemcnt() { return exmemcnt; }
    void write_exmemcnt(u16 value, u32 mask);

    u8 read_postflg7() { return postflg7; }
    u8 read_postflg9() { return postflg9; }
    u16 read_rcnt() { return rcnt; }

    using UpdateCallback = common::Callback<void(f32)>;

    void set_update_callback(UpdateCallback update_callback) { this->update_callback = update_callback; }

    enum class State {
        Running,
        Paused,
        Idle,
    };

    State get_state() { return state; }

    ARM7 arm7;
    ARM9 arm9;
    Cartridge cartridge;
    VideoUnit video_unit;
    Input input;
    SPU spu;
    DMA dma7;
    DMA dma9;
    IPC ipc;
    MathsUnit maths_unit;
    RTC rtc;
    SPI spi;
    Timers timers7;
    Timers timers9;
    Wifi wifi;
    Scheduler scheduler;
    std::array<u8, 0x400000> main_memory;
    std::array<u8, 0x8000> shared_wram;
    u8 wramcnt;
    u8 haltcnt;
    u16 exmemcnt;
    u8 postflg7;
    u8 postflg9;
    u16 rcnt;

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
    State state = State::Idle;

    using Frame = std::chrono::duration<int, std::ratio<1, 60>>;

    int frames;
    bool framelimiter = false;
    UpdateCallback update_callback;

    static constexpr int FPS_UPDATE_INTERVAL = 1000;
};

} // namespace core