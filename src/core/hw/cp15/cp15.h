#pragma once

#include "Common/Types.h"

class System;

class CP15 {
public:
    CP15(System& system);

    void Reset();
    void DirectBoot();

    u32 Read(u32 cn, u32 cm, u32 cp);
    void Write(u32 cn, u32 cm, u32 cp, u32 data);

    u32 GetITCMSize();
    u32 GetDTCMSize();
    u32 GetDTCMBase();
    u32 GetExceptionBase();

    bool GetITCMWriteEnabled();
    bool GetDTCMWriteEnabled();
    bool GetITCMReadEnabled();
    bool GetDTCMReadEnabled();

    u8 itcm[0x8000] = {};
    u8 dtcm[0x4000] = {};

    u32 control_register;
    u32 dtcm_reg;
    u32 itcm_reg;
    u32 dtcm_base;
    u32 dtcm_size;
    u32 itcm_size;

    System& system;
};