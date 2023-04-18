#pragma once

#include "core/system.h"
#include "core/nds/config.h"
#include "core/nds/hardware/cartridge/cartridge.h"

namespace core::nds {

class System : public core::System {
public:
    System(Config config);

    void run_frame() override;
    void reset() override;

private:
    Cartridge cartridge;
};

} // namespace core::nds