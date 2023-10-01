#pragma once

#include <array>
#include <memory>
#include "common/types.h"
#include "common/system.h"
#include "arm/cpu.h"
#include "gba/memory.h"
#include "gba/cp14.h"

namespace gba {

class System : public common::System {
public:
    System();
    ~System();

    void reset() override;
    void run_frame() override;
    void set_audio_device(std::shared_ptr<common::AudioDevice> audio_device) override;
    std::vector<u32*> fetch_framebuffers() override;
    
private:
    void skip_bios();

    Memory memory;
    CP14 cp14;
    std::unique_ptr<arm::CPU> cpu;
};

} // namespace gba