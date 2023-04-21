#pragma once

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

    ARM7 arm7;
    ARM9 arm9;
    Cartridge cartridge;
    Scheduler scheduler;

private:
    void direct_boot();
};

} // namespace core::nds