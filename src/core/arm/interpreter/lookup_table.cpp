#include <core/arm/interpreter/interpreter.h>
#include <common/lut_helpers.h>

template <u32 instruction>
static constexpr Instruction GetARMInstruction() {
    switch ((instruction >> 25) & 0x7) {
    case 0x0: {
        const bool set_flags = instruction & (1 << 20);
        const u8 opcode = (instruction >> 21) & 0xF;

        if ((instruction & 0x90) == 0x90) {
            // multiplies, extra load/stores
            if ((instruction & 0x60) == 0) {
                const bool set_flags = instruction & (1 << 20);
                const bool accumulate = instruction & (1 << 21);
                switch ((instruction >> 23) & 0x3) {
                case 0x0: {
                    return &Interpreter::ARMMultiply<accumulate, set_flags>;
                }
                case 0x1: {
                    const bool sign = instruction & (1 << 22);
                    return &Interpreter::ARMMultiplyLong<accumulate, set_flags, sign>;
                }
                case 0x2:
                    return &Interpreter::ARMSingleDataSwap;
                }
            } else {
                return &Interpreter::ARMHalfwordDataTransfer;
            }
        } else if (!set_flags && (opcode >= 0x8) && (opcode <= 0xB)) {
            // miscellaneous instructions
            if ((instruction & 0xF0) == 0) {
                return &Interpreter::ARMPSRTransfer;
            } else if ((instruction & 0xFF000F0) == 0x1200010) {
                return &Interpreter::ARMBranchExchange;
            } else if ((instruction & 0xFF000F0) == 0x1600010) {
                return &Interpreter::ARMCountLeadingZeroes;
            } else if ((instruction & 0xFF000F0) == 0x1200030) {
                return &Interpreter::ARMBranchLinkExchangeRegister;
            } else if ((instruction & 0xF0) == 0x50) {
                return &Interpreter::ARMSaturatingAddSubtract;
            } else if ((instruction & 0x70) == 0x70) {
                return &Interpreter::ARMBreakpoint;
            } else if ((instruction & 0x90) == 0x80) {
                return &Interpreter::ARMSignedHalfwordMultiply;
            }
        } else {
            return &Interpreter::ARMDataProcessing;
        }
    }
    case 0x1: {
        const bool set_flags = instruction & (1 << 20);
        const u8 opcode = (instruction >> 21) & 0xF;

        if (!set_flags && (opcode >= 0x8) && (opcode <= 0xB)) {
            if (instruction & (1 << 21)) {
                return &Interpreter::ARMPSRTransfer;
            } else {
                return &Interpreter::ARMUndefined;
            }
        } else {
            return &Interpreter::ARMDataProcessing;
        }
    }
    case 0x2:
        return &Interpreter::ARMSingleDataTransfer;
    case 0x3:
        if (instruction & (1 << 4)) {
            return &Interpreter::ARMUndefined;
        } else {
            return &Interpreter::ARMSingleDataTransfer;
        }
    case 0x4:
        if ((instruction >> 28) == 0xF) {
            return &Interpreter::ARMUndefined;
        } else {
            return &Interpreter::ARMBlockDataTransfer;
        }
    case 0x5: {
        // b/bl/blx
        const bool link = instruction & (1 << 24);

        return &Interpreter::ARMBranchLinkMaybeExchange<link>;
    }
    case 0x6:
        return &Interpreter::UnimplementedInstruction;
    case 0x7:
        if ((instruction & 0x1000010) == 0x10) {
            return &Interpreter::ARMCoprocessorRegisterTransfer;
        } else if ((instruction & 0x1000010) == 0) {
            return &Interpreter::UnimplementedInstruction;
        } else {
            return &Interpreter::ARMSoftwareInterrupt;
        }
    default:
        return &Interpreter::UnimplementedInstruction;
    }
}

template <u16 instruction>
static constexpr Instruction GetThumbInstruction() {
    switch ((instruction >> 13) & 0x7) {
    case 0x0:
        if (((instruction >> 11) & 0x3) == 0x3) {
            return &Interpreter::ThumbAddSubtract;
        } else {
            return &Interpreter::ThumbShiftImmediate;
        }
    case 0x1:
        return &Interpreter::ThumbALUImmediate;
    case 0x2:
        if (((instruction >> 10) & 0x7) == 0x0) {
            return &Interpreter::ThumbDataProcessingRegister;
        } else if (((instruction >> 10) & 0x7) == 0x1) {
            if ((instruction & 0xFF00) == 0x4700) {
                if (instruction & (1 << 7)) {
                    return &Interpreter::ThumbBranchLinkExchange;
                } else {
                    return &Interpreter::ThumbBranchExchange;
                }
            } else {
                return &Interpreter::ThumbSpecialDataProcesing;
            }
        } else {
            if (((instruction >> 12) & 0x1) == 0x1) {
                return &Interpreter::ThumbLoadStore;
            } else {
                return &Interpreter::ThumbLoadPC;
            }
        }
    case 0x3:
        return &Interpreter::ThumbLoadStoreImmediate;
    case 0x4:
        if (instruction & (1 << 12)) {
            return &Interpreter::ThumbLoadStoreSPRelative;
        } else {
            return &Interpreter::ThumbLoadStoreHalfword;
        }
    case 0x5:
        if (instruction & (1 << 12)) {
            // miscellaneous
            if (((instruction >> 8) & 0x1F) == 0x10) {
                return &Interpreter::ThumbAdjustStackPointer;
            } else if (((instruction >> 8) & 0x1F) == 0x1E) {
                return &Interpreter::ThumbSoftwareInterrupt;
            } else {
                return &Interpreter::ThumbPushPop;
            }
        } else {
            return &Interpreter::ThumbAddSPPC;
        }
    case 0x6:
        if (!(instruction & (1 << 12))) {
            return &Interpreter::ThumbLoadStoreMultiple;
        } else {
            if ((instruction & 0xFF00) == 0xDF00) {
                return &Interpreter::ThumbSoftwareInterrupt;
            } else {
                return &Interpreter::ThumbBranchConditional;
            }
        }
    case 0x7:
        if (instruction & (1 << 12)) {
            if (instruction & (1 << 11)) {
                return &Interpreter::ThumbBranchLinkOffset;
            } else {
                return &Interpreter::ThumbBranchLinkSetup;
            }
        } else {
            if (instruction & (1 << 11)) {
                return &Interpreter::ThumbBranchLinkExchangeOffset;
            } else {
                return &Interpreter::ThumbBranch;
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

    static_for<std::size_t, 0, 1024>([&](auto i) {
        thumb_lut[i] = GetThumbInstruction<i << 6>();
    }); 

    return thumb_lut;
}

void Interpreter::GenerateARMTable() {
    arm_lut = StaticGenerateARMTable();
}

void Interpreter::GenerateThumbTable() {
    thumb_lut = StaticGenerateThumbTable();
}
