#include "core/arm/disassembler/disassembler.h"

std::string Disassembler::DisassembleARMInstruction(u32 instruction) {
    return "n/a";
}
std::string Disassembler::DisassembleThumbInstruction(u16 instruction) {
    return "n/a";
}

std::string Disassembler::UnimplementedInstruction(u32 instruction) {
    return "n/a";
}