#pragma once

#include <common/types.h>
#include <common/log.h>
#include <string.h>

struct Core;

struct CP15 {
    CP15(Core* core);

    void Reset();
    void DirectBoot();

    u32 Read(u32 cn, u32 cm, u32 cp);
    void Write(u32 cn, u32 cm, u32 cp, u32 data);

    u32 GetITCMSize();
    u32 GetDTCMSize();

    u32 GetDTCMBase();

    bool GetITCMEnabled();
    bool GetDTCMEnabled();

    u32 GetExceptionBase();

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



    Core* core;
};