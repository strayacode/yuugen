#include <core/arm/interpreter/interpreter.h>
#include <common/lut_helpers.h>

template <u32 instruction>
static constexpr Instruction GetARMInstruction() {
    switch ((instruction >> 25) & 0x7) {
    case 0x5: {
        // b/bl/blx
        const bool link = instruction & (1 << 24);

        if ((instruction & 0xF0000000) != 0xF0000000) {
            return &Interpreter::ARMBranchLink<link>;
        } else {
            return &Interpreter::ARMBranchLinkExchange;
        }
    }
    default:
        return &Interpreter::UnimplementedInstruction;
    }
}

static constexpr auto StaticGenerateARMTable() -> std::array<Instruction, 4096> {
    std::array<Instruction, 4096> arm_lut = {};

    static_for<std::size_t, 0, 4096>([&](auto i) {
        arm_lut[i] = GetARMInstruction<((i & 0xFF0) << 16) | ((i & 0xF) << 4)>();
    }); 

    return arm_lut;
}

static constexpr auto StaticGenerateThumbTable() -> std::array<Instruction, 1024> {
    std::array<Instruction, 1024> thumb_lut = {};

    return thumb_lut;
}

void Interpreter::GenerateARMTable() {
    arm_lut = StaticGenerateARMTable();
}


void Interpreter::GenerateThumbTable() {
    thumb_lut = StaticGenerateThumbTable();
}