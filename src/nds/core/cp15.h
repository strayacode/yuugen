#pragma once
#include <nds/common/types.h>

class CP15 {
public:
    u32 read_reg(u32 cn, u32 cm, u32 cp);



private:
    u32 tcm_size;
    u32 control_register;



};