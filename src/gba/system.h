#pragma once

#include <array>
#include <memory>
#include "common/types.h"
#include "common/system.h"
#include "common/scheduler.h"
#include "arm/cpu.h"
#include "arm/null_coprocessor.h"
#include "gba/memory.h"
#include "gba/hardware/cartridge.h"
#include "gba/hardware/irq.h"
#include "gba/hardware/input.h"
#include "gba/hardware/timers.h"
#include "gba/hardware/dma.h"
#include "gba/hardware/apu.h"
#include "gba/video/ppu.h"

namespace gba {

class System final : public common::System {
public:
    System();
    ~System();

    void reset() override;
    void run_frame() override;
    void set_audio_device(std::shared_ptr<common::AudioDevice> audio_device) override;
    std::vector<u32*> fetch_framebuffers() override;
    void select_cpu_backend(arm::BackendType backend_type, bool optimise);

    Memory memory;
    arm::NullCoprocessor cp14;
    std::unique_ptr<arm::CPU> cpu;
    common::Scheduler scheduler;
    Cartridge cartridge;
    PPU ppu;
    IRQ irq;
    Input input;
    Timers timers;
    DMA dma;
    APU apu;
    
private:
    void skip_bios();
};

} // namespace gba