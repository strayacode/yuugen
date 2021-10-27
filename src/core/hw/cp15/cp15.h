#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>
#include <algorithm>

class System;

class CP15 {
public:
    CP15(System& system);

    void Reset();
    void DirectBoot();

    auto Read(u32 cn, u32 cm, u32 cp) -> u32;
    void Write(u32 cn, u32 cm, u32 cp, u32 data);

    auto GetITCMSize() -> u32;
    auto GetDTCMSize() -> u32;

    auto GetDTCMBase() -> u32;

    auto GetITCMWriteEnabled() -> bool;
    auto GetDTCMWriteEnabled() -> bool;

    auto GetITCMReadEnabled() -> bool;
    auto GetDTCMReadEnabled() -> bool;

    auto GetExceptionBase() -> u32;

    u8 itcm[0x8000] = {};
    u8 dtcm[0x4000] = {};

    u32 control_register;

    u32 dtcm_reg;
    u32 itcm_reg;

    u32 dtcm_base;

    // this is always set to 0 as itcm starts at 0
    u32 itcm_base;

    u32 dtcm_size;
    u32 itcm_size;

    System& system;
};