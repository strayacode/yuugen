#pragma once

#include <array>
#include <memory>
#include "common/types.h"
#include "arm/coprocessor.h"
#include "arm/cpu.h"

namespace nds {

class ARM9Memory;

class ARM9Coprocessor : public arm::Coprocessor {
public:
    ARM9Coprocessor(std::unique_ptr<arm::CPU>& cpu, ARM9Memory& memory);

    void reset() override;
    u32 read(u32 cn, u32 cm, u32 cp) override;
    void write(u32 cn, u32 cm, u32 cp, u32 value) override;
    u32 get_exception_base() override;
    bool has_side_effects(u32 cn, u32 cm, u32 cp) override;

    union Control {
        struct {
            bool mmu : 1;
            bool alignment_fault : 1;
            bool data_cache : 1;
            bool write_buffer : 1;
            bool exception_handling : 1;
            bool faults_26bit : 1;
            bool abort_model : 1;
            bool endian : 1;
            bool system_protection : 1;
            bool rom_protection : 1;
            u32 : 1;
            bool branch_prediction : 1;
            bool instruction_cache : 1;
            bool exception_vector : 1;
            bool cache_replacement : 1;
            bool pre_armv5 : 1;
            bool dtcm_enable : 1;
            bool dtcm_write_only : 1;
            bool itcm_enable : 1;
            bool itcm_write_only : 1;
            u32 : 2;
            bool unaligned_access : 1;
            bool extended_page_table : 1;
            u32 : 1;
            bool cpsr_on_exceptions : 1;
            u32 : 1;
            bool fiq_behaviour : 1;
            bool tex_remap : 1;
            bool force_ap : 1;
            u32 : 2;
        };

        u32 data;
    };

    Control control;
    std::array<u8, 0x8000> dtcm;
    std::array<u8, 0x4000> itcm;

private:
    union TCMControl {
        struct {
            u32 : 1;
            u32 size : 5;
            u32 : 6;
            u32 base : 20;
        };

        u32 data;
    };

    TCMControl dtcm_control;
    TCMControl itcm_control;
    std::unique_ptr<arm::CPU>& cpu;
    ARM9Memory& memory;
};

} // namespace nds