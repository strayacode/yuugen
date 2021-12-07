#pragma once

#include <string>
#include "common/types.h"

std::string DisassembleARMDataProcessing(u32 instruction) {
    std::string result = "";
    u8 opcode = (instruction >> 21) & 0xF;
    bool set_flags = (instruction >> 20) & 0x1;
    u8 rd = (instruction >> 12) & 0xF;
    u8 rn = (instruction >> 16) & 0xF;

    std::string regs = "";

    switch (opcode) {
    case 0x0:
        result += "and";
        break;
    case 0x1:
        result += "eor";
        break;
    case 0x2:
        result += "sub";
        regs += " r" + std::to_string(rd) + " ,r" + std::to_string(rn);
        break;
    case 0x3:
        result += "rsb";
        break;
    case 0x4:
        result += "add";
        break;
    case 0x5:
        result += "adc";
        break;
    case 0x6:
        result += "sbc";
        break;
    case 0x7:
        result += "rsc";
        break;
    case 0x8:
        result += "tst";
        break;
    case 0x9:
        result += "teq";
        break;
    case 0xA:
        result += "cmp";
        break;
    case 0xB:
        result += "cmn";
        break;
    case 0xC:
        result += "orr";
        break;
    case 0xD:
        result += "mov";
        break;
    case 0xE:
        result += "bic";
        break;
    case 0xF:
        result += "mvn";
        break;
    }

    if (set_flags) {
        result += "s";
    }

    return result + regs;
}