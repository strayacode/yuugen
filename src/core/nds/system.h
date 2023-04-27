#pragma once

#include <array>
#include "core/system.h"
#include "core/nds/config.h"
#include "core/nds/arm7/arm7.h"
#include "core/nds/arm9/arm9.h"
#include "core/nds/hardware/cartridge/cartridge.h"
#include "core/scheduler.h"

namespace core::nds {

class System : public core::System {
public:
    System(Config config);

    void run_frame() override;
    void reset() override;

    void write_wramcnt(u8 data);

    ARM7 arm7;
    ARM9 arm9;
    Cartridge cartridge;
    Scheduler scheduler;
    std::array<u8, 0x400000> main_memory;
    std::array<u8, 0x8000> shared_wram;
    u8 wramcnt;

private:
    void direct_boot();
};

} // namespace core::nds