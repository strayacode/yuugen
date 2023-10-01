#pragma once

#include <array>
#include <memory>
#include "common/types.h"
#include "common/system.h"
#include "nds/arm7/arm7.h"
#include "nds/arm9/arm9.h"
#include "nds/hardware/cartridge/cartridge.h"
#include "nds/video/video_unit.h"
#include "nds/hardware/input.h"
#include "nds/hardware/spu.h"
#include "nds/hardware/dma.h"
#include "nds/hardware/ipc.h"
#include "nds/hardware/maths_unit.h"
#include "nds/hardware/rtc.h"
#include "nds/hardware/spi.h"
#include "nds/hardware/timers.h"
#include "nds/hardware/wifi.h"
#include "nds/scheduler.h"

namespace nds {

class System : public common::System {
public:
    System();
    ~System();

    void reset() override;
    void run_frame() override;
    void set_audio_device(std::shared_ptr<common::AudioDevice> audio_device) override;
    std::vector<u32*> fetch_framebuffers() override;
    void select_cpu_backend(arm::BackendType backend_type, bool optimise);
    
    u8 read_wramcnt() { return wramcnt; }
    void write_wramcnt(u8 value);
    void write_haltcnt(u8 value);

    u16 read_exmemcnt() { return exmemcnt; }
    void write_exmemcnt(u16 value, u32 mask);

    u16 read_exmemstat() { return exmemstat; }
    void write_exmemstat(u16 value, u32 mask);

    u16 read_rcnt() { return rcnt; }

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
    std::unique_ptr<std::array<u8, 0x400000>> main_memory;
    std::array<u8, 0x8000> shared_wram;
    u8 wramcnt;
    u8 haltcnt;
    u16 exmemcnt;
    u16 exmemstat;
    u16 rcnt;

private:
    void direct_boot();
    void firmware_boot();
};

} // namespace nds