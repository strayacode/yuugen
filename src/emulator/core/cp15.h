#pragma once
#include <emulator/common/types.h>

// TODO: handle dtcm and itcm virtual sizes

class CP15 {
public:
    u32 read_reg(u32 cn, u32 cm, u32 cp);
    bool get_dtcm_enabled();
    bool get_itcm_enabled();

private:
    u32 control_register;



};