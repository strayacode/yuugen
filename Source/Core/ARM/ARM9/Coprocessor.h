#pragma once

#include <array>
#include "Common/Types.h"
#include "Core/ARM/CoprocessorBase.h"

class System;

class ARM9Coprocessor : public CoprocessorBase {
public:
    ARM9Coprocessor(System& system);

    void reset();
    void direct_boot();

    u32 get_exception_base() override;

    u32 read(u32 cn, u32 cm, u32 cp) override;
    void write(u32 cn, u32 cm, u32 cp, u32 data) override;

    u32 itcm_base();
    u32 itcm_size();
    u32 dtcm_base();
    u32 dtcm_size();

    bool itcm_is_readable();
    bool itcm_is_writeable();
    bool dtcm_is_readable();
    bool dtcm_is_writeable();

    std::array<u8, 0x8000>& itcm() { return m_itcm; }
    std::array<u8, 0x4000>& dtcm() { return m_dtcm; }

private:
    u32 m_control = 0;
    u32 m_itcm_control = 0;
    u32 m_dtcm_control = 0;

    std::array<u8, 0x8000> m_itcm;
    std::array<u8, 0x4000> m_dtcm;

    System& m_system;
};
