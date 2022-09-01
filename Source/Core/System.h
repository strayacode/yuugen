#pragma once

#include <array>
#include <string>
#include <memory>
#include "Core/ARM/ARM7/ARM7.h"
#include "Core/ARM/ARM9/ARM9.h"
#include "Core/Scheduler.h"
#include "Core/EmulatorThread.h"
#include "Core/Hardware/Cartridge/Cartridge.h"
#include "Core/Hardware/SPI.h"
#include "Core/Hardware/DMA.h"
#include "Core/Hardware/Input.h"
#include "Core/Hardware/IPC.h"
#include "Core/Hardware/Timers.h"
#include "Core/Hardware/SPU.h"
#include "Core/Hardware/RTC.h"
#include "Core/Hardware/MathsUnit.h"
#include "VideoCommon/VideoUnit.h"
#include "AudioCommon/AudioInterface.h"
#include "AudioCommon/SDLAudioInterface.h"

enum class BootMode {
    Direct,
    Firmware,
};

enum class State {
    Running,
    Paused,
    Idle,
};

class System {
public:
    System(UpdateFunction update_fps);

    void reset();
    void start();
    void shutdown();

    void direct_boot();
    void firmware_boot();
    void run_frame();

    // single step the whole system
    void single_step();
    
    void set_state(State state);
    void set_boot_mode(BootMode boot_mode);
    void boot(std::string game_path = "");
    void toggle_framelimiter();

    void write_haltcnt(u8 data);
    void write_wramcnt(u8 data);

    bool cartridge_access_rights();

    // get a reference to a cpu based on an id
    // (0 = arm7, 1 = arm9)
    inline CPUBase& cpu(int cpu) {
        return cpu ? arm9.cpu() : arm7.cpu();
    }

    bool framelimiter_enabled() { return m_emulator_thread.framelimiter_enabled(); }

    State state() { return m_state; }
    BootMode boot_mode() { return m_boot_mode; }

    VideoUnit video_unit;
    Scheduler scheduler;
    Cartridge cartridge;
    SPI spi;
    DMA dma[2];
    Input input;
    IPC ipc;
    Timers timers[2];
    SPU spu;
    RTC rtc;
    MathsUnit maths_unit;

    ARM7 arm7;
    ARM9 arm9;

    std::array<u8, 0x400000> main_memory;
    std::array<u8, 0x8000> shared_wram;

    u8 wramcnt;
    u8 powcnt2;
    u16 rcnt;
    u8 haltcnt;
    u16 exmemcnt;
    u8 postflg7;
    u8 postflg9;
    u32 biosprot;
    u16 siocnt;

private:
    BootMode m_boot_mode = BootMode::Direct;
    State m_state = State::Idle;
    EmulatorThread m_emulator_thread;
    std::shared_ptr<AudioInterface> m_audio_interface;

    std::string m_game_path;

    u64 frame_end_time = 0;
};
