#pragma once
#include <emulator/common/types.h>

// TODO: handle dtcm and itcm virtual sizes

class CP15 {
public:
    u32 read_reg(u32 cn, u32 cm, u32 cp);
    void write_reg(u32 cn, u32 cm, u32 cp, u32 data);

    // helpers to check if tcm is enabled
    bool get_dtcm_enabled();
    bool get_itcm_enabled();

    // helpers to check the size of tcm
    u32 get_dtcm_size();
    u32 get_itcm_size();

    // helpers to check the base addr of tcm
    u32 get_dtcm_addr();

    void direct_boot();
private:
    u32 control_register;

    // these are changed on write
    u32 dtcm_base_addr = 0;

    // itcm base is always 0
    u32 itcm_base_addr = 0;
    u32 dtcm_size = 0;
    u32 itcm_size = 0;

    // these hold the raw data of base_addr and size
    u32 dtcm_reg;
    u32 itcm_reg;

    // quick sidenote: if dtcm and itcm overlap, itcm has priority

};